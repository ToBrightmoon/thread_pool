#pragma once

#include <future>
#include "worker.h"

#include <iostream>
#include <numeric>
#include <ostream>

class ThreadPool
{
    using Worker_ptr = std::shared_ptr<Worker>;

    enum Status : int32_t
    {
        Running,
        Pause,
        Stop
    };

public:
    explicit ThreadPool(size_t min_thread_num = 1, size_t thread_num = std::thread::hardware_concurrency() - 1,
                        size_t max_thread_num = std::thread::hardware_concurrency() - 1);

    ~ThreadPool();

    void start();

    void stop();

    void pause();

    void resume();

    template<typename Fn, typename... Args>
    auto add_task(TaskPriority priority, Fn &&f, Args &&...args)
            -> std::future<decltype(f(std::forward<Args>(args)...))>;

    size_t get_thread_num() const;

private:
    void dispatch_task(const Task &task);

    void add_worker();

    void monitor();

    Status status_ = Status::Stop;
    mutable std::shared_mutex mtx_;
    PriorityQueue<Task> task_queue_;
    std::vector<Worker_ptr> workers_;
    std::condition_variable_any cond_;
    std::unique_ptr<std::thread> thread_;

    size_t min_thread_num_ = 1;
    size_t thread_num_ = std::thread::hardware_concurrency() - 1;
    size_t max_thread_num_ = std::thread::hardware_concurrency() - 1;
};

inline ThreadPool::ThreadPool(size_t min_thread_num, size_t thread_num, size_t max_thread_num) :
    min_thread_num_(min_thread_num), thread_num_(thread_num), max_thread_num_(max_thread_num)
{
    workers_.reserve(max_thread_num_);
}

inline ThreadPool::~ThreadPool() { stop(); }

inline void ThreadPool::start()
{
    std::unique_lock<std::shared_mutex> lock(mtx_);
    if (status_ == Status::Stop)
    {
        status_ = Status::Running;
    }
    thread_ = std::make_unique<std::thread>([this]() { monitor(); });

    for (size_t i = 0; i < thread_num_; ++i)
    {
        add_worker();
    }
}

inline void ThreadPool::stop()
{
    {
        std::unique_lock lock(mtx_);
        if (status_ == Status::Stop)
            return;
        status_ = Status::Stop;
        for (auto &worker: workers_)
        {
            worker->stop();
        }
        workers_.clear();
        task_queue_.clear();
    }
    cond_.notify_all();
    if (thread_ != nullptr && thread_->joinable())
        thread_->join();
}

inline void ThreadPool::pause()
{
    std::unique_lock<std::shared_mutex> lock(mtx_);
    if (status_ == Status::Running)
    {
        status_ = Status::Pause;
    }
    for (auto &worker: workers_)
    {
        worker->rest();
    }
}

inline void ThreadPool::resume()
{
    std::unique_lock<std::shared_mutex> lock(mtx_);
    if (status_ == Status::Pause)
    {
        status_ = Status::Running;
    }

    cond_.notify_all();
    for (auto &worker: workers_)
    {
        worker->work();
    }
}

inline size_t ThreadPool::get_thread_num() const
{
    std::shared_lock lock(mtx_);
    return workers_.size();
}

inline void ThreadPool::add_worker()
{
    if (status_ == Status::Running)
    {
        workers_.emplace_back(std::make_shared<Worker>());
    }
}

template<typename Fn, typename... Args>
auto ThreadPool::add_task(TaskPriority priority, Fn &&f, Args &&...args)
        -> std::future<decltype(f(std::forward<Args>(args)...))>
{
    using return_type = decltype(f(std::forward<Args>(args)...));
    auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Fn>(f), std::forward<Args>(args)...));
    Task priority_task([task]() { (*task)(); }, priority);

    auto future = task->get_future();

    {
        std::unique_lock lock(mtx_);
        if (status_ == Status::Stop)
        {
            throw std::runtime_error("ThreadPool::add_task() failed");
        }
        task_queue_.push(priority_task);
    }
    cond_.notify_all();
    return future;
}

inline void ThreadPool::monitor()
{
    while (true)
    {
        std::unique_lock<std::shared_mutex> lock(mtx_);
        cond_.wait_for(lock, std::chrono::milliseconds(100),
                       [&]() { return status_ == Status::Stop || (status_ == Status::Running && !task_queue_.empty()); });

        if (status_ == Status::Stop)
            return;

        size_t rest_thread_num =
                std::count_if(workers_.begin(), workers_.end(), [&](const Worker_ptr &worker) { return !worker->is_busy(); });
        size_t pending_task_num =
                std::accumulate(workers_.begin(), workers_.end(), (size_t) 0,
                                [](size_t sum, const Worker_ptr &worker) { return sum + worker->pending_task_size(); });

        if (rest_thread_num >= (workers_.size() / 2)  && max_thread_num_ > 1)
        {
            size_t remove_worker_num = std::min(rest_thread_num / 2, workers_.size() - min_thread_num_);
            for (size_t i = 0; i < remove_worker_num; ++i)
            {
                auto it = std::min_element(workers_.begin(), workers_.end(), [](const Worker_ptr &a, const Worker_ptr &b)
                {
                    return a->pending_task_size() < b->pending_task_size();
                });
                if (it == workers_.end())
                    break;
                it->get()->stop();
                workers_.erase(it);
            }
        };

        if (rest_thread_num == 0 && pending_task_num > 2 * max_thread_num_)
        {
            size_t add_worker_num = 0;

            add_worker_num = std::min(max_thread_num_ - workers_.size(), (workers_.size() + 1) / 2);

            for (size_t i = 0; i < add_worker_num; ++i)
            {
                add_worker();
            }
        }
        thread_num_ = workers_.size();

        if (task_queue_.empty())
            continue;
        size_t task_num = task_queue_.size();
        for (size_t i = 0; i < task_num; ++i)
        {
            dispatch_task(task_queue_.pop());
        }
    }
}

inline void ThreadPool::dispatch_task(const Task &task)
{
    if (!workers_.empty())
    {
        auto it = std::min_element(workers_.begin(), workers_.end(), [](const Worker_ptr &a, const Worker_ptr &b)
                 {
                     if (a == b)
                         return false;
                     return a->pending_task_size() < b->pending_task_size();
                 });
        if (it == workers_.end())
            return;
        it->get()->add_task(task);
    }
}

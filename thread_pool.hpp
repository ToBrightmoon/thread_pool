#pragma once

#include "default_strategy.h"

#include <future>
#include <iostream>
#include <numeric>

class ThreadPool
{
    using Worker_ptr = std::shared_ptr<Worker>;

public:
    enum Status : int32_t
    {
        Running,
        Pause,
        Stop
    };

    explicit ThreadPool(size_t min_thread_num = 1, size_t thread_num = std::thread::hardware_concurrency() - 1,
                        size_t max_thread_num = std::thread::hardware_concurrency() - 1,
                        const std::shared_ptr<ThreadPoolStrategy> &strategy = std::make_shared<DefaultStrategy>());

    ~ThreadPool();

    void start();

    void stop();

    void pause();

    void resume();

    template<typename Fn, typename... Args>
    auto add_task(TaskPriority priority, Fn &&f, Args &&...args)
            -> std::future<decltype(f(std::forward<Args>(args)...))>;

    template<typename Fn, typename... Args>
    auto add_task(Fn &&f, Args &&...args)
            -> std::future<decltype(f(std::forward<Args>(args)...))>;

    size_t get_thread_num() const;

    Status get_status() const;

    size_t get_task_num() const;

    static std::string status_to_string(const Status &status);

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
    std::shared_ptr<ThreadPoolStrategy> strategy_;

    size_t min_thread_num_ = 1;
    size_t thread_num_ = std::thread::hardware_concurrency() - 1;
    size_t max_thread_num_ = std::thread::hardware_concurrency() - 1;
};

inline ThreadPool::ThreadPool(size_t min_thread_num, size_t thread_num, size_t max_thread_num,
                              const std::shared_ptr<ThreadPoolStrategy> &strategy) :
    strategy_(strategy), min_thread_num_(min_thread_num), thread_num_(thread_num), max_thread_num_(max_thread_num)
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
    {
        std::unique_lock<std::shared_mutex> lock(mtx_);
        if (status_ == Status::Pause)
        {
            status_ = Status::Running;
        }
        for (auto &worker: workers_)
        {
            worker->work();
        }
    }

    cond_.notify_all();
}

inline size_t ThreadPool::get_thread_num() const
{
    std::shared_lock lock(mtx_);
    return workers_.size();
}

inline size_t ThreadPool::get_task_num() const
{
    std::shared_lock lock(mtx_);
    return std::accumulate(workers_.begin(), workers_.end(), task_queue_.size(), [](size_t total, const auto &worker)
    {
        return total + worker->pending_task_size();
    });
}

inline ThreadPool::Status ThreadPool::get_status() const
{
    std::shared_lock lock(mtx_);
    return status_;
}

inline std::string ThreadPool::status_to_string(const Status &status)
{
    switch (status)
    {
        case Status::Running:
            return "Running";
        case Status::Pause:
            return "Pause";
        case Status::Stop:
            return "Stop";
        default:
            return "Unknown";
    }
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
            throw std::runtime_error("ThreadPool::add_task() failed, The ThreadPool has been Stopped.");
        }
        task_queue_.push(priority_task);
    }
    cond_.notify_all();
    return future;
}

template<typename Fn, typename... Args>
    auto ThreadPool::add_task(Fn &&f, Args &&...args)
            -> std::future<decltype(f(std::forward<Args>(args)...))>
{
    return add_task(TaskPriority::Normal,std::forward<Fn>(f), std::forward<Args>(args)...);
}

inline void ThreadPool::monitor()
{
    while (true)
    {
        std::unique_lock<std::shared_mutex> lock(mtx_);
        cond_.wait_for(lock, std::chrono::milliseconds(100), [&]()
                       { return status_ == Status::Stop || (status_ == Status::Running && !task_queue_.empty()); });

        if (status_ == Status::Stop)
            return;

        strategy_->adjust_worker(min_thread_num_,max_thread_num_,task_queue_.size(),workers_);

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
    strategy_->dispatch_task(workers_, task);
}


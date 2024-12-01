#pragma once

#include <future>
#include "worker.h"

class ThreadPool
{
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

    void stop() ;

    void pause();

    void resume();

    template<typename Fn, typename... Args>
    auto add_task(TaskPriority priority, Fn &&f, Args &&...args)
            -> std::future<decltype(f(std::forward<Args>(args)...))>;

private:
    void dispatch_task(Task task);

    void add_worker();

    void monitor();

    mutable std::shared_mutex mtx_;
    std::vector<Worker> workers_;
    std::condition_variable cond_;
    Status status_ = Status::Stop;
    std::unique_ptr<std::thread> thread_;

    size_t min_thread_num_ = 1;
    size_t thread_num_ = std::thread::hardware_concurrency() - 1;
    size_t max_thread_num_ = std::thread::hardware_concurrency() - 1;
};

inline ThreadPool::ThreadPool(size_t min_thread_num, size_t thread_num, size_t max_thread_num) :
    min_thread_num_(min_thread_num), thread_num_(thread_num), max_thread_num_(max_thread_num)
{
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
    std::unique_lock lock(mtx_);
    if (status_ != Status::Stop)
    {
        if (thread_ != nullptr && thread_->joinable())
            thread_->join();
        for (auto &worker : workers_)
        {
            worker.stop();
        }
    }
}

inline void ThreadPool::pause()
{
    std::unique_lock<std::shared_mutex> lock(mtx_);
    if (status_ == Status::Running)
    {
        status_ = Status::Pause;
    }
    for (auto &  worker : workers_)
    {
        worker.rest();
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
        worker.work();
    }
}

inline void ThreadPool::add_worker()
{
    if (status_ == Status::Running)
    {
        workers_.emplace_back();
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
        dispatch_task(priority_task);
    }
    return future;
}

inline void ThreadPool::monitor() {}

inline void ThreadPool::dispatch_task(Task task)
{
    if (!workers_.empty())
    {
        std::sort(workers_.begin(), workers_.end(),
                  [](const Worker &l, const Worker &r) { return l.pending_task_size() < r.pending_task_size(); });
        workers_.at(0).add_task(task);
    }
}

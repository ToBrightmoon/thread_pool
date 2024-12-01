#include "worker.h"

#include <iostream>
#include <ostream>

#include "thread_pool.hpp"

Worker::Worker()
{
    thread_ptr_ = std::make_unique<std::thread>([this]() { run(); });
}

Worker::Worker(const Worker &other)
{
    std::shared_lock lock(other.mtx_);
    status_ = other.status_;
    task_queue_ = other.task_queue_;

    if (other.thread_ptr_ && thread_ptr_ == nullptr)
    {
        thread_ptr_ = std::make_unique<std::thread>([this]() { run(); });
    }
}

Worker &Worker::operator=(const Worker &other)
{
    if (this != &other)
    {
        std::unique_lock lock_m(mtx_);
        std::shared_lock lock_o(other.mtx_);

        status_ = other.status_;
        task_queue_ = other.task_queue_;

        if (other.thread_ptr_ && thread_ptr_ == nullptr)
        {
            thread_ptr_ = std::make_unique<std::thread>([this]() { run(); });
        }
    }
    return *this;
}

Worker::Worker(Worker&& other) noexcept
{
    std::unique_lock lock_m(mtx_);
    status_ = other.status_;
    task_queue_ = std::move(other.task_queue_);
    thread_ptr_ = std::move(other.thread_ptr_);
}

Worker &Worker::operator=(Worker&& other) noexcept
{
    if (this != &other)
    {
        std::unique_lock lock_m(mtx_);
        status_ = other.status_;
        task_queue_ = std::move(other.task_queue_);
        if (other.thread_ptr_ && thread_ptr_ == nullptr)
        {
            thread_ptr_ = std::move(other.thread_ptr_);
        }
    }
    return *this;
}

Worker::~Worker()
{
    stop();
}

void Worker::work()
{
    std::unique_lock lock(mtx_);
    if (status_ == WorkerStatus::Rest)
    {
        status_ = WorkerStatus::Busy;
    }
    notify();
}

void Worker::rest()
{
    std::unique_lock lock(mtx_);
    if (status_ == WorkerStatus::Busy)
    {
        status_ = WorkerStatus::Rest;
    }
}

void Worker::stop()
{
    {
        std::unique_lock lock(mtx_);
        if (status_ == WorkerStatus::Finish)
            return;
        status_ = WorkerStatus::Finish;
        task_queue_.clear();
    }
    notify();

    if (thread_ptr_ && thread_ptr_->joinable())
    {
        thread_ptr_->join();
    }
}

bool Worker::is_busy() const
{
    std::shared_lock lock(mtx_);
    return !task_queue_.empty();
}

size_t Worker::pending_task_size() const
{
    std::shared_lock lock(mtx_);
    return task_queue_.size();
}

void Worker::notify()
{
    cond_.notify_all();
}

void Worker::add_task(const Task &task)
{
    task_queue_.push(task);
    notify();
}

void Worker::run()
{
    while (true)
    {
        Task task;
        {
            std::unique_lock lock(mtx_);
            cond_.wait(lock, [this]
                       { return (status_ == WorkerStatus::Busy && !task_queue_.empty()) || status_ == WorkerStatus::Finish; });
            if (status_ == WorkerStatus::Finish)
            {
                return;
            }

            task = task_queue_.pop();
        }
        task();
    }
}



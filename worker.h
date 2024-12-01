#pragma once

#include <thread>
#include <condition_variable>

#include "priority_queue.hpp"

#include "task.h"

class Worker
{
    enum class WorkerStatus
    {
        Busy = 0,
        Rest = 1,
        Finish = 2
    };
public:
    Worker();

    Worker(const Worker&);

    Worker& operator=(const Worker&);

    Worker(Worker&&) noexcept;

    Worker& operator=(Worker&&) noexcept;

    ~Worker();

    void work();

    void rest();

    void stop();

    void add_task(const Task&);

    void notify() ;

    bool is_busy() const;

    size_t pending_task_size() const;

private:
    void run();

    mutable std::shared_mutex mtx_;
    WorkerStatus status_ = WorkerStatus::Busy;
    PriorityQueue<Task> task_queue_;
    std::condition_variable_any cond_;
    std::unique_ptr<std::thread> thread_ptr_;
};

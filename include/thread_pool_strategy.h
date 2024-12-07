#pragma once

#include <vector>
#include <memory>
#include "worker.h"

class ThreadPoolStrategy
{
public:
    virtual ~ThreadPoolStrategy() = default;

    virtual void dispatch_task(std::vector<std::shared_ptr<Worker>> workers,const Task &task) = 0;

    virtual void adjust_worker(size_t min_thread_num, size_t max_thread_num,size_t new_task_num,std::vector<std::shared_ptr<Worker>>& workers) = 0;
};



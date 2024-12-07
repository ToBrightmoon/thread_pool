#pragma once

#include "thread_pool_strategy.h"

class DefaultStrategy : public ThreadPoolStrategy
{
public:
  DefaultStrategy() = default;

  ~DefaultStrategy() override = default;

  void dispatch_task(std::vector<std::shared_ptr<Worker>> workers,const Task &task) override;

  void adjust_worker(size_t min_thread_num, size_t max_thread_num,size_t new_task_num,std::vector<std::shared_ptr<Worker>>& workers) override;
};

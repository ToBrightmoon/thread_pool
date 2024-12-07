#include "default_strategy.h"

#include <numeric>

void DefaultStrategy::dispatch_task(std::vector<std::shared_ptr<Worker>> workers,const Task &task) 
{
    if (!workers.empty())
    {
        auto it = std::min_element(workers.begin(), workers.end(),
                                   [](const auto &a, const auto &b)
                                   {
                                       if (a == b)
                                           return false;
                                       return a->pending_task_size() < b->pending_task_size();
                                   });
        if (it == workers.end())
            return;
        it->get()->add_task(task);
    }
}

void DefaultStrategy::adjust_worker(size_t min_thread_num, size_t max_thread_num,size_t new_task_num,std::vector<std::shared_ptr<Worker>>& workers)
{
    size_t rest_thread_num = std::count_if(workers.begin(), workers.end(),
                                               [&](const auto &worker) { return !worker->is_busy(); });
    size_t pending_task_num =
            std::accumulate(workers.begin(), workers.end(), new_task_num,
                            [](size_t sum, const auto &worker) { return sum + worker->pending_task_size(); });

    if (rest_thread_num >= (workers.size() / 2) && max_thread_num > 1)
    {
        size_t remove_worker_num = std::min(rest_thread_num / 2, workers.size() - min_thread_num);
        for (size_t i = 0; i < remove_worker_num; ++i)
        {
            auto it =
                    std::min_element(workers.begin(), workers.end(), [](const auto &a, const auto &b)
                                     { return a->pending_task_size() < b->pending_task_size(); });
            if (it == workers.end())
                break;
            it->get()->stop();
            workers.erase(it);
        }
    };

    if (rest_thread_num == 0 && pending_task_num > 2 * max_thread_num)
    {
        size_t add_worker_num = 0;

        add_worker_num = std::min(max_thread_num - workers.size(), (workers.size() + 1) / 2);

        for (size_t i = 0; i < add_worker_num; ++i)
        {
            workers.emplace_back(std::make_shared<Worker>());
        }
    }
}
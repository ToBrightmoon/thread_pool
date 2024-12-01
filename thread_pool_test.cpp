#include <iostream>
#include "thread_pool.hpp"

int main()
{
    ThreadPool pool(1,1);
    pool.start();

    pool.pause();
    pool.add_task(TaskPriority::Normal, []()
    {
        std::cout << "hello world\n";
    });
    pool.resume();
    std::this_thread::sleep_for(std::chrono::seconds(10));
}
#include "thread_pool.hpp"
#include <iostream>

int main()
{
    ThreadPool pool(1, 10);
    pool.start();

    pool.pause();
    pool.add_task(TaskPriority::Normal, []() { std::cout << "hello world\n"; });
    pool.resume();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    for (int i = 0; i < 10000; i++)
    {
        pool.add_task(TaskPriority::Normal, [&,i]()
        {
            std::cout << pool.get_thread_num() << "\n";
            // std::cout << i << ": hello world\n";
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        });
        if (i == 999)
        {
            i = 999;
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "sleep weak"<< "\n";
    pool.stop();
}

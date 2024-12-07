#include "thread_pool.hpp"

int main()
{
    ThreadPool pool;
    std::mutex mutex;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lock(mutex);

    pool.start();

    for (int i = 0; i < 1000; i++)
    {
        pool.add_task([&pool,&cv]()
        {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            cv.notify_one();
        });
    }

    for (int i = 0 ; i < 10000; ++i)
    {
        std::string msg = "the worker num is: " + std::to_string(pool.get_thread_num()) + "\n";
        std::cout << msg;
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        if (i % 100 == 0)
        {
            for (int j = 0 ; j < i; ++j)
            {
                pool.add_task([&pool,&cv]()
                {
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                    cv.notify_one();
                });
            }
        }
    }

    cv.wait(lock,[&pool]()
    {
        return pool.get_pending_task_num() == 0;
    });
}
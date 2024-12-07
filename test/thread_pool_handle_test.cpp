#include "thread_pool.hpp"

int main()
{
    ThreadPool pool;
    std::mutex mutex;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lock(mutex);

    pool.start();

    pool.add_task(
            [&cv]()
            {
                std::cout << "Exce a task" << std::endl;
                cv.notify_all();
            });

    cv.wait(lock);

    pool.pause();

    pool.add_task(
            [&cv]()
            {
                std::cout << "Exce the task" << std::endl;
                cv.notify_one();
            });

    pool.resume();
    std::cout << "The Task resume" << std::endl;
    cv.wait(lock);

    pool.stop();

    try
    {
        pool.add_task(
                [&cv]()
                {
                    std::cout << "Exce the task" << std::endl;
                    cv.notify_one();
                });
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

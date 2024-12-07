#include "thread_pool.hpp"

int main()
{
    ThreadPool pool(1,1,1);
    std::mutex mutex;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lock(mutex);

    pool.start();

    pool.pause();

    pool.add_task(TaskPriority::Lowest,[&cv]()
    {
        std::cout << "Lowest Task Exec" << std::endl;
        cv.notify_one();
    });

    pool.add_task(TaskPriority::Low,[]()
    {
        std::cout << "Low Task Exec" << std::endl;
    });

    pool.add_task(TaskPriority::Normal,[&cv]()
    {
        std::cout << "Normal Task Exec" << std::endl;
    });

    pool.add_task(TaskPriority::High,[]()
    {
        std::cout << "High Task Exec" << std::endl;
    });

    pool.add_task(TaskPriority::Highest,[]()
    {
       std::cout << "Highest Task Exec" << std::endl;
    });

    pool.resume();
    std::cout << "*****The Pool Resume*****" << std::endl;
    cv.wait(lock);
}

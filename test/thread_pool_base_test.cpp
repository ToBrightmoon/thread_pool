#include "thread_pool.hpp"

int main()
{
    std::mutex m;
    std::condition_variable cv;
    ThreadPool pool;

    pool.start();

    pool.add_task([&cv]()
                  {
                      std::cout << "Base Thread Task" << std::endl;
                      cv.notify_one();
                  });

    {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk);
    }

    auto task_f = pool.add_task([](int param)
    {
        std::cout << "Base Thread Task param is: " << param << std::endl;
        return param * 2;
    },1);

    auto res = task_f.get();
    std::cout << "The Task Result is: " << res << std::endl;
}
[中文版](README_CH.md)

[![Language](https://img.shields.io/badge/language-c++-red.svg)](https://en.cppreference.com/)
[![Platform](https://img.shields.io/badge/platform-linux%20%7C%20macos%20%7C%20windows-lightgrey.svg)](https://img.shields.io/badge/platform-linux%20%7C%20macos20%7C%20windows-lightgrey.svg)

## ThreadPool
A feature-rich C++ thread pool that supports dynamic thread management, multi-priority task scheduling, and thread pool state control. Suitable for a variety of scenarios.

## **Features**
- **Dynamic Thread Management** ：Supports configuring the minimum and maximum number of threads in the pool, dynamically adjusting the number of threads based on the work status of the pool, and allows custom thread management strategies.
- **Task Priority Support**：Supports task submission with priority levels.
- **Thread Pool State Management** ：Supports starting, pausing, resuming, and stopping the thread pool.
- **Cross-Platform**： Built on the C++17 standard, compatible with multiple platforms.

## **API Reference**

### Add Task
- **Add Regular Task**
```C++
template<typename Fn, typename... Args>
auto add_task(Fn &&f, Args &&...args) -> std::future<decltype(f(std::forward<Args>(args)...))>;
```
- **Add Priority Task**
```C++
template<typename Fn, typename... Args>
auto add_task(TaskPriority priority, Fn &&f, Args &&...args) -> std::future<decltype(f(std::forward<Args>(args)...))>;
```
### State Management
- `void start()`: Start the thread pool.
- `void stop()`: Stop the thread pool and release all resources.
- `void pause()`: Pause the thread pool and stop all task execution.
- `void resume()`: Resume the thread pool and continue task execution.
### State Queries
- `size_t get_thread_num()`:Get the current number of threads in the pool.
- `Status get_status()`: Get the current status of the thread pool.
- `size_t get_task_num()` ：Get the total number of tasks in the thread pool, including tasks currently being executed.

## Usage Example

Here’s a simple example of how to create and use the thread pool:
```c++
#include "thread_pool.hpp"
#include <iostream>

int main() 
{
    // Create a thread pool with a minimum of 2 threads and a maximum of 4 threads
    ThreadPool pool(2, 4);

    // Start the thread pool
    pool.start();

    //  Submit a regular task
    auto future1 = pool.add_task([] {
        std::cout << "Task 1 running" << std::endl;
        return 42;
    });

    //  Submit a high-priority task
    auto future2 = pool.add_task(TaskPriority::High, [] {
        std::cout << "Task 2 running with high priority" << std::endl;
        return 99;
    });

    // Get the results of the tasks
    std::cout << "Result of Task 1: " << future1.get() << std::endl;
    std::cout << "Result of Task 2: " << future2.get() << std::endl;

    // Pause and resume the thread pool
    pool.pause();
    std::cout << "Thread pool paused" << std::endl;
    pool.resume();

    // Stop the thread pool
    pool.stop();
    return 0;
}
```


[English Version](README.md)
[![Language](https://img.shields.io/badge/language-c++-red.svg)](https://en.cppreference.com/)
[![Platform](https://img.shields.io/badge/platform-linux%20%7C%20macos%20%7C%20windows-lightgrey.svg)](https://img.shields.io/badge/platform-linux%20%7C%20macos20%7C%20windows-lightgrey.svg)

## ThreadPool
一个功能丰富的C++线程池，支持动态线程管理，多任务优先级调度，以及线程池的状态线控，适用于多种场景

## **特性**
- **动态线程管理** ：支持配置线程池的最小，最大线程数，并根据线程池中的工作状态动态调整线程数量，并且支持用户定制线程的管理策略
- **任务优先级支持**：支持带有优先级的任务提交机制
- **线程池状态管理** ：支持线程池的启动，暂停，恢复和停止操作
- **跨平台支持**：基于C++17 标准，兼容多种平台

## **接口说明**

### 添加任务
- **添加普通任务**
```C++
template<typename Fn, typename... Args>
auto add_task(Fn &&f, Args &&...args) -> std::future<decltype(f(std::forward<Args>(args)...))>;
```
- **添加优先级任务**
```C++
template<typename Fn, typename... Args>
auto add_task(TaskPriority priority, Fn &&f, Args &&...args) -> std::future<decltype(f(std::forward<Args>(args)...))>;
```
### 状态管理
- `void start()`: 启动线程池
- `void stop()`: 停止线程池，释放所有资源
- `void pause()`: 暂停线程池，暂停所有任务的执行
- `void resume()`: 恢复线程池，继续执行任务
### 状态查询
- `size_t get_thread_num()`: 获取当前的线程数量
- `Status get_status()`: 获取当前线程池的状态
- `size_t get_task_num()` ： 获取线程池中，包括正在被执行的任务的总数

## 使用说明

以下是一个创建并且使用线程池的简单示例:
```c++
#include "thread_pool.hpp"
#include <iostream>

int main() 
{
    // 创建一个线程池，最小线程数为 2，最大线程数为 4
    ThreadPool pool(2, 4);

    // 启动线程池
    pool.start();

    // 提交普通任务
    auto future1 = pool.add_task([] {
        std::cout << "Task 1 running" << std::endl;
        return 42;
    });

    // 提交带优先级的任务
    auto future2 = pool.add_task(TaskPriority::High, [] {
        std::cout << "Task 2 running with high priority" << std::endl;
        return 99;
    });

    // 获取任务结果
    std::cout << "Result of Task 1: " << future1.get() << std::endl;
    std::cout << "Result of Task 2: " << future2.get() << std::endl;

    // 暂停和恢复线程池
    pool.pause();
    std::cout << "Thread pool paused" << std::endl;
    pool.resume();

    // 停止线程池
    pool.stop();
    return 0;
}
```


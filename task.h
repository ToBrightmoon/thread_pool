#pragma once

#include <functional>

#include "thread_pool_types.h"

class Task
{
public:
    Task() = default;

    explicit Task(std::function<void()>,TaskPriority = TaskPriority::Normal);

    Task(const Task&);

    Task(Task&&) noexcept;

    Task& operator=(const Task&);

    Task& operator=(Task&&) noexcept;

    ~Task() = default;

    bool operator<(const Task& other) const;

    bool operator>(const Task& other) const;

    bool operator<=(const Task& other) const;

    bool operator>=(const Task& other) const;

    void operator()() const noexcept;

private:
    std::function<void()> task_;

    TaskPriority priority_ = TaskPriority::Normal;
};
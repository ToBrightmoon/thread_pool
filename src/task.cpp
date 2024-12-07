#include "task.h"

Task::Task(std::function<void()> task, TaskPriority priority):task_(std::move(task)),priority_(priority)
{
}

Task::Task(const Task & other)  : task_(other.task_), priority_(other.priority_)
{
}

Task & Task::operator=(const Task & other)
{
    if (this != &other)
    {
        task_ = other.task_;
        priority_ = other.priority_;
    }
    return *this;
}

Task::Task(Task && task) noexcept : task_(std::move(task.task_)), priority_(task.priority_)
{
}

Task & Task::operator=(Task && other) noexcept
{
    if (this != &other)
    {
        task_ = std::move(other.task_);
        priority_ = other.priority_;
    }
    return *this;
}

bool Task::operator<(const Task &other) const
{
    return static_cast<int32_t>(priority_) < static_cast<int32_t>(other.priority_);
}

bool Task::operator>(const Task &other) const
{
    return static_cast<int32_t>(priority_) > static_cast<int32_t>(other.priority_);
}

bool Task::operator>=(const Task &other) const
{
    return static_cast<int32_t>(priority_) >= static_cast<int32_t>(other.priority_);
}

bool Task::operator<=(const Task &other) const
{
    return static_cast<int32_t>(priority_) <= static_cast<int32_t>(other.priority_);
}

void Task::operator()() const noexcept
{
    try
    {
        task_();
    }
    catch(...)
    {
    }
}






#pragma once

#include <queue>
#include <cstddef>
#include <shared_mutex>

template <typename T>
class PriorityQueue
{
public:
    PriorityQueue() = default;

    ~PriorityQueue() = default;

    PriorityQueue(const PriorityQueue& other)
    {
        std::shared_lock lock(other.mtx_);
        queue_ = other.queue_;
    }

    PriorityQueue& operator=(const PriorityQueue& other)
    {
        if (this != &other)
        {
            std::unique_lock lock_m(mtx_);
            std::shared_lock lock(other.mtx_);
            queue_ = other.queue_;
        }
        return *this;
    }

    PriorityQueue(PriorityQueue&& other) noexcept
    {
        std::unique_lock lock_m(mtx_);
        queue_ = std::move(other.queue_);
    }

    PriorityQueue& operator=(PriorityQueue&& other) noexcept
    {
        if (this != &other)
        {
            std::unique_lock lock_m(mtx_);
            queue_ = std::move(other.queue_);
        }
        return *this;
    }

    void push(T val)
    {
        std::unique_lock lock(mtx_);
        queue_.push(val);
    }

    T pop()
    {
        std::unique_lock lock(mtx_);
        if (!queue_.empty())
        {
            T val = queue_.top();
            queue_.pop();
            return val;
        }
        throw std::out_of_range("PriorityQueue::pop");
    }

    size_t size() const
    {
        std::shared_lock lock(mtx_);
        return queue_.size();
    }

    void clear()
    {
        std::unique_lock lock(mtx_);
        while (!queue_.empty())
        {
            queue_.pop();
        }
    }

    bool empty() const
    {
        std::shared_lock lock(mtx_);
        return queue_.empty();
    }

private:
    mutable std::shared_mutex mtx_;

    std::priority_queue<T> queue_;
};



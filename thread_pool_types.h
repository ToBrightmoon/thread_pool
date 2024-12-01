#pragma once

#include <cstdint>

enum TaskPriority : int32_t
{
    Lowest = 0,
    Low = 1,
    Normal = 2,
    High = 3,
    Highest = 4
};
#pragma once

#include <cstdint>

struct Vec2 {
    int64_t x, y;

    bool operator==(const Vec2& other) const { return x == other.x && y == other.y; }
};


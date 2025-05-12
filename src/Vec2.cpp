#include "Vec2.h"

#include <functional>

Vec2 Vec2::operator+(const Vec2& other) const {
  return Vec2{x + other.x, y + other.y};
}

Vec2& Vec2::operator+=(const Vec2& other) {
  x += other.x;
  y += other.y;
  return *this;
}

bool Vec2::operator<(const Vec2& other) const {
  return (y < other.y) || (y == other.y && x < other.x);
}

bool Vec2::operator==(const Vec2& other) const {
  return x == other.x && y == other.y;
}

std::size_t std::hash<Vec2>::operator()(const Vec2& v) const noexcept {
  return (std::hash<int>()(v.x) << 1) ^ std::hash<int>()(v.y);
}

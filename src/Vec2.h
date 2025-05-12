#pragma once

#include <cstddef>
#include <functional>

struct Vec2 {
  int x = 0;
  int y = 0;

  Vec2 operator+(const Vec2& other) const;
  Vec2& operator+=(const Vec2& other);

  bool operator<(const Vec2& other) const;
  bool operator==(const Vec2& other) const;
};

inline bool operator!=(const Vec2& a, const Vec2& b) {
  return !(a == b);
}

// Hash specialization for unordered_map
namespace std {
  template <>
  struct hash<Vec2> {
    std::size_t operator()(const Vec2& v) const noexcept;
  };
}

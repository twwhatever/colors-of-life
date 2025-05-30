#pragma once

#include "Vec2.h"
#include "Grid.h"
#include <SFML/Graphics/Color.hpp>
#include <vector>

// Forward declaration for Ruby VALUE
typedef unsigned long VALUE;

class Entity {
public:
  Vec2 pos;
  sf::Color color;
  int energy;

  Entity(Vec2 position, const sf::Color color, int initial_energy = 10);

  // Called each turn to decide movement
  Vec2 request_move(const Grid& local_grid);

  // Update position after resolving movement
  void update_position(const Vec2& new_pos);
};

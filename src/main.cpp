#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <chrono>
#include <ruby.h>
#include <iostream>

#include "Entity.h"
#include "Grid.h"

constexpr int GRID_WIDTH = 128;
constexpr int GRID_HEIGHT = 128;
constexpr int CELL_SIZE = 5;
constexpr int WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;
constexpr int WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE;


bool in_bounds(Vec2 v) {
  if (v.x < 0 || v.x >= GRID_WIDTH) {
    return false;
  }
  if (v.y < 0 || v.y >= GRID_HEIGHT) {
    return false;
  }
  return true;
}

// Randomly fill initial grid
void initialize_grid(Grid& grid) {
  std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
  std::bernoulli_distribution dist(0.27);
  for (int x = 0; x < GRID_WIDTH; ++x)
  for (int y = 0; y < GRID_HEIGHT; ++y)
  grid[x][y] = dist(rng) == 1 ? CellType::Food : CellType::Empty;
}

std::vector<Entity> spawn_entities(int width, int height, int count_per_team) {
  const int INITIAL_ENERGY = 60;
  std::vector<Entity> entities;
  std::unordered_map<Vec2, bool> occupied;

  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<int> y_dist(0, height - 1);

  auto unique_random_pos = [&](int x_min, int x_max) {
    std::uniform_int_distribution<int> x_dist(x_min, x_max);
    while (true) {
      Vec2 pos{x_dist(rng), y_dist(rng)};
      const auto& [_, inserted] = occupied.emplace(std::pair{pos, false});
      if (inserted) {
        return pos;
      }
    }
  };

  // Team A - Red - Left half
  for (int i = 0; i < count_per_team; ++i) {
    Vec2 pos = unique_random_pos(0, width / 2 - 1);
    entities.emplace_back(pos, sf::Color::Red, INITIAL_ENERGY);
  }

  // Team B - Blue - Right half
  for (int i = 0; i < count_per_team; ++i) {
    Vec2 pos = unique_random_pos(width / 2, width - 1);
    entities.emplace_back(pos, sf::Color::Blue, INITIAL_ENERGY);
  }

  return entities;
}

// Count alive neighbors for Game of Life
int count_neighbors(const Grid& grid, int x, int y) {
  int count = 0;
  for (int dx = -1; dx <= 1; ++dx)
  for (int dy = -1; dy <= 1; ++dy) {
    if (dx == 0 && dy == 0) continue;
    int nx = x + dx;
    int ny = y + dy;
    if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT)
    count += grid[nx][ny] == CellType::Food ? 1 : 0;
  }
  return count;
}

// Apply Game of Life rules
void update_grid(Grid& grid) {
  Grid new_grid = grid;
  for (int x = 0; x < GRID_WIDTH; ++x) {
    for (int y = 0; y < GRID_HEIGHT; ++y) {
      int neighbors = count_neighbors(grid, x, y);
      if (grid[x][y] == CellType::Food) {
        new_grid[x][y] = (neighbors == 2 || neighbors == 3) ? CellType::Food : CellType::Empty;
      } else {
        new_grid[x][y] = (neighbors == 3) ? CellType::Food : CellType::Empty;
      }
    }
  }
  grid = std::move(new_grid);
}

// Apply entity movement(
void update_entities(std::vector<Entity>& entities, Grid& food_grid) {
  // What moves do entities want to make?
  std::map<Vec2, std::vector<Entity*>> movement_requests;
  std::map<Vec2, const Entity*> entity_positions;
  for (const auto& entity : entities) {
    entity_positions[entity.pos] = &entity;
  }
  for (auto& entity : entities) {
    if (entity.energy <= 0) {
      // Entity cannot move due to low energy.  Generate a
      // request to "stay" so that other entities won't be
      // able to move to this square.
      movement_requests[entity.pos].emplace_back(&entity);
      continue;
    }
    // 5x5 local grid centered on entity
    Grid local(5, std::vector<CellType>(5, CellType::Empty));
    for (int dx = -2; dx <= 2; ++dx) {
      for (int dy = -2; dy <= 2; ++dy) {
        int gx = entity.pos.x + dx;
        int gy = entity.pos.y + dy;
        if (gx >= 0 && gx < GRID_WIDTH && gy >= 0 && gy < GRID_HEIGHT) {
          local[dx + 2][dy + 2] = food_grid[gx][gy];
        } else if (
          entity_positions.count(Vec2{.x = gx, .y = gy})) {
            local[dx + 2][dy + 2] = (
              entity_positions[Vec2{.x = gx, .y = gy}]->color == entity.color
             ) ? CellType::Teammate : CellType::Opponent; 
          }
        }
      }

    const auto movement_request = entity.request_move(local);
    if (!in_bounds(movement_request)) {
      // Illegal move, reinterpret as "stay".
      movement_requests[entity.pos].emplace_back(&entity);
      continue; 
    }
    movement_requests[movement_request].emplace_back(&entity);
  }

  // Resolve movement.
  for (auto& [target, movers] : movement_requests) {
    if (movers.size() == 1) {
      movers.front()->update_position(target);
    }
    // Ignore movement requests for squares multiple
    // entities are trying to move to.
  }
}

void eat(std::vector<Entity>& entities, Grid& food_grid) {
  for (auto& entity : entities) {
    const auto& pos = entity.pos;
    if (food_grid[pos.x][pos.y] == CellType::Food) {
      entity.energy += 5;  // TODO: tune this.
      food_grid[pos.x][pos.y] = CellType::Empty;
    }
  }
}

int main() {
  sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Food Grid - Game of Life");
  window.setFramerateLimit(60);
  
  Grid food_grid(GRID_WIDTH, std::vector<CellType>(GRID_HEIGHT, CellType::Empty));
  initialize_grid(food_grid);
  
  std::vector<Entity> entities = spawn_entities(GRID_WIDTH, GRID_HEIGHT, 200);

  sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
  cell.setFillColor(sf::Color::Green);
  
  ruby_init();
  ruby_init_loadpath();
  ruby_script("ruby_life_embedded");
  ruby_options(0, 0);

  rb_eval_string("$LOAD_PATH.unshift('./scripts')");
  int state = 0;
  rb_load_protect(rb_str_new_cstr("ai_red.rb"), 0, &state);
  if (state) {
      std::cerr << "Ruby script ai_red.rb failed to load (state = " << state << ")\n";
      ruby_cleanup(0);
      return 1;
  }

  rb_load_protect(rb_str_new_cstr("ai_blue.rb"), 0, &state);
  if (state) {
      std::cerr << "Ruby script ai_blue.rb failed to load (state = " << state << ")\n";
      ruby_cleanup(0);
      return 1;
  }
  if (state) {
      std::cerr << "Ruby script failed to load (state = " << state << ")\n";
      ruby_cleanup(0);
      return 1;
  } 

  sf::Clock update_timer;
  const float update_interval = 0.01f; // seconds
  
  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event))
    if (event.type == sf::Event::Closed)
    window.close();
    
    if (update_timer.getElapsedTime().asSeconds() > update_interval) {
      update_entities(entities, food_grid);
      eat(entities, food_grid);
      update_grid(food_grid);
      update_timer.restart();
    }
    
    window.clear(sf::Color::Black);
    for (int x = 0; x < GRID_WIDTH; ++x) {
      for (int y = 0; y < GRID_HEIGHT; ++y) {
        if (food_grid[x][y] == CellType::Food) {
          bool has_entity = false;
          for (const auto& e : entities)
          if (e.pos.x == x && e.pos.y == y)
          has_entity = true;
          if (!has_entity) {
            cell.setPosition(x * CELL_SIZE, y * CELL_SIZE);
            window.draw(cell);
          }
        }
      }
    }
    
    for (const auto& entity : entities) {
      sf::RectangleShape square(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
      square.setFillColor(entity.color);
      square.setPosition(entity.pos.x * CELL_SIZE, entity.pos.y * CELL_SIZE);
      window.draw(square);
    }
    
    window.display();
  }
  
  ruby_finalize();
  return 0;
}

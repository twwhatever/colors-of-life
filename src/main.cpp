#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <chrono>
#include <ruby.h>
#include <iostream>

constexpr int GRID_WIDTH = 64;
constexpr int GRID_HEIGHT = 64;
constexpr int CELL_SIZE = 10;
constexpr int WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;
constexpr int WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE;

using Grid = std::vector<std::vector<bool>>;

struct Entity {
  int x, y;
  sf::Color color;
  int energy;
};

// Randomly fill initial grid
void initialize_grid(Grid& grid) {
  std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
  std::uniform_int_distribution<> dist(0, 1);
  for (int x = 0; x < GRID_WIDTH; ++x)
  for (int y = 0; y < GRID_HEIGHT; ++y)
  grid[x][y] = dist(rng) == 1;
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
    count += grid[nx][ny] ? 1 : 0;
  }
  return count;
}

// Wrapper for ruby decide_action function.
VALUE call_decide_action(const std::vector<std::vector<bool>>& local_grid, int energy) {
  VALUE rb_local = rb_ary_new();
  for (const auto& row : local_grid) {
    VALUE rb_row = rb_ary_new();
    for (bool cell : row)
    rb_ary_push(rb_row, cell ? Qtrue : Qfalse);
    rb_ary_push(rb_local, rb_row);
  }
  
  VALUE rb_energy = INT2NUM(energy);
    // Pack args into an array
    VALUE args = rb_ary_new();
    rb_ary_push(args, rb_local);
    rb_ary_push(args, rb_energy);

    // Prepare a function to call safely
    auto safe_call = [](VALUE arg) -> VALUE {
        VALUE* argv = reinterpret_cast<VALUE*>(arg);
        return rb_funcall(rb_cObject, rb_intern("decide_action"), 2, argv[0], argv[1]);
    };

    VALUE argv[2] = {rb_local, rb_energy};
    int state = 0;
    VALUE result = rb_protect(safe_call, reinterpret_cast<VALUE>(argv), &state);
    if (state) {
      VALUE err = rb_errinfo();
      VALUE err_str = rb_funcall(err, rb_intern("to_s"), 0);
      std::cerr << "Ruby error: " << StringValueCStr(err_str) << "\n";
      rb_set_errinfo(Qnil);  // Clear it
      return ID2SYM(rb_intern("stay"));
  }
  return result;
}

// Apply Game of Life rules
void update_grid(Grid& grid) {
  Grid new_grid = grid;
  for (int x = 0; x < GRID_WIDTH; ++x) {
    for (int y = 0; y < GRID_HEIGHT; ++y) {
      int neighbors = count_neighbors(grid, x, y);
      if (grid[x][y]) {
        new_grid[x][y] = (neighbors == 2 || neighbors == 3);
      } else {
        new_grid[x][y] = (neighbors == 3);
      }
    }
  }
  grid = std::move(new_grid);
}

// Apply entity movement(
void update_entities(std::vector<Entity>& entities, Grid& food_grid) {
  for (auto& entity : entities) {
    // 5x5 local grid centered on entity
    std::vector<std::vector<bool>> local(5, std::vector<bool>(5, false));
    for (int dx = -2; dx <= 2; ++dx) {
      for (int dy = -2; dy <= 2; ++dy) {
        int gx = entity.x + dx;
        int gy = entity.y + dy;
        if (gx >= 0 && gx < GRID_WIDTH && gy >= 0 && gy < GRID_HEIGHT)
        local[dx + 2][dy + 2] = food_grid[gx][gy];
      }
    }
    
    VALUE action = call_decide_action(local, entity.energy);
    VALUE rb_str = rb_sym2str(action);
    std::string direction = StringValueCStr(rb_str);
    
    int dx = 0, dy = 0;
    if (direction == "north") dy = -1;
    if (direction == "south") dy = 1;
    if (direction == "west") dx = -1;
    if (direction == "east") dx = 1;
    
    int new_x = entity.x + dx;
    int new_y = entity.y + dy;
    
    if (new_x >= 0 && new_x < GRID_WIDTH && new_y >= 0 && new_y < GRID_HEIGHT) {
      entity.x = new_x;
      entity.y = new_y;
    }
  }
}

int main() {
  sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Food Grid - Game of Life");
  window.setFramerateLimit(60);
  
  Grid food_grid(GRID_WIDTH, std::vector<bool>(GRID_HEIGHT, false));
  initialize_grid(food_grid);
  
  std::vector<Entity> entities = {
    {10, 10, sf::Color::Red, 10},
    {20, 20, sf::Color::Blue, 10}
  };
  
  sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
  cell.setFillColor(sf::Color::Green);
  
  ruby_init();
  ruby_init_loadpath();
  ruby_script("ruby_life_embedded");
  ruby_options(0, 0);

  rb_eval_string("$LOAD_PATH.unshift('./scripts')");
  int state = 0;
  rb_load_protect(rb_str_new_cstr("ai.rb"), 0, &state);
  if (state) {
      std::cerr << "Ruby script failed to load (state = " << state << ")\n";
      ruby_cleanup(0);
      return 1;
  } 

  sf::Clock update_timer;
  const float update_interval = 0.5f; // seconds
  
  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event))
    if (event.type == sf::Event::Closed)
    window.close();
    
    if (update_timer.getElapsedTime().asSeconds() > update_interval) {
      update_entities(entities, food_grid);
      update_grid(food_grid);
      update_timer.restart();
    }
    
    window.clear(sf::Color::Black);
    for (int x = 0; x < GRID_WIDTH; ++x) {
      for (int y = 0; y < GRID_HEIGHT; ++y) {
        if (food_grid[x][y]) {
          bool has_entity = false;
          for (const auto& e : entities)
          if (e.x == x && e.y == y)
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
      square.setPosition(entity.x * CELL_SIZE, entity.y * CELL_SIZE);
      window.draw(square);
    }
    
    window.display();
  }
  
  ruby_finalize();
  return 0;
}

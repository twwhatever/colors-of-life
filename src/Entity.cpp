#include "Entity.h"
#include <ruby.h>
#include <iostream>

// Helper: Convert Ruby symbol (e.g., :north) to Vec2 delta
static Vec2 direction_from_symbol(VALUE sym) {
  VALUE dir_str = rb_funcall(sym, rb_intern("to_s"), 0);
  std::string dir = StringValueCStr(dir_str);

  if (dir == "north") return {0, -1};
  if (dir == "south") return {0, 1};
  if (dir == "west")  return {-1, 0};
  if (dir == "east")  return {1, 0};
  return {0, 0};  // stay
}

// Helper: Convert celltype to Ruby symbol
static VALUE cell_type_to_symbol(CellType t) {
  switch (t) {
    case CellType::Food:     return ID2SYM(rb_intern("food"));
    case CellType::Teammate: return ID2SYM(rb_intern("teammate"));
    case CellType::Opponent: return ID2SYM(rb_intern("opponent"));
    default:                 return ID2SYM(rb_intern("empty"));
  }
}

Entity::Entity(Vec2 position, const sf::Color color, int initial_energy)
    : pos(position), color(color), energy(initial_energy) {}

Vec2 Entity::request_move(const Grid& local_grid) {
  if (energy <= 0) return pos;

  // Convert local_grid to Ruby VALUE
  VALUE rb_local = rb_ary_new();
  rb_gc_register_address(&rb_local);
  for (const auto& row : local_grid) {
    VALUE rb_row = rb_ary_new();
    for (auto cell : row) {
      VALUE ruby_cell_type = cell_type_to_symbol(cell);
        rb_ary_push(rb_row, ruby_cell_type);
    }
    rb_ary_push(rb_local, rb_row);
  }

  VALUE rb_energy = INT2NUM(energy);
  VALUE argv[2] = {rb_local, rb_energy};

  auto safe_call = [](VALUE arg) -> VALUE {
    VALUE* args = reinterpret_cast<VALUE*>(arg);
    return rb_funcall(rb_cObject, rb_intern("decide_action"), 2, args[0], args[1]);
  };

  int state = 0;
  VALUE result = rb_protect(safe_call, reinterpret_cast<VALUE>(argv), &state);
  rb_gc_unregister_address(&rb_local);
  if (state != 0) {
    VALUE err = rb_errinfo();
    VALUE msg = rb_funcall(err, rb_intern("to_s"), 0);
    std::cerr << "Ruby error: " << StringValueCStr(msg) << std::endl;
    rb_set_errinfo(Qnil);
    return pos;  // fallback: no move
  }

  Vec2 delta = direction_from_symbol(result);
  return pos + delta;
}

void Entity::update_position(const Vec2& new_pos) {
  if (pos != new_pos) {
    pos = new_pos;
    energy -= 1;
  }
}

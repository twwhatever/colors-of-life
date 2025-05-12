#include <ruby.h>
#include <iostream>

int main() {
  std::cout << "This may crash\n";
  ruby_init();
  ruby_init_loadpath();
  
  VALUE arr = rb_ary_new();

#ifdef PROTECT_ARRAY
  rb_gc_register_address(&arr);
#endif 
  
  for (int i = 0; i < 100000; ++i) {
    VALUE dummy = rb_ary_new();
    rb_ary_push(dummy, INT2NUM(i));
    if (i % 1000 == 0) {
      rb_ary_push(arr, dummy);
    }
  }
  
  rb_ary_push(arr, rb_str_new_cstr("hello"));

#ifdef PROTECT_ARRAY
  rb_gc_unregister_address(&arr);
#endif
  
  ruby_finalize();
  std::cout << "But it didn't\n";
}

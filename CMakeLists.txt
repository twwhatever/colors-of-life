cmake_minimum_required(VERSION 3.15)
project(RubyLifeGame)

# Conan toolchain
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

# SFML via Conan
find_package(SFML REQUIRED COMPONENTS graphics window system)
include_directories(${SFML_INCLUDE_DIRS})

# Get Ruby include dir via Ruby itself
execute_process(
    COMMAND /opt/homebrew/opt/ruby/bin/ruby -rrbconfig -e "puts RbConfig::CONFIG['rubyhdrdir']"
    OUTPUT_VARIABLE RUBY_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND /opt/homebrew/opt/ruby/bin/ruby -rrbconfig -e "puts RbConfig::CONFIG['rubyarchhdrdir']"
    OUTPUT_VARIABLE RUBY_ARCH_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND /opt/homebrew/opt/ruby/bin/ruby -rrbconfig -e "puts RbConfig::CONFIG['LIBRUBYARG_SHARED']"
    OUTPUT_VARIABLE RUBY_LIB
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND /opt/homebrew/opt/ruby/bin/ruby -rrbconfig -e "puts RbConfig::CONFIG['libdir']"
    OUTPUT_VARIABLE RUBY_LIBRARY_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Log it for debugging
message(STATUS "Ruby include: ${RUBY_INCLUDE_DIR}")
message(STATUS "Ruby arch include: ${RUBY_ARCH_INCLUDE_DIR}")
message(STATUS "Ruby lib: ${RUBY_LIB}")
message(STATUS "Ruby lib path: ${RUBY_LIBRARY_DIR}")

include_directories(${RUBY_INCLUDE_DIR} ${RUBY_ARCH_INCLUDE_DIR})
link_directories(${RUBY_LIBRARY_DIR})

add_executable(ruby_life
    src/main.cpp
    src/entity.cpp
    src/vec2.cpp
)

set_target_properties(ruby_life PROPERTIES
    INSTALL_RPATH "/opt/homebrew/opt/ruby/lib"
)

target_link_libraries(ruby_life
    PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    ${RUBY_LIB}
)

add_executable(gc_registration_crash tests/gc_registration_crash.cpp)
target_include_directories(gc_registration_crash PRIVATE ${RUBY_INCLUDE_DIR})
target_link_libraries(gc_registration_crash PRIVATE ${RUBY_LIB})

add_executable(gc_registration_fixed tests/gc_registration_crash.cpp)
target_compile_definitions(gc_registration_fixed PRIVATE PROTECT_ARRAY)
target_include_directories(gc_registration_fixed PRIVATE ${RUBY_INCLUDE_DIR})
target_link_libraries(gc_registration_fixed PRIVATE ${RUBY_LIB})
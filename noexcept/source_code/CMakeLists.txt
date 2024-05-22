
cmake_minimum_required(VERSION 3.27)

project(noexcept LANGUAGES CXX)

find_package(prebuilt-picolibc REQUIRED)

add_executable(app.elf
  src/main.cpp
  src/external.cpp
  src/dtor_paths.cpp
  src/except_vs_noexcept.cpp
)

target_compile_options(app.elf PRIVATE
  -g
  -fexceptions
  -fno-rtti
  -Wall
  -Wpedantic
)

target_include_directories(app.elf PUBLIC src)
target_compile_features(app.elf PRIVATE cxx_std_23)
target_link_options(app.elf PRIVATE
  -L${CMAKE_SOURCE_DIR}/
  -Wl,-T ${CMAKE_SOURCE_DIR}/linker.ld
  -Wl,--wrap=__cxa_allocate_exception
  -Wl,--wrap=__cxa_free_exception
)
target_link_libraries(app.elf PRIVATE picolibc)

libhal_post_build(app.elf)
libhal_disassemble(app.elf)

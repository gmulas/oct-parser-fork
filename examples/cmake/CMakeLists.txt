# Example: using an installed oct_parser from CMake.
#   cmake -S . -B build -DCMAKE_PREFIX_PATH=<oct_parser prefix>
#   cmake --build build
# Set -DWITH_FORTRAN=ON to also build the Fortran example.
cmake_minimum_required(VERSION 3.12)
project(oct_parser_example C)

option(WITH_FORTRAN "Build the Fortran example too" OFF)

if(WITH_FORTRAN)
  enable_language(Fortran)
  find_package(OctParser 2026.10 REQUIRED COMPONENTS Fortran)
  add_executable(example_f main.f90)
  target_link_libraries(example_f PRIVATE OctParser::oct_parser_f)
else()
  find_package(OctParser 2026.10 REQUIRED)
endif()

add_executable(example_c main.c)
target_link_libraries(example_c PRIVATE OctParser::oct_parser)

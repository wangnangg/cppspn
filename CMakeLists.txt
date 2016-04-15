cmake_minimum_required(VERSION 3.3)
project(spnp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_subdirectory(unit_tests)

set(SOURCE_FILES src/PetriNetModel.cpp src/Estimating.cpp src/Estimating.h src/Simulating.cpp src/Simulating.h src/Statistics.h src/Statistics.cpp)
add_library(spnp ${SOURCE_FILES})


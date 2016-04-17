cmake_minimum_required(VERSION 3.3)
project(spnp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
SET(CMAKE_C_FLAGS_DEBUG "-D_DEBUG")

add_subdirectory(unit_tests)

include_directories(src)

set(SOURCE_FILES src/Estimating.cpp src/Estimating.h
        src/Simulating.cpp src/Simulating.h
        src/Statistics.h src/Statistics.cpp
        src/PetriNetModel/PetriNetModel.h src/PetriNetModel/PetriNetStatic.cpp src/PetriNetModel/PetriNet.cpp)
add_library(spnp ${SOURCE_FILES})


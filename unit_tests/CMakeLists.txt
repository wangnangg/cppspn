add_subdirectory(googletest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
include_directories(googletest/include ../src)
link_libraries(spnp gtest gtest_main)
add_executable(unit_test estimating_test.cpp simulating_test.cpp petri_net_model_test.cpp)
add_executable(experiment_test test.cpp)

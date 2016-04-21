add_subdirectory(googletest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall")
include_directories(googletest/include ../src)
add_executable(unit_test estimating_test.cpp simulating_test.cpp petri_net_model_test.cpp helper.h helper.cpp)
target_link_libraries(unit_test spnp gtest gtest_main)

add_executable(experiment_test test.cpp)
target_link_libraries(experiment_test gtest gtest_main)

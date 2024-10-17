cmake_minimum_required(VERSION 3.30)
project(cache_sim)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src/lib)
add_subdirectory(src)
target_link_libraries(${PROJECT_NAME} io)

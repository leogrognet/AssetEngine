cmake_minimum_required(VERSION 3.20)
project(AssetEngine LANGUAGES CXX)

add_subdirectory(lib)

add_executable(demo src/main.cpp)
target_link_libraries(demo PRIVATE AssetEngine)
cmake_minimum_required(VERSION 3.7)
project(VkEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(renderer ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/renderer)
add_subdirectory(application ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/application)
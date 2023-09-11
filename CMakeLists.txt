cmake_minimum_required(VERSION 3.7)
project(VkEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CCACHE "ccache")
if(CCACHE)
    message("CCache Found.")
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE}) # Less useful to do it for linking, see edit2
endif(CCACHE)

add_subdirectory(renderer ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/renderer)
add_subdirectory(application ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/application)
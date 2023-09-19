# project(renderer)

add_library(
    renderer STATIC
    renderer.cpp includes/renderer.hpp
)

# CMake 3.7 added the FindVulkan module
message(STATUS "Finding Vulkan SDK using CMake...")
find_package(Vulkan)

if(NOT Vulkan_FOUND)
    message("Error: Couldn't Locate the Vulkan SDK!")
    return()
endif()

message(STATUS "Vulkan FOUND = ${Vulkan_FOUND}")
message(STATUS "Vulkan Include = ${Vulkan_INCLUDE_DIR}")
message(STATUS "Vulkan Lib = ${Vulkan_LIBRARY}")

# Vulkan, GLFW & GLM
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw ${CMAKE_BINARY_DIR}/external/glfw)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/glm ${CMAKE_BINARY_DIR}/external/glm)
target_link_libraries(renderer
    PRIVATE ${Vulkan_LIBRARIES}
    PRIVATE glfw
    PRIVATE glm
)

# target_compile_options(renderer PUBLIC -fexceptions)

target_include_directories(renderer
    PUBLIC ${Vulkan_INCLUDE_DIR}
    PUBLIC ${CMAKE_SOURCE_DIR}/external/glfw/include
    PUBLIC ${CMAKE_SOURCE_DIR}/external/glm
    PUBLIC ${CMAKE_SOURCE_DIR}/engine/config
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/utils
)
project(engine)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/renderer)

add_library(
    engine STATIC
    engine.cpp # includes/engine.hpp
)
target_link_libraries(engine PUBLIC renderer)

target_include_directories(engine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes
)
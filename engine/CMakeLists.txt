project(engine)

add_library(
    engine STATIC
    engine.cpp
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/renderer)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sim)

target_link_libraries(engine PUBLIC renderer)
target_link_libraries(engine PUBLIC sim)

target_include_directories(engine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes
)

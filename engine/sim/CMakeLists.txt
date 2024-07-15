add_library(
    sim STATIC
    sim.cpp includes/sim.hpp
    element.cpp includes/element.hpp
)

target_include_directories(sim 
    PUBLIC includes
    PUBLIC ${CMAKE_SOURCE_DIR}/engine/cell
)

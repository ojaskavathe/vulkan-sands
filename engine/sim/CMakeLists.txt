add_library(
    sim STATIC
    sim.cpp sim.hpp
)
target_link_libraries(engine PRIVATE sim)
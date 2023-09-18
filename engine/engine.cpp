#include <engine.hpp>
#include <renderer.hpp>
#include <sim.hpp>

#include <iostream>
#include <stdexcept>

namespace Engine {    
    Renderer r;

    void run()
    {
        try {
            r.setCellUpdate(sim::updateCell);
            r.run();
        } catch (const std::exception& e) {
            std::cerr << "[Renderer] except: " << e.what() << std::endl;
        }
    }
}
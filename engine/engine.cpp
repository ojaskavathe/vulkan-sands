#include <engine.hpp>
#include <renderer.hpp>
#include <sim.hpp>

#include <iostream>
#include <stdexcept>

namespace Engine {
    void run()
    {
        try {
            Renderer r;
            r.setCellUpdate(sim::updateCell);
            r.run();
        } catch (const std::exception& e) {
            std::cerr << "[Renderer] except: " << e.what() << std::endl;
        }
    }
}
#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include <renderer.hpp>

namespace Engine {
    void run()
    {
        try {
            Renderer r;
            r.run();
        } catch (const std::exception& e) {
            std::cerr << "[Renderer] except: " << e.what() << std::endl;
        }
    }   
}
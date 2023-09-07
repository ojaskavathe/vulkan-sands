#include <string>
#include <cstdlib>
#include <filesystem>

#ifdef _WIN32
    // Collides with std::numeric_limits::max thus we define NOMINMAX
    #define NOMINMAX
    #include <Windows.h>
#elif defined(__linux__)
    #include <unistd.h>
    #include <linux/limits.h>
#endif

namespace utils {
    inline std::filesystem::path getExecutableDir() {
        std::filesystem::path path;
    #ifdef _WIN32
        char buffer[MAX_PATH];
        GetModuleFileName(NULL, buffer, MAX_PATH);
        path = buffer;
    #elif defined(__linux__)
        char buffer[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
        if (len != -1) {
            buffer[len] = '\0';
            path = buffer;
        }
    #else
        #error Unsupported platform
    #endif
        // Extract directory component of path
        path = path.parent_path();
        return path;
    }
}
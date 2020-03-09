#include "RTXApplication.h"

#include <vulkan/vulkan.hpp>

#include <iostream>

int main() {
    RTXApplication app;

    try {
        app.run();
    }
    catch (std::exception & e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
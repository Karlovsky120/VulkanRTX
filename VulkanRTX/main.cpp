#include "RTXApplication.h"

#include <iostream>
#include <stdexcept>

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
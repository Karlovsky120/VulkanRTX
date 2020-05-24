#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#ifdef RTX
#include "RTXApplication.h"
#else
#include "RasterApplication.h"
#endif

#include <iostream>

int main(int argc, char** argv) {
    std::string modelPath = argv[1];

#ifdef RTX
    RTXApplication app(modelPath);
#else
    RasterApplication app(modelPath);
#endif

    try {
        app.run();
    }
    catch (std::exception & e) {
        std::cout << e.what() << std::endl;
        std::cout.flush();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }
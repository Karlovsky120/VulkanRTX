#include "GlobalDefines.h"

#ifdef RTX
#include "RTXApplication.h"
#else
#include "RasterApplication.h"
#endif

#include <iostream>

int main(int argc, char** argv) {

#ifdef RTX
    RTXApplication app;
#else
    RasterApplication app;
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
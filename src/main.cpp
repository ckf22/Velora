#define DEBUG

#include "application.hpp"

#include <stdexcept>
#include <cstdlib>
#include <iostream>

int main() {

    velora::Application app{};
    try{
        app.run();
    } catch(const std::exception& e ){
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return 0;
}

#include "application.hpp"

#include <stdexcept>
#include <cstdlib>
#include <iostream>

int main() {

    velora::Application app{};
    try{
        app.run(20);
    } catch(const std::exception& e ){
        std::cerr << "Fatal error occured:\n" << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return 0;
}

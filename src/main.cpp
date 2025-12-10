#include <iostream>
#include "app.h"

int main() {
    std::cout << "Welcome to ShaderGraph!" << std::endl;
    
    App app;
    app.init(1280, 720, "ShaderGraph");
    app.run();
    
    return 0;
}

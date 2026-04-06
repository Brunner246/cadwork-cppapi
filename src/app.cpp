#include <iostream>
#include <cwapi3d/CwAPI3D.h>

// Minimal function so the DLL has at least one symbol
void app_hello()
{
    std::cout << "Hello from app DLL" << std::endl;
}

CWAPI3D_PLUGIN bool plugin_x64_init(CwAPI3D::ControllerFactory *factory)
{
    std::cout << "Initializing plugin with factory: " << factory << std::endl;
    
    auto* elementController = factory->getElementController();
    elementController->getActiveIdentifiableElementIDs();
    
    return true;
}

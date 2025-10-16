#include "gui/CryptoCoreGUI.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    CryptoCoreGUI app;

    if (!app.init())
    {
        std::cerr << "Failed to initialize CryptoCore GUI" << std::endl;
        return -1;
    }

    app.run();
    return 0;
}
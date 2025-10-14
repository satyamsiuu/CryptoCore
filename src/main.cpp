#include <iostream>
#include <string>

int main()
{
    std::string dir;
    std::cout << "phir aayega enter directory path : ";
    if (!std::getline(std::cin, dir))
        return 1;
    std::cout << "Received directory: " << dir << std::endl;
    return 0;
}

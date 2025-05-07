#include "RPG.h"
#include <iostream>

int main() {
    try {
        std::cout << "Enter your character's name: ";
        std::string name;
        std::getline(std::cin, name);
        
        Game game(name);
        game.start();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

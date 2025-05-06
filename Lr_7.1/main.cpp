#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

class Entity {
protected:
    std::string name;
    int health;
    int level;

public:
    Entity(const std::string& n, int h, int lvl) : name(n), health(h), level(lvl) {}
    virtual ~Entity() {} 

    virtual void display() const {
        std::cout << "Name: " << name << ", Health: " << health << ", Level: " << level << std::endl;
    }

    // Геттеры
    std::string getName() const { return name; }
    int getHealth() const { return health; }
    int getLevel() const { return level; }

    // Виртуальный метод для сохранения данных
    virtual void save(std::ofstream& file) const {
        file << name << "\n" << health << "\n" << level << "\n";
    }

    // Виртуальный метод для загрузки данных
    virtual void load(std::ifstream& file) {
        std::getline(file, name);
        file >> health >> level;
        file.ignore(); 
    }
};

class Character : public Entity {
public:
    Character(const std::string& n, int h, int lvl) : Entity(n, h, lvl) {}

    void display() const override {
        std::cout << "Character: " << name << ", HP: " << health << ", Level: " << level << std::endl;
    }
};


class GameManager {
private:
    std::vector<std::unique_ptr<Entity>> entities;

public:
    void addEntity(Entity* entity) {
        entities.emplace_back(entity);
    }

    // Сохранение в файл
    void saveToFile(const std::string& filename) {
        std::ofstream file(filename);
        if (!file) {
            throw std::runtime_error("Failed to open file for writing.");
        }

        file << entities.size() << "\n";

        
        for (const auto& entity : entities) {
            entity->save(file);
        }

        std::cout << "Data saved successfully to " << filename << std::endl;
    }

  
    void loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            throw std::runtime_error("Failed to open file for reading.");
        }

        entities.clear();

        int entityCount;
        file >> entityCount;
        file.ignore();

        
        for (int i = 0; i < entityCount; ++i) {
            std::unique_ptr<Entity> entity(new Character("", 0, 0)); 
            entity->load(file);
            entities.push_back(std::move(entity));
        }

        std::cout << "Data loaded successfully from " << filename << std::endl;
    }

    void displayAll() const {
        for (const auto& entity : entities) {
            entity->display();
        }
    }
};

int main() {
    try {
        GameManager manager;
        manager.addEntity(new Character("Hero", 100, 1));
        manager.addEntity(new Character("Mage", 80, 2));
        manager.addEntity(new Character("Warrior", 120, 3));

        std::cout << "Original characters:" << std::endl;
        manager.displayAll();

        manager.saveToFile("game_save.txt");

        GameManager loadedManager;
        
        loadedManager.loadFromFile("game_save.txt");

        std::cout << "\nLoaded characters:" << std::endl;
        loadedManager.displayAll();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

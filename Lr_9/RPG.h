#ifndef RPG_H
#define RPG_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <ctime>
#include <stdexcept>
#include <random>

class InvalidHealthException : public std::runtime_error {
public:
    explicit InvalidHealthException(const std::string& msg) : std::runtime_error(msg) {}
};

template<typename T>
class Logger {
private:
    std::ofstream logFile;
    std::string getCurrentTime() const {
        time_t now = time(nullptr);
        char buf[100];
        strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S] ", localtime(&now));
        return std::string(buf);
    }

public:
    explicit Logger(const std::string& filename) : logFile(filename, std::ios::app) {
        if (!logFile.is_open()) throw std::runtime_error("Cannot open log file");
        log("=== Game session started ===");
    }

    ~Logger() { if (logFile.is_open()) logFile.close(); }

    void log(const T& message) {
        logFile << getCurrentTime() << message << std::endl;
    }
};

class Item {
protected:
    std::string name;
    std::string description;
public:
    Item(const std::string& n, const std::string& desc) : name(n), description(desc) {}
    virtual ~Item() = default;
    virtual void use() = 0;
    virtual std::string getType() const = 0;
    std::string getName() const { return name; }
    std::string getDescription() const { return description; }
};

class Weapon : public Item {
    int attackBonus;
public:
    Weapon(const std::string& n, const std::string& desc, int atk) 
        : Item(n, desc), attackBonus(atk) {}
    void use() override { std::cout << "Equipped " << name << " (+" << attackBonus << " attack)\n"; }
    std::string getType() const override { return "Weapon"; }
    int getAttackBonus() const { return attackBonus; }
};

class Potion : public Item {
    int healAmount;
public:
    Potion(const std::string& n, const std::string& desc, int heal) 
        : Item(n, desc), healAmount(heal) {}
    void use() override { std::cout << "Drank " << name << " (heals " << healAmount << " HP)\n"; }
    std::string getType() const override { return "Potion"; }
    int getHealAmount() const { return healAmount; }
};

template<typename T>
class Inventory {
    std::vector<T> items;
public:
    void addItem(const T& item) { items.push_back(item); }
    void removeItem(size_t index) {
        if (index >= items.size()) throw std::out_of_range("Invalid inventory index");
        items.erase(items.begin() + index);
    }
    void display() const {
        for (size_t i = 0; i < items.size(); ++i)
            std::cout << i << ": " << items[i] << std::endl;
    }
};

class Monster {
protected:
    std::string name;
    int health;
    int attack;
    int defense;
public:
    Monster(const std::string& n, int h, int a, int d) : name(n), health(h), attack(a), defense(d) {}
    virtual ~Monster() = default;
    virtual void attackTarget(class Character& target) = 0;
    void takeDamage(int damage) {
        health = std::max(0, health - damage);
        if (health <= 0) throw InvalidHealthException(name + " defeated!");
    }
    virtual void displayInfo() const {
        std::cout << name << " | HP: " << health << " | ATK: " << attack << " | DEF: " << defense << std::endl;
    }
    std::string getName() const { return name; }
    int getHealth() const { return health; }
    int getAttack() const { return attack; }
    int getDefense() const { return defense; }
};

class Goblin : public Monster {
public:
    Goblin() : Monster("Goblin", 30, 8, 2) {}
    void attackTarget(class Character& target) override;
};

class Dragon : public Monster {
public:
    Dragon() : Monster("Dragon", 100, 20, 10) {}
    void attackTarget(class Character& target) override;
};

class Skeleton : public Monster {
public:
    Skeleton() : Monster("Skeleton", 40, 10, 5) {}
    void attackTarget(class Character& target) override;
};

class Character {
    std::string name;
    int maxHealth;
    int health;
    int baseAttack;
    int attack;
    int baseDefense;
    int defense;
    int level;
    int experience;
    Inventory<std::shared_ptr<Item>> inventory;
public:
    Character(const std::string& n, int h = 100, int a = 10, int d = 5)
        : name(n), maxHealth(h), health(h), baseAttack(a), attack(a),
          baseDefense(d), defense(d), level(1), experience(0) {}
    
    void attackEnemy(Monster& enemy);
    void takeDamage(int damage);
    void heal(int amount);
    void gainExperience(int exp);
    void levelUp();
    void addToInventory(std::shared_ptr<Item> item);
    void useItem(const std::string& itemName);
    void displayInfo() const;
    void displayInventory() const;
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);
    
    std::string getName() const { return name; }
    int getHealth() const { return health; }
    int getAttack() const { return attack; }
    int getDefense() const { return defense; }
    bool isAlive() const { return health > 0; }
};

// Класс игры
class Game {
    Character player;
    Logger<std::string> logger;
    std::vector<std::unique_ptr<Monster>> monsters;
    
    void initializeMonsters();
    void explore();
    void battle();
    void findItem();
    void saveGame();
    void loadGame();
public:
    explicit Game(const std::string& playerName);
    void start();
};

#endif 

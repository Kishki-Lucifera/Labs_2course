#include "RPG.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <memory>
#include <random>

// ==================== Character Methods ====================

Character::Character(const std::string& n, int h, int a, int d)
    : name(n), maxHealth(h), health(h), baseAttack(a), attack(a),
      baseDefense(d), defense(d), level(1), experience(0) {}

void Character::attackEnemy(Monster& enemy) {
    int damage = std::max(1, attack - enemy.getDefense() / 2);
    enemy.takeDamage(damage);
    std::cout << name << " attacks " << enemy.getName() << " for " << damage << " damage!\n";
}

void Character::takeDamage(int damage) {
    health = std::max(0, health - damage);
    if (health <= 0) {
        throw InvalidHealthException(name + " has been defeated!");
    }
}

void Character::heal(int amount) {
    health = std::min(maxHealth, health + amount);
    std::cout << name << " heals for " << amount << " HP\n";
}

void Character::gainExperience(int exp) {
    experience += exp;
    if (experience >= level * 100) {
        level++;
        experience -= level * 100;
        maxHealth += 20;
        health = maxHealth;
        baseAttack += 5;
        attack = baseAttack;
        baseDefense += 3;
        defense = baseDefense;
        std::cout << name << " leveled up to level " << level << "!\n";
    }
}

void Character::displayInfo() const {
    std::cout << "=== Character Info ===\n"
              << "Name: " << name << "\n"
              << "HP: " << health << "/" << maxHealth << "\n"
              << "Attack: " << attack << " | Defense: " << defense << "\n"
              << "Level: " << level << " | EXP: " << experience << "/" << level*100 << "\n";
}

void Character::addToInventory(std::shared_ptr<Item> item) {
    inventory.addItem(item);
    std::cout << "Added " << item->getName() << " to inventory.\n";
}

void Character::useItem(const std::string& itemName) {
    auto item = inventory.getItem(itemName);
    if (!item) {
        throw std::runtime_error("Item not found: " + itemName);
    }
    
    if (auto potion = std::dynamic_pointer_cast<Potion>(item)) {
        heal(potion->getHealAmount());
        inventory.removeItem(itemName);
    }
    else if (auto weapon = std::dynamic_pointer_cast<Weapon>(item)) {
        attack = baseAttack + weapon->getAttackBonus();
        std::cout << "Equipped " << weapon->getName() << "!\n";
    }
}

void Character::displayInventory() const {
    std::cout << "=== Inventory ===\n";
    inventory.display();
}

void Character::saveToFile(std::ofstream& out) const {
    out << name << "\n"
        << maxHealth << " " << health << " "
        << baseAttack << " " << attack << " "
        << baseDefense << " " << defense << " "
        << level << " " << experience << "\n";
    
    // Save inventory size and items
    inventory.save(out);
}

void Character::loadFromFile(std::ifstream& in) {
    std::getline(in, name);
    in >> maxHealth >> health 
       >> baseAttack >> attack 
       >> baseDefense >> defense 
       >> level >> experience;
    
    // Load inventory
    inventory.load(in);
}

// ==================== Monster Methods ====================

void Monster::takeDamage(int damage) {
    health = std::max(0, health - damage);
    if (health <= 0) {
        throw InvalidHealthException(name + " defeated!");
    }
}

void Monster::displayInfo() const {
    std::cout << "=== Monster Info ===\n"
              << "Name: " << name << "\n"
              << "HP: " << health << "\n"
              << "Attack: " << attack << " | Defense: " << defense << "\n";
}

// ==================== Specific Monsters ====================

void Goblin::attackTarget(Character& target) {
    int damage = std::max(1, attack - target.getDefense() / 3);
    target.takeDamage(damage);
    std::cout << name << " scratches " << target.getName() << " for " << damage << " damage!\n";
}

void Dragon::attackTarget(Character& target) {
    if (rand() % 5 == 0) { // 20% chance for critical
        int damage = std::max(1, (attack * 2) - target.getDefense() / 2);
        target.takeDamage(damage);
        std::cout << name << " CRITS " << target.getName() << " for " << damage << " damage!\n";
    } else {
        int damage = std::max(1, attack - target.getDefense() / 2);
        target.takeDamage(damage);
        std::cout << name << " attacks " << target.getName() << " for " << damage << " damage!\n";
    }
}

void Skeleton::attackTarget(Character& target) {
    int damage = std::max(1, attack - target.getDefense() / 2);
    target.takeDamage(damage);
    std::cout << name << " hits " << target.getName() << " for " << damage << " damage!\n";
    
    if (rand() % 3 == 0) { // 33% chance for double attack
        int secondDamage = std::max(1, attack - target.getDefense() / 2);
        target.takeDamage(secondDamage);
        std::cout << name << " attacks again for " << secondDamage << " damage!\n";
    }
}

// ==================== Game Methods ====================

Game::Game(const std::string& playerName) 
    : player(playerName), logger("game_log.txt") {
    initializeMonsters();
    logger.log("Game started for player: " + playerName);
    
    // Starter items
    player.addToInventory(std::make_shared<Weapon>("Rusty Sword", "Basic sword", 3));
    player.addToInventory(std::make_shared<Potion>("Health Potion", "Restores 20 HP", 20));
}

void Game::initializeMonsters() {
    monsters.push_back(std::make_unique<Goblin>());
    monsters.push_back(std::make_unique<Dragon>());
    monsters.push_back(std::make_unique<Skeleton>());
}

void Game::start() {
    std::cout << "=== Simple RPG Game ===\n";
    player.displayInfo();
    
    while (player.isAlive()) {
        std::cout << "\nMain Menu:\n"
                  << "1. Explore\n2. Inventory\n3. Save\n4. Load\n5. Quit\n"
                  << "Choose: ";
        
        int choice;
        std::cin >> choice;
        
        try {
            switch (choice) {
                case 1: explore(); break;
                case 2: 
                    player.displayInventory(); 
                    if (!player.getInventory().isEmpty()) {
                        std::cout << "Use item? (name or 'no'): ";
                        std::string input;
                        std::cin >> input;
                        if (input != "no") {
                            player.useItem(input);
                        }
                    }
                    break;
                case 3: saveGame(); break;
                case 4: loadGame(); break;
                case 5: return;
                default: std::cout << "Invalid choice!\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            logger.log("Error: " + std::string(e.what()));
        }
    }
    
    std::cout << "\nGame Over! " << player.getName() << " was defeated.\n";
    logger.log("Game Over - Player defeated");
}

void Game::explore() {
    std::cout << "\n" << player.getName() << " explores the area...\n";
    logger.log(player.getName() + " explores the area");

    // 60% chance to encounter a monster
    if (rand() % 100 < 60) {
        battle();
    } 
    // 30% chance to find an item
    else if (rand() % 100 < 30) {
        findItem();
    }
    else {
        std::cout << "Nothing interesting found.\n";
        logger.log(player.getName() + " found nothing");
    }
}

void Game::battle() {
    if (monsters.empty()) {
        initializeMonsters();
    }

    // Select random monster
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, monsters.size()-1);
    auto& monster = *monsters[dist(gen)];
    
    std::cout << "\nA wild " << monster.getName() << " appears!\n";
    logger.log(player.getName() + " encounters " + monster.getName());

    while (player.isAlive() && monster.isAlive()) {
        std::cout << "\n" << player.getName() << " (HP: " << player.getHealth() << ") vs "
                  << monster.getName() << " (HP: " << monster.getHealth() << ")\n";
        std::cout << "1. Attack\n2. Use Item\n3. Flee\nChoice: ";
        
        int choice;
        std::cin >> choice;

        try {
            switch (choice) {
                case 1:
                    player.attackEnemy(monster);
                    logger.log(player.getName() + " attacks " + monster.getName());
                    break;
                case 2: {
                    player.displayInventory();
                    std::cout << "Enter item name to use: ";
                    std::string itemName;
                    std::cin.ignore();
                    std::getline(std::cin, itemName);
                    player.useItem(itemName);
                    logger.log(player.getName() + " uses " + itemName);
                    break;
                }
                case 3:
                    if (rand() % 2 == 0) {
                        std::cout << "Successfully fled!\n";
                        logger.log(player.getName() + " fled from battle");
                        return;
                    } else {
                        std::cout << "Failed to flee!\n";
                        logger.log(player.getName() + " failed to flee");
                    }
                    break;
                default:
                    std::cout << "Invalid choice! Lost turn.\n";
            }

            if (monster.isAlive()) {
                monster.attackTarget(player);
                logger.log(monster.getName() + " attacks " + player.getName());
            }

        } catch (const InvalidHealthException& e) {
            std::cout << e.what() << "\n";
            logger.log(e.what());
            break;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            logger.log(std::string("Error: ") + e.what());
        }
    }

    if (!monster.isAlive()) {
        int exp = 30 + rand() % 20;
        player.gainExperience(exp);
        std::cout << "Defeated " << monster.getName() << "! Gained " << exp << " XP.\n";
        logger.log(player.getName() + " defeated " + monster.getName() + " and gained " + std::to_string(exp) + " XP");
        
        // 50% chance to get loot
        if (rand() % 2 == 0) {
            findItem();
        }
    }
}

void Game::findItem() {
    int itemType = rand() % 2;
    std::shared_ptr<Item> item;

    if (itemType == 0) { // Weapon
        std::vector<std::string> weapons = {"Iron Sword", "Steel Axe", "Magic Staff"};
        std::vector<std::string> descs = {"Sharp iron blade", "Heavy steel axe", "Staff with magic powers"};
        int index = rand() % weapons.size();
        int bonus = 5 + rand() % 10;
        item = std::make_shared<Weapon>(weapons[index], descs[index], bonus);
    } 
    else { // Potion
        std::vector<std::string> potions = {"Health Potion", "Greater Potion", "Elixir"};
        std::vector<std::string> descs = {"Restores 25 HP", "Restores 50 HP", "Fully restores HP"};
        int index = rand() % potions.size();
        int heal = (index+1)*25; // 25, 50 or 75 HP
        item = std::make_shared<Potion>(potions[index], descs[index], heal);
    }

    player.addToInventory(item);
    std::cout << "Found: " << item->getName() << "!\n";
    logger.log(player.getName() + " found " + item->getName());
}

void Game::saveGame() {
    std::ofstream out("save.txt");
    if (!out) {
        throw std::runtime_error("Cannot open save file");
    }
    player.saveToFile(out);
    logger.log("Game saved");
    std::cout << "Game saved successfully.\n";
}

void Game::loadGame() {
    std::ifstream in("save.txt");
    if (!in) {
        throw std::runtime_error("Cannot open save file");
    }
    player.loadFromFile(in);
    logger.log("Game loaded");
    std::cout << "Game loaded successfully.\n";
    player.displayInfo();
}

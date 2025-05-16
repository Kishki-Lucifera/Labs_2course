#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <ctime>
#include <stdexcept>
#include <random>
#include <algorithm>

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
        struct tm timeinfo;
        char buf[100];

        if (localtime_s(&timeinfo, &now) == 0) {
            strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S] ", &timeinfo);
            return std::string(buf);
        }
        else {
            return "[time_error] ";
        }
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
        : Item(n, desc), attackBonus(atk) {
    }
    void use() override { std::cout << "Equipped " << name << " (+" << attackBonus << " attack)\n"; }
    std::string getType() const override { return "Weapon"; }
    int getAttackBonus() const { return attackBonus; }
};

class Potion : public Item {
    int healAmount;
public:
    Potion(const std::string& n, const std::string& desc, int heal)
        : Item(n, desc), healAmount(heal) {
    }
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
    void removeItem(const std::string& itemName) {
        auto it = std::find_if(items.begin(), items.end(),
            [&itemName](const auto& item) { return item->getName() == itemName; });
        if (it != items.end()) {
            items.erase(it);
        }
        else {
            throw std::runtime_error("Item not found: " + itemName);
        }
    }

    T getItem(const std::string& name) {
        for (const auto& item : items)
            if (item->getName() == name) return item;
        return T{}; 
    }

    bool isEmpty() const { return items.empty(); }

    void display() const {
        if (items.empty()) {
            std::cout << "Inventory is empty.\n";
            return;
        }
        for (size_t i = 0; i < items.size(); ++i) {
            std::cout << i + 1 << ". " << items[i]->getName() << " - " << items[i]->getDescription();
            if (auto potion = std::dynamic_pointer_cast<Potion>(items[i])) {
                std::cout << " (Heals: " << potion->getHealAmount() << " HP)";
            }
            else if (auto weapon = std::dynamic_pointer_cast<Weapon>(items[i])) {
                std::cout << " (+" << weapon->getAttackBonus() << " ATK)";
            }
            std::cout << "\n";
        }
    }

    void save(std::ofstream& out) const {
        out << items.size() << "\n";
        for (const auto& item : items) {
            out << item->getType() << "\n";
            out << item->getName() << "\n";
            out << item->getDescription() << "\n";
            if (auto potion = std::dynamic_pointer_cast<Potion>(item)) {
                out << potion->getHealAmount() << "\n";
            }
            else if (auto weapon = std::dynamic_pointer_cast<Weapon>(item)) {
                out << weapon->getAttackBonus() << "\n";
            }
        }
    }

    void load(std::ifstream& in) {
        items.clear();
        size_t size;
        in >> size;
        in.ignore();

        for (size_t i = 0; i < size; ++i) {
            std::string type, name, desc;
            std::getline(in, type);
            std::getline(in, name);
            std::getline(in, desc);

            if (type == "Potion") {
                int heal;
                in >> heal;
                in.ignore();
                items.push_back(std::make_shared<Potion>(name, desc, heal));
            }
            else if (type == "Weapon") {
                int atk;
                in >> atk;
                in.ignore();
                items.push_back(std::make_shared<Weapon>(name, desc, atk));
            }
        }
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
    void takeDamage(int damage);
    virtual void displayInfo() const;
    std::string getName() const { return name; }
    int getHealth() const { return health; }
    bool isAlive() const { return health > 0; }
    int getAttack() const { return attack; }
    int getDefense() const { return defense; }
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
        baseDefense(d), defense(d), level(1), experience(0) {
    }

    void attackEnemy(Monster& enemy);
    void takeDamage(int damage);
    void heal(int amount);
    void gainExperience(int exp);
    void addToInventory(std::shared_ptr<Item> item);
    void useItem(const std::string& itemName);
    void displayInfo() const;
    void displayInventory() const;
    void saveToFile(std::ofstream& out) const;
    void loadFromFile(std::ifstream& in);

    std::string getName() const { return name; }
    int getHealth() const { return health; }
    int getAttack() const { return attack; }
    int getDefense() const { return defense; }
    bool isAlive() const { return health > 0; }
    Inventory<std::shared_ptr<Item>>& getInventory() { return inventory; }
};

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
        << "Level: " << level << " | EXP: " << experience << "/" << level * 100 << "\n";
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
    inventory.display();
}

void Character::saveToFile(std::ofstream& out) const {
    out << name << "\n"
        << maxHealth << " " << health << " "
        << baseAttack << " " << attack << " "
        << baseDefense << " " << defense << " "
        << level << " " << experience << "\n";

    inventory.save(out);
}

void Character::loadFromFile(std::ifstream& in) {
    std::getline(in, name);
    in >> maxHealth >> health
        >> baseAttack >> attack
        >> baseDefense >> defense
        >> level >> experience;
    in.ignore();

    inventory.load(in);
}

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

class Goblin : public Monster {
public:
    Goblin() : Monster("Goblin", 30, 8, 2) {}
    void attackTarget(Character& target) override;
};

class Dragon : public Monster {
public:
    Dragon() : Monster("Dragon", 100, 20, 10) {}
    void attackTarget(Character& target) override;
};

class Skeleton : public Monster {
public:
    Skeleton() : Monster("Skeleton", 40, 10, 5) {}
    void attackTarget(Character& target) override;
};

void Goblin::attackTarget(Character& target) {
    int damage = std::max(1, attack - target.getDefense() / 3);
    target.takeDamage(damage);
    std::cout << name << " scratches " << target.getName() << " for " << damage << " damage!\n";
}

void Dragon::attackTarget(Character& target) {
    if (rand() % 5 == 0) {
        int damage = std::max(1, (attack * 2) - target.getDefense() / 2);
        target.takeDamage(damage);
        std::cout << name << " CRITS " << target.getName() << " for " << damage << " damage!\n";
    }
    else {
        int damage = std::max(1, attack - target.getDefense() / 2);
        target.takeDamage(damage);
        std::cout << name << " attacks " << target.getName() << " for " << damage << " damage!\n";
    }
}

void Skeleton::attackTarget(Character& target) {
    int damage = std::max(1, attack - target.getDefense() / 2);
    target.takeDamage(damage);
    std::cout << name << " hits " << target.getName() << " for " << damage << " damage!\n";

    if (rand() % 3 == 0) {
        int secondDamage = std::max(1, attack - target.getDefense() / 2);
        target.takeDamage(secondDamage);
        std::cout << name << " attacks again for " << secondDamage << " damage!\n";
    }
}

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

Game::Game(const std::string& playerName)
    : player(playerName), logger("game_log.txt") {
    initializeMonsters();
    logger.log("Game started for player: " + playerName);

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
        }
        catch (const std::exception& e) {
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

    if (rand() % 100 < 60) {
        battle();
    }
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

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, monsters.size() - 1);
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
                }
                else {
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

        }
        catch (const InvalidHealthException& e) {
            std::cout << e.what() << "\n";
            logger.log(e.what());
            break;
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            logger.log(std::string("Error: ") + e.what());
        }
    }

    if (!monster.isAlive()) {
        int exp = 30 + rand() % 20;
        player.gainExperience(exp);
        std::cout << "Defeated " << monster.getName() << "! Gained " << exp << " XP.\n";
        logger.log(player.getName() + " defeated " + monster.getName() + " and gained " + std::to_string(exp) + " XP");

        if (rand() % 2 == 0) {
            findItem();
        }
    }
}

void Game::findItem() {
    int itemType = rand() % 2;
    std::shared_ptr<Item> item;

    if (itemType == 0) {
        std::vector<std::string> weapons = { "Iron Sword", "Steel Axe", "Magic Staff" };
        std::vector<std::string> descs = { "Sharp iron blade", "Heavy steel axe", "Staff with magic powers" };
        int index = rand() % weapons.size();
        int bonus = 5 + rand() % 10;
        item = std::make_shared<Weapon>(weapons[index], descs[index], bonus);
    }
    else {
        std::vector<std::string> potions = { "Health Potion", "Greater Potion", "Elixir" };
        std::vector<std::string> descs = { "Restores 25 HP", "Restores 50 HP", "Fully restores HP" };
        int index = rand() % potions.size();
        int heal = (index + 1) * 25;
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

int main() {
    try {
        std::cout << "Enter your character's name: ";
        std::string name;
        std::getline(std::cin, name);

        Game game(name);
        game.start();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <string>
#include <random>
#include <memory>

using namespace std;

class Entity {
protected:
    string name;      
    int health;       
    int attack;       
    int defense;      

public:
    Entity(string n, int h, int a, int d) : name(n), health(h), attack(a), defense(d) {}
    virtual ~Entity() = default;

    virtual void attackEnemy(Entity& target) {
        int damage = max(0, attack - target.getDefense());
        target.takeDamage(damage);
        cout << name << " атакует " << target.getName() << " и наносит " << damage << " урона!" << endl;
    }

    virtual void displayInfo() const {
        cout << name << " - Здоровье: " << health 
             << ", Атака: " << attack 
             << ", Защита: " << defense << endl;
    }


    void takeDamage(int damage) { health -= damage; }  
    bool isAlive() const { return health > 0; }       
    string getName() const { return name; }           
    int getDefense() const { return defense; }        
    int getHealth() const { return health; }          
    void heal(int amount) { health += amount; }      
};

class Character : public Entity {
public:
    Character(string n, int h, int a, int d) : Entity(n, h, a, d) {}

    void attackEnemy(Entity& target) override {
        int baseDamage = attack - target.getDefense();
        if (baseDamage <= 0) {
            cout << name << " атакует " << target.getName() << ", но безрезультатно!" << endl;
            return;
        }

        // 20% шанс на критический удар
        if (rand() % 100 < 20) {
            int damage = baseDamage * 2;
            target.takeDamage(damage);
            cout << "Критический удар! ";
            cout << name << " атакует " << target.getName() << " и наносит " << damage << " урона!" << endl;
        } else {
            Entity::attackEnemy(target);  
        }
    }

    // Отображение информации о персонаже
    void displayInfo() const override {
        cout << "Персонаж ";
        Entity::displayInfo();
    }
};

class Monster : public Entity {
public:
    Monster(string n, int h, int a, int d) : Entity(n, h, a, d) {}

    void attackEnemy(Entity& target) override {
        int baseDamage = attack - target.getDefense();
        if (baseDamage <= 0) {
            cout << name << " атакует " << target.getName() << ", но безрезультатно!" << endl;
            return;
        }

        // 30% шанс на ядовитую атаку
        if (rand() % 100 < 30) {
            int damage = baseDamage + 5;
            target.takeDamage(damage);
            cout << "Ядовитая атака! ";
            cout << name << " атакует " << target.getName() << " и наносит " << damage << " урона!" << endl;
        } else {
            Entity::attackEnemy(target);  
        }
    }

    // Отображение информации о монстре
    void displayInfo() const override {
        cout << "Монстр ";
        Entity::displayInfo();
    }
};

vector<shared_ptr<Entity>> entities;  
mutex entitiesMutex;                
mutex fightMutex;                  

void generateMonsters() {
    random_device rd;  /
    mt19937 gen(rd()); /
    uniform_int_distribution<> healthDist(30, 70);
    uniform_int_distribution<> attackDist(5, 20);
    uniform_int_distribution<> defenseDist(2, 10);
    vector<string> names = {"Гоблин", "Орк", "Тролль", "Скелет", "Зомби"};

    while (true) {
        this_thread::sleep_for(chrono::seconds(3));  
        
        lock_guard<mutex> lock(entitiesMutex);  
        string name = names[rand() % names.size()];
     
        auto monster = make_shared<Monster>(
            name, 
            healthDist(gen), 
            attackDist(gen), 
            defenseDist(gen)
        );
        entities.push_back(monster);
        cout << "Появился новый монстр: " << name << endl;
    }
}

// Функция проведения боя между персонажем и монстром
void fight(shared_ptr<Entity> hero, shared_ptr<Entity> monster) {
    cout << "\nНачался бой между " << hero->getName() << " и " << monster->getName() << "!\n";
    
    while (hero->isAlive() && monster->isAlive()) {
        this_thread::sleep_for(chrono::milliseconds(500));  
        
        lock_guard<mutex> lock(fightMutex);  
        
        // Ход персонажа
        hero->attackEnemy(*monster);
        if (!monster->isAlive()) {
            cout << monster->getName() << " побежден!\n";
            break;
        }

        // Ход монстра
        monster->attackEnemy(*hero);
        if (!hero->isAlive()) {
            cout << hero->getName() << " побежден!\n";
            break;
        }
    }
    
    // Лечение после победы
    if (hero->isAlive()) {
        hero->heal(20);
        cout << hero->getName() << " восстанавливает 20 здоровья после боя!\n";
    }
}

int main() {
    srand(time(nullptr));  

    auto hero = make_shared<Character>("Герой", 100, 20, 10);
 
    thread monsterGenerator(generateMonsters);
    monsterGenerator.detach();  

    while (hero->isAlive()) {
        this_thread::sleep_for(chrono::seconds(1));  

        shared_ptr<Entity> monster;
        {
            lock_guard<mutex> lock(entitiesMutex);
            if (!entities.empty()) {
                monster = entities.front();  
                entities.erase(entities.begin());  
            }
        }

        if (monster) {
            thread fightThread(fight, hero, monster);
            fightThread.join();  
        }

        cout << "\nТекущее состояние:\n";
        hero->displayInfo();
        {
            lock_guard<mutex> lock(entitiesMutex);
            cout << "Монстров в ожидании: " << entities.size() << "\n";
        }
    }

    cout << "Игра окончена!\n";
    return 0;
}

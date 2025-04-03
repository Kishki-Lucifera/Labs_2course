#include <iostream>
#include <string>

class Character
{
private:
    std::string name; // Приватное поле: имя персонажа
    int health;       // Приватное поле: уровень здоровья
    int attack;       // Приватное поле: уровень атаки
    int defense;      // Приватное поле: уровень защиты

public:
    // Конструктор для инициализации данных
    Character(const std::string &n, int h, int a, int d)
        : name(n), health(h), attack(a), defense(d) {}

    // Метод для получения уровня здоровья
    int getHealth() const
    {
        return health;
    }

    // Метод для вывода информации о персонаже
    void displayInfo() const
    {
        std::cout << "Name: " << name << ", HP: " << health
                  << ", Attack: " << attack << ", Defense: " << defense << std::endl;
    }

    // Метод для атаки другого персонажа
    void attackEnemy(Character &enemy)
    {
        int damage = attack - enemy.defense;
        if (damage > 0)
        {
            enemy.health -= damage;
            std::cout << name << " attacks " << enemy.name << " for " << damage << " damage!" << std::endl;
        }
        else
        {
            std::cout << name << " attacks " << enemy.name << ", but it has no effect!" << std::endl;
        }
    }
    //Метод для увеличения уровня здоровья персонажа на указанное количество, не больше максимального значения 
    void heal(int amount)
    {
        if (health<100)
        {
            health+=amount;
            if (health>100)
            {
                health=100;
            }
            ;
        }
        
    }
    // Метод для уменьшения уровня здоровья на указанное количество, не меньше минимального значения 
    void takeDamage(int amount){
        health-=amount;
        if (health<0){
            health=0;
        }
    }
};

int main()
{
    // Создаем объекты персонажей
    Character hero("Hero", 100, 20, 10);
    Character monster("Goblin", 50, 15, 5);

    // Выводим информацию о персонажах
    hero.displayInfo();
    monster.displayInfo();

    // Герой атакует монстра
    hero.attackEnemy(monster);
    monster.displayInfo();

    // Во время удара герой ранит себя
    hero.takeDamage(10);
    hero.displayInfo();

    //Гоблин выпивает зелье
    monster.heal(15);
    monster.displayInfo();

    return 0;
}

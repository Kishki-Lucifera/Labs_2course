#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>
#include <limits>

using namespace std;

class User {
protected:
    string name;
    int id;
    int accessLevel;

public:
    User(const string& name, int id, int accessLevel) 
        : name(name), id(id), accessLevel(accessLevel) {
        if (name.empty()) throw invalid_argument("ФИО пользователя не может быть пустым");
        if (accessLevel < 0) throw invalid_argument("Уровень доступа не может быть отрицательным");
    }

    virtual ~User() {}

    string getName() const { return name; }
    int getId() const { return id; }
    int getAccessLevel() const { return accessLevel; }

    void setName(const string& newName) { 
        if (newName.empty()) throw invalid_argument("ФИО пользователя не может быть пустым");
        name = newName; 
    }
    void setId(int newId) { id = newId; }
    void setAccessLevel(int level) { 
        if (level < 0) throw invalid_argument("Уровень доступа не может быть отрицательным");
        accessLevel = level; 
    }

    virtual void displayInfo() const {
        cout << "ID: " << setw(5) << id << " | ФИО: " << setw(20) << name 
             << " | Уровень доступа: " << setw(2) << accessLevel;
    }

    virtual void saveToFile(ofstream& out) const {
        out << name << "\n" << id << "\n" << accessLevel << "\n";
    }

    virtual void loadFromFile(ifstream& in) {
        getline(in, name);
        in >> id >> accessLevel;
        in.ignore();
    }

    virtual string getType() const = 0;
    virtual string getAdditionalInfo() const = 0;
};

class Student : public User {
private:
    string group;

public:
    Student(const string& name, int id, int accessLevel, const string& group)
        : User(name, id, accessLevel), group(group) {
        if (group.empty()) throw invalid_argument("Группа не может быть пустой");
    }

    string getGroup() const { return group; }
    void setGroup(const string& newGroup) { 
        if (newGroup.empty()) throw invalid_argument("Группа не может быть пустой");
        group = newGroup; 
    }

    void displayInfo() const override {
        User::displayInfo();
        cout << " | Группа: " << setw(10) << group << " (Студент)\n";
    }

    void saveToFile(ofstream& out) const override {
        out << "Student\n";
        User::saveToFile(out);
        out << group << "\n";
    }

    void loadFromFile(ifstream& in) override {
        User::loadFromFile(in);
        getline(in, group);
    }

    string getType() const override { return "Студент"; }
    string getAdditionalInfo() const override { return group; }
};

class Teacher : public User {
private:
    string department;

public:
    Teacher(const string& name, int id, int accessLevel, const string& department)
        : User(name, id, accessLevel), department(department) {
        if (department.empty()) throw invalid_argument("Кафедра не может быть пустой");
    }

    string getDepartment() const { return department; }
    void setDepartment(const string& newDepartment) { 
        if (newDepartment.empty()) throw invalid_argument("Кафедра не может быть пустой");
        department = newDepartment; 
    }

    void displayInfo() const override {
        User::displayInfo();
        cout << " | Кафедра: " << setw(15) << department << " (Преподаватель)\n";
    }

    void saveToFile(ofstream& out) const override {
        out << "Teacher\n";
        User::saveToFile(out);
        out << department << "\n";
    }

    void loadFromFile(ifstream& in) override {
        User::loadFromFile(in);
        getline(in, department);
    }

    string getType() const override { return "Преподаватель"; }
    string getAdditionalInfo() const override { return department; }
};

class Administrator : public User {
private:
    string position;

public:
    Administrator(const string& name, int id, int accessLevel, const string& position)
        : User(name, id, accessLevel), position(position) {
        if (position.empty()) throw invalid_argument("Должность не может быть пустой");
    }

    string getPosition() const { return position; }
    void setPosition(const string& newPosition) { 
        if (newPosition.empty()) throw invalid_argument("Должность не может быть пустой");
        position = newPosition; 
    }

    void displayInfo() const override {
        User::displayInfo();
        cout << " | Должность: " << setw(15) << position << " (Администратор)\n";
    }

    void saveToFile(ofstream& out) const override {
        out << "Administrator\n";
        User::saveToFile(out);
        out << position << "\n";
    }

    void loadFromFile(ifstream& in) override {
        User::loadFromFile(in);
        getline(in, position);
    }

    string getType() const override { return "Администратор"; }
    string getAdditionalInfo() const override { return position; }
};

class Resource {
private:
    string name;
    int requiredAccessLevel;

public:
    Resource(const string& name, int requiredAccessLevel)
        : name(name), requiredAccessLevel(requiredAccessLevel) {
        if (name.empty()) throw invalid_argument("Название ресурса не может быть пустым");
        if (requiredAccessLevel < 0) throw invalid_argument("Требуемый уровень доступа не может быть отрицательным");
    }

    string getName() const { return name; }
    int getRequiredAccessLevel() const { return requiredAccessLevel; }

    void setName(const string& newName) { 
        if (newName.empty()) throw invalid_argument("Название ресурса не может быть пустым");
        name = newName; 
    }
    void setRequiredAccessLevel(int level) { 
        if (level < 0) throw invalid_argument("Требуемый уровень доступа не может быть отрицательным");
        requiredAccessLevel = level; 
    }

    bool checkAccess(const User& user) const {
        return user.getAccessLevel() >= requiredAccessLevel;
    }

    void displayInfo() const {
        cout << "Ресурс: " << setw(20) << name 
             << " | Требуемый уровень доступа: " << requiredAccessLevel << "\n";
    }

    void saveToFile(ofstream& out) const {
        out << name << "\n" << requiredAccessLevel << "\n";
    }

    void loadFromFile(ifstream& in) {
        getline(in, name);
        in >> requiredAccessLevel;
        in.ignore();
    }
};

template<typename T>
class AccessControlSystem {
private:
    vector<unique_ptr<User>> users;
    vector<T> resources;

public:
    void addUser(unique_ptr<User> user) {
        users.push_back(move(user));
    }

    void addResource(const T& resource) {
        resources.push_back(resource);
    }

    bool checkAccess(int userId, const string& resourceName) const {
        auto userIt = find_if(users.begin(), users.end(), 
            [userId](const auto& user) { return user->getId() == userId; });
        
        auto resourceIt = find_if(resources.begin(), resources.end(), 
            [resourceName](const auto& resource) { return resource.getName() == resourceName; });

        if (userIt == users.end() || resourceIt == resources.end()) {
            return false;
        }

        return resourceIt->checkAccess(**userIt);
    }

    vector<User*> findUsersByName(const string& name) const {
        vector<User*> result;
        for (const auto& user : users) {
            if (user->getName() == name) {
                result.push_back(user.get());
            }
        }
        return result;
    }

    User* findUserById(int id) const {
        auto it = find_if(users.begin(), users.end(), 
            [id](const auto& user) { return user->getId() == id; });
        return it != users.end() ? it->get() : nullptr;
    }

    void sortUsersByName() {
        sort(users.begin(), users.end(), 
            [](const auto& a, const auto& b) { 
                return a->getName() < b->getName(); 
            });
    }

    void sortUsersById() {
        sort(users.begin(), users.end(), 
            [](const auto& a, const auto& b) { 
                return a->getId() < b->getId(); 
            });
    }

    void sortUsersByAccessLevel() {
        sort(users.begin(), users.end(), 
            [](const auto& a, const auto& b) { 
                return a->getAccessLevel() < b->getAccessLevel(); 
            });
    }

    void displayAllUsers() const {
        if (users.empty()) {
            cout << "В системе нет пользователей.\n";
            return;
        }
        cout << "Список всех пользователей:\n";
        cout << "-----------------------------------------------------------------\n";
        for (const auto& user : users) {
            user->displayInfo();
        }
        cout << "-----------------------------------------------------------------\n";
    }

    void displayAllResources() const {
        if (resources.empty()) {
            cout << "В системе нет ресурсов.\n";
            return;
        }
        cout << "Список всех ресурсов:\n";
        cout << "-----------------------------------------------------------------\n";
        for (const auto& resource : resources) {
            resource.displayInfo();
        }
        cout << "-----------------------------------------------------------------\n";
    }

    void saveToFile(const string& filename) const {
        ofstream out(filename);
        if (!out) throw runtime_error("Не удалось открыть файл для записи");

        out << users.size() << "\n";
        for (const auto& user : users) {
            user->saveToFile(out);
        }

        out << resources.size() << "\n";
        for (const auto& resource : resources) {
            resource.saveToFile(out);
        }

        cout << "Данные успешно сохранены в файл '" << filename << "'\n";
    }

    void loadFromFile(const string& filename) {
        ifstream in(filename);
        if (!in) throw runtime_error("Не удалось открыть файл для чтения");

        users.clear();
        resources.clear();

        int userCount;
        in >> userCount;
        in.ignore();

        for (int i = 0; i < userCount; ++i) {
            string userType;
            getline(in, userType);

            unique_ptr<User> user;
            if (userType == "Student") {
                user = make_unique<Student>("", 0, 0, "");
            } else if (userType == "Teacher") {
                user = make_unique<Teacher>("", 0, 0, "");
            } else if (userType == "Administrator") {
                user = make_unique<Administrator>("", 0, 0, "");
            } else {
                throw runtime_error("Неизвестный тип пользователя в файле");
            }

            user->loadFromFile(in);
            users.push_back(move(user));
        }

        int resourceCount;
        in >> resourceCount;
        in.ignore();

        for (int i = 0; i < resourceCount; ++i) {
            T resource("", 0);
            resource.loadFromFile(in);
            resources.push_back(resource);
        }

        cout << "Данные успешно загружены из файла '" << filename << "'\n";
    }

    bool hasUsers() const { return !users.empty(); }
    bool hasResources() const { return !resources.empty(); }
};

void displayMainMenu() {
    cout << "\nСистема контроля доступа университета\n";
    cout << "====================================\n";
    cout << "1. Добавить пользователя\n";
    cout << "2. Добавить ресурс\n";
    cout << "3. Показать всех пользователей\n";
    cout << "4. Показать все ресурсы\n";
    cout << "5. Найти пользователя по ID\n";
    cout << "6. Найти пользователя по имени\n";
    cout << "7. Проверить доступ пользователя к ресурсу\n";
    cout << "8. Сортировка пользователей\n";
    cout << "9. Сохранить данные в файл\n";
    cout << "10. Загрузить данные из файла\n";
    cout << "0. Выход\n";
    cout << "====================================\n";
    cout << "Выберите действие: ";
}

void displayUserTypeMenu() {
    cout << "\nВыберите тип пользователя:\n";
    cout << "1. Студент\n";
    cout << "2. Преподаватель\n";
    cout << "3. Администратор\n";
    cout << "0. Назад\n";
    cout << "Выберите тип: ";
}

void displaySortMenu() {
    cout << "\nВыберите тип сортировки:\n";
    cout << "1. По имени\n";
    cout << "2. По ID\n";
    cout << "3. По уровню доступа\n";
    cout << "0. Назад\n";
    cout << "Выберите тип сортировки: ";
}

unique_ptr<User> createUser() {
    string name, additionalInfo;
    int id, accessLevel;
    int userType;

    while (true) {
        displayUserTypeMenu();
        
        if (!(cin >> userType)) {
            cin.clear(); 
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
            cout << "Ошибка: введите число от 0 до 3\n";
            continue;
        }
        cin.ignore(); 

        if (userType == 0) return nullptr;

        if (userType < 1 || userType > 3) {
            cout << "Ошибка: выберите вариант от 1 до 3 или 0 для отмены\n";
            continue;
        }

        try {
            cout << "Введите ФИО: ";
            getline(cin, name);
            if (name.empty()) throw invalid_argument("ФИО не может быть пустым");

            cout << "Введите ID: ";
            if (!(cin >> id)) throw invalid_argument("ID должен быть числом");
            
            cout << "Введите уровень доступа: ";
            if (!(cin >> accessLevel)) throw invalid_argument("Уровень доступа должен быть числом");
            cin.ignore();

            switch (userType) {
                case 1: 
                    cout << "Введите группу: ";
                    getline(cin, additionalInfo);
                    return make_unique<Student>(name, id, accessLevel, additionalInfo);
                case 2: 
                    cout << "Введите кафедру: ";
                    getline(cin, additionalInfo);
                    return make_unique<Teacher>(name, id, accessLevel, additionalInfo);
                case 3: 
                    cout << "Введите должность: ";
                    getline(cin, additionalInfo);
                    return make_unique<Administrator>(name, id, accessLevel, additionalInfo);
            }
        } catch (const exception& e) {
            cout << "Ошибка: " << e.what() << ". Попробуйте снова.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

Resource createResource() {
    string name;
    int requiredAccessLevel;

    while (true) {
        try {
            cout << "Введите название ресурса: ";
            getline(cin, name);
            if (name.empty()) throw invalid_argument("Название не может быть пустым");

            cout << "Введите требуемый уровень доступа: ";
            cin >> requiredAccessLevel;
            if (cin.fail()) throw invalid_argument("Уровень доступа должен быть числом");
            if (requiredAccessLevel < 0) throw invalid_argument("Уровень доступа не может быть отрицательным");
            cin.ignore();

            return Resource(name, requiredAccessLevel);
        } catch (const exception& e) {
            cout << "Ошибка: " << e.what() << ". Попробуйте снова.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

void runAccessControlSystem() {
    AccessControlSystem<Resource> system;
    int choice;
    string filename = "university_access_system.txt";

    while (true) {
        try {
            displayMainMenu();
            cin >> choice;
            cin.ignore();

            if (choice == 0) break;

            switch (choice) {
                case 1: { // Добавить пользователя
                    auto user = createUser();
                    if (user) {
                        system.addUser(move(user));
                        cout << "Пользователь успешно добавлен.\n";
                    }
                    break;
                }
                case 2: { // Добавить ресурс
                    Resource resource = createResource();
                    system.addResource(resource);
                    cout << "Ресурс успешно добавлен.\n";
                    break;
                }
                case 3: // Показать всех пользователей
                    system.displayAllUsers();
                    break;
                case 4: // Показать все ресурсы
                    system.displayAllResources();
                    break;
                case 5: { // Найти пользователя по ID
                    if (!system.hasUsers()) {
                        cout << "В системе нет пользователей.\n";
                        break;
                    }
                    int id;
                    cout << "Введите ID пользователя: ";
                    cin >> id;
                    cin.ignore();
                    User* user = system.findUserById(id);
                    if (user) {
                        cout << "Найден пользователь:\n";
                        user->displayInfo();
                    } else {
                        cout << "Пользователь с ID " << id << " не найден.\n";
                    }
                    break;
                }
                case 6: { // Найти пользователя по имени
                    if (!system.hasUsers()) {
                        cout << "В системе нет пользователей.\n";
                        break;
                    }
                    string name;
                    cout << "Введите ФИО пользователя: ";
                    getline(cin, name);
                    auto users = system.findUsersByName(name);
                    if (!users.empty()) {
                        cout << "Найдены пользователи:\n";
                        for (const auto& u : users) {
                            u->displayInfo();
                        }
                    } else {
                        cout << "Пользователи с именем '" << name << "' не найдены.\n";
                    }
                    break;
                }
                case 7: { // Проверить доступ
                    if (!system.hasUsers() || !system.hasResources()) {
                        cout << "Для проверки доступа нужны и пользователи, и ресурсы.\n";
                        break;
                    }
                    int userId;
                    string resourceName;
                    cout << "Введите ID пользователя: ";
                    cin >> userId;
                    cin.ignore();
                    cout << "Введите название ресурса: ";
                    getline(cin, resourceName);
                    bool hasAccess = system.checkAccess(userId, resourceName);
                    cout << "Пользователь " << userId << (hasAccess ? " имеет " : " не имеет ")
                         << "доступ к ресурсу '" << resourceName << "'\n";
                    break;
                }
                case 8: { // Сортировка пользователей
                    if (!system.hasUsers()) {
                        cout << "В системе нет пользователей для сортировки.\n";
                        break;
                    }
                    int sortChoice;
                    displaySortMenu();
                    cin >> sortChoice;
                    cin.ignore();
                    switch (sortChoice) {
                        case 1:
                            system.sortUsersByName();
                            cout << "Пользователи отсортированы по имени.\n";
                            break;
                        case 2:
                            system.sortUsersById();
                            cout << "Пользователи отсортированы по ID.\n";
                            break;
                        case 3:
                            system.sortUsersByAccessLevel();
                            cout << "Пользователи отсортированы по уровню доступа.\n";
                            break;
                        case 0:
                            break;
                        default:
                            cout << "Неверный выбор.\n";
                    }
                    system.displayAllUsers();
                    break;
                }
                case 9: // Сохранить в файл
                    try {
                        system.saveToFile(filename);
                    } catch (const exception& e) {
                        cout << "Ошибка при сохранении: " << e.what() << "\n";
                    }
                    break;
                case 10: // Загрузить из файла
                    try {
                        system.loadFromFile(filename);
                    } catch (const exception& e) {
                        cout << "Ошибка при загрузке: " << e.what() << "\n";
                    }
                    break;
                default:
                    cout << "Неверный выбор. Попробуйте снова.\n";
            }
        } catch (const exception& e) {
            cout << "Ошибка: " << e.what() << "\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
    cout << "Работа системы завершена.\n";
}

int main() {
    runAccessControlSystem();
    return 0;
}

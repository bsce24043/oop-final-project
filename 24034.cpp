#include "24034.h"
#include <iostream>
#include <map>
using namespace std;

// Static members for User ID and singleton instance
int User::nextID = 1;
UserManager* UserManager::instance = nullptr;

// ==== USER BASE CLASS IMPLEMENTATION ====
// Constructor: demonstrates Encapsulation
User::User(string name, string role, string username, string password) : name(name), role(role), username(username), password(password) {
    userID = nextID++; // Unique ID generation using static member
}

// Polymorphic method (virtual): to be overridden by derived classes
void User::displayDetails() const {
    cout << "UserID: " << userID << ", Name: " << name << ", Role: " << role << ", Username: " << username << endl;
}

// Getter methods: Encapsulation (controlled access)
int User::getUserID() const { 
    return userID; 
}
string User::getUsername() const { 
    return username; 
}
string User::getRole() const { 
    return role; 
}

// Password verification: Encapsulation + Authentication
bool User::verifyPassword(string entered) const { 
    return password == entered; 
}

// JSON conversion: Serialization for persistence
json User::toJSON() const {
    return json{{"id", userID}, {"name", name}, {"role", role}, {"username", username}, {"password", password}};
}

// JSON parsing: Deserialization + Polymorphism (dynamic role creation)
shared_ptr<User> User::fromJSON(const json& j) {
    string role = j.at("role");
    shared_ptr<User> user;
    // Inheritance: Creating derived object dynamically based on role
    if (role == "Admin"){
        user = make_shared<Admin>(j.at("name"), j.at("username"), j.at("password"));
    }
    else if (role == "Teacher"){
        user = make_shared<Teacher>(j.at("name"), j.at("username"), j.at("password"));
    }
    else{
        user = make_shared<Student>(j.at("name"), j.at("username"), j.at("password"));
    }

    user->userID = j.at("id");  // Restore ID
    if (j.at("id").get<int>() >= nextID){
        nextID = j.at("id").get<int>() + 1;  // Keep ID unique
    }
    return user;
}

// ==== OPERATOR OVERLOADING ====
// Equality based on user ID
bool operator==(const User& u1, const User& u2) {
    return u1.userID == u2.userID;
}

// Stream output overload: Polymorphic output
ostream& operator<<(ostream& out, const User& user) {
    out << "[" << user.userID << "] " << user.role << " - " << user.name << " (" << user.username << ")";
    return out;
}

// ==== DERIVED CLASSES IMPLEMENTATION ====
// Each derived class shows Inheritance from User
Admin::Admin(string name, string username, string password) : User(name, "Admin", username, password) {}
void Admin::displayDetails() const {
    cout << "[Admin] ";  // Role-specific info
    User::displayDetails();  // Polymorphism
}

Teacher::Teacher(string name, string username, string password) : User(name, "Teacher", username, password) {}
void Teacher::displayDetails() const {
    cout << "[Teacher] ";
    User::displayDetails();  // Polymorphism
}

Student::Student(string name, string username, string password) : User(name, "Student", username, password) {}
void Student::displayDetails() const {
    cout << "[Student] ";
    User::displayDetails();  // Polymorphism
}

// ==== LIST MANAGER IMPLEMENTATION ====
// Template class functionality for user list management (could be generalized)

// Composition: ListManager is used within UserManager
void ListManager::addItem(shared_ptr<User> item) {
    items.push_back(item);  // Aggregation: items can exist independently
}

void ListManager::removeItem(int id) {
    items.erase(remove_if(items.begin(), items.end(), [id](shared_ptr<User> u) {
        return u->getUserID() == id;
    }), items.end());
}

// Display all users (Polymorphism: calls virtual displayDetails)
void ListManager::displayAll() const {
    for (const auto& user : items) {
        user->displayDetails();
    }
}

// Accessor for aggregated items
vector<shared_ptr<User>>& ListManager::getAllItems() {
    return items;
}

// ==== USER MANAGER SINGLETON IMPLEMENTATION ====
// Singleton Pattern: ensures only one instance
UserManager* UserManager::getInstance() {
    if (!instance) instance = new UserManager();  // Lazy initialization
    return instance;
}

// Role-Based Object Creation + Aggregation
void UserManager::registerUser(string name, string role, string username, string password) {
    shared_ptr<User> newUser;
    if (role == "Admin") newUser = make_shared<Admin>(name, username, password);
    else if (role == "Teacher") newUser = make_shared<Teacher>(name, username, password);
    else newUser = make_shared<Student>(name, username, password);

    userList.addItem(newUser);  // Composition
    users.push_back(newUser);   // Add to users vector as well
    cout << "User registered successfully.\n";
}

// Simple Authentication + Polymorphic behavior
bool UserManager::loginUser(string username, string password) {
    for (auto& user : userList.getAllItems()) {
        if (user->getUsername() == username && user->verifyPassword(password)) {
            cout << "Login successful.\n";
            user->displayDetails();
            return true;
        }
    }
    cout << "Invalid username or password.\n";
    return false;
}

void UserManager::logoutUser() {
    cout << "Logout not tracked in current implementation.\n";
}

// Admin functionality (Controlled Deletion)
void UserManager::deleteUser(int userID) {
    userList.removeItem(userID);
    
    // Also remove from users vector
    users.erase(remove_if(users.begin(), users.end(), [userID](shared_ptr<User> u) {
        return u->getUserID() == userID;
    }), users.end());
    
    cout << "User deleted.\n";
}

// Modify existing user info (Encapsulation)
void UserManager::updateUser(int userID, string newName) {
    for (auto& user : userList.getAllItems()) {
        if (user->getUserID() == userID) {
            user->name = newName;  // Direct access allowed here for brevity
            
            // Update in users vector as well
            for (auto& u : users) {
                if (u->getUserID() == userID) {
                    u->name = newName;
                }
            }
            
            cout << "User name updated.\n";
            return;
        }
    }
    cout << "User not found.\n";
}

// Overloaded display: All or single user
void UserManager::displayUserInfo(int userID) {
    if (userID == -1) {
        userList.displayAll();  // Composition & Polymorphism
    } else {
        for (auto& user : userList.getAllItems()) {
            if (user->getUserID() == userID) {
                user->displayDetails();  // Polymorphism
                return;
            }
        }
        cout << "User not found.\n";
    }
}

// JSON persistence
void UserManager::saveUsersToFile() {
    ofstream outFile("users.json");
    json j;
    for (const auto& user : userList.getAllItems()) {
        j.push_back(user->toJSON());  // Serialization
    }
    outFile << j.dump(4);
    outFile.close();
}

// JSON loading + Polymorphic restoration
void UserManager::loadUsersFromFile() {
    ifstream inFile("users.json");
    if (!inFile.is_open()) return;
    json j;
    inFile >> j;
    users.clear(); // Clear existing users
    for (const auto& item : j) {
        auto user = User::fromJSON(item);
        userList.addItem(user);  // Polymorphic
        users.push_back(user);   // Add to users vector as well
    }
    inFile.close();
}
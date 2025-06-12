#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include"json.hpp"

using namespace std;
using json = nlohmann::json;

class User {
protected:
    int userID;
    string name;
    string role;
    string username;
    string password;
    static int nextID;

public:
    User(string name, string role, string username, string password);
    virtual void displayDetails() const;
    int getUserID() const;
    string getUsername() const;
    string getRole() const;
    bool verifyPassword(string entered) const;

    virtual json toJSON() const;
    static shared_ptr<User> fromJSON(const json& j);

    friend bool operator==(const User& u1, const User& u2);
    friend ostream& operator<<(ostream& out, const User& user);
    friend class UserManager;
};

class Admin : public User {
public:
    Admin(string name, string username, string password);
    void displayDetails() const override;
};

class Teacher : public User {
public:
    Teacher(string name, string username, string password);
    void displayDetails() const override;
};

class Student : public User {
public:
    Student(string name, string username, string password);
    void displayDetails() const override;
};

class ListManager {
private:
    vector<shared_ptr<User>> items;

public:
    void addItem(shared_ptr<User> item);
    void removeItem(int id);
    void displayAll() const;
    vector<shared_ptr<User>>& getAllItems();
};

class UserManager {
private:
    static UserManager* instance;
    ListManager userList;
    UserManager() = default;
    std::vector<std::shared_ptr<User>> users;

public:
    static UserManager* getInstance();
    const std::vector<std::shared_ptr<User>>& getAllUsers() const {
        return users;
    }
    void registerUser(string name, string role, string username, string password);
    bool loginUser(string username, string password);  // Changed return type to bool
    void logoutUser();
    void deleteUser(int userID);
    void updateUser(int userID, string newInfo);
    void displayUserInfo(int userID = -1);
    void saveUsersToFile();
    void loadUsersFromFile();
    
    // Add this to access userList if needed
    ListManager& getUserList() { return userList; }
};

#endif
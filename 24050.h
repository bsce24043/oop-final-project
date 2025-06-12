// reminder.h
#ifndef REMINDER_MODULE_H
#define REMINDER_MODULE_H

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <memory>
#include "json.hpp"
#include "24043.h" // Exam Module
#include "24034.h" // User Module
using namespace std;
using json = nlohmann::json;

// ==================== EXCEPTION CLASS ====================
class ReminderException : public exception {
private:
    string message;
public:
    ReminderException(const string& msg) : message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

// ==================== FORWARD DECLARATIONS ====================
class Deadline;
class Reminder;

// ==================== TEMPLATE CLASS ====================
template <typename T>
class Logger {
private:
    vector<T> logs;
public:
    void addLog(const T& log) {
        logs.push_back(log);
    }
    
    void displayLogs() const {
        if (logs.empty()) {
            throw ReminderException("No logs available");
        }
        for (const auto& log : logs) {
            cout << "LOG: " << log << endl;
        }
    }
    
    void clearLogs() {
        logs.clear();
    }
};

// ==================== DEADLINE CLASS (Composition) ====================
class Deadline {
private:
    string dueDate;
    int examID; // Association with Exam
public:
    Deadline(const string& date, int eid) : dueDate(date), examID(eid) {
        if (date.empty() || eid <= 0) {
            throw ReminderException("Invalid deadline parameters");
        }
    }
    
    json toJson() const {
        return {{"dueDate", dueDate}, {"examID", examID}};
    }
    
    string getDueDate() const { return dueDate; }
    int getExamID() const { return examID; }
};

// ==================== REMINDER BASE CLASS ====================
class Reminder {
private:
    int reminderID;
    string message;
    unique_ptr<Deadline> deadline; // Composition with unique_ptr
protected:
    bool isSent;
public:
    Reminder(int id, const string& msg, Deadline* dl) : 
        reminderID(id), message(msg), deadline(dl), isSent(false) {
        if (id <= 0 || msg.empty() || !dl) {
            throw ReminderException("Invalid reminder parameters");
        }
    }
    
    virtual ~Reminder() = default;
    
    virtual void display() const {
        cout << "[" << reminderID << "] " << message 
             << " (Due: " << deadline->getDueDate() << ")";
    }
    
    virtual json toJson() const {
        return {
            {"id", reminderID},
            {"message", message},
            {"deadline", deadline->toJson()},
            {"isSent", isSent}
        };
    }
    
    int getID() const { return reminderID; }
    string getMessage() const { return message; }
    bool getIsSent() const { return isSent; }
    void setIsSent(bool sent) { isSent = sent; }
    
    friend ostream& operator<<(ostream& os, const Reminder& r) {
        r.display();
        return os;
    }
    
    friend class ReminderTester;
};

// ==================== PRIORITY REMINDER (Inheritance) ====================
class PriorityReminder : public Reminder {
private:
    int priority;
public:
    PriorityReminder(int id, const string& msg, Deadline* dl, int prio) : 
        Reminder(id, msg, dl), priority(prio) {
        if (prio < 1 || prio > 5) {
            throw ReminderException("Priority must be 1-5");
        }
    }
    
    void display() const override {
        Reminder::display();
        cout << " [PRIORITY: " << priority << "/5]";
    }
    
    json toJson() const override {
        auto j = Reminder::toJson();
        j["priority"] = priority;
        return j;
    }
};

// ==================== REMINDER MANAGER (Singleton) ====================
class ReminderManager {
private:
    static ReminderManager* instance;
    vector<unique_ptr<Reminder>> reminders;
    map<int, bool> sentStatus;
    Logger<string> activityLog;
    
    ReminderManager() = default;
    
public:
    // Singleton pattern
    static ReminderManager* getInstance() {
        if (!instance) {
            instance = new ReminderManager();
        }
        return instance;
    }
    
    // Delete copy constructor and assignment
    ReminderManager(const ReminderManager&) = delete;
    ReminderManager& operator=(const ReminderManager&) = delete;
    
    // Function overloading
    void addReminder(Reminder* r) {
        if (!r) {
            throw ReminderException("Cannot add null reminder");
        }
        reminders.emplace_back(r);
        activityLog.addLog("Added regular reminder ID: " + to_string(r->getID()));
    }
    
    void addReminder(Reminder* r, int priority) {
        try {
            if (!r) throw ReminderException("Null reminder");
            
            auto* pr = dynamic_cast<PriorityReminder*>(r);
            if (!pr) throw ReminderException("Invalid priority reminder cast");
            
            reminders.insert(reminders.begin(), unique_ptr<Reminder>(r));
            activityLog.addLog("Added PRIORITY reminder ID: " + to_string(r->getID()));
        } catch (const exception& e) {
            activityLog.addLog(string("Error: ") + e.what());
            throw;
        }
    }
    
    void sendReminders() {
        if (reminders.empty()) {
            throw ReminderException("No reminders to send");
        }
        for (const auto& r : reminders) {
            if (!r->getIsSent()) {
                cout << "SENDING: " << *r << endl;
                r->setIsSent(true);
                sentStatus[r->getID()] = true;
                activityLog.addLog("Sent reminder ID: " + to_string(r->getID()));
            }
        }
    }
    
    void saveToFile(const string& filename = "reminders.json") {
        try {
            json j;
            for (const auto& r : reminders) {
                j.push_back(r->toJson());
            }
            
            ofstream outFile(filename);
            if (!outFile) {
                throw ReminderException("Failed to open file: " + filename);
            }
            outFile << j.dump(4);
            activityLog.addLog("Saved reminders to " + filename);
        } catch (const exception& e) {
            activityLog.addLog(string("Save failed: ") + e.what());
            throw;
        }
    }
    
    void loadFromFile(const string& filename = "reminders.json") {
    try {
        ifstream inFile(filename);
        if (!inFile) {
            // File doesn't exist yet, create an empty one
            ofstream outFile(filename);
            if (!outFile) {
                throw ReminderException("Failed to create file: " + filename);
            }
            outFile << "[]"; // Empty JSON array
            outFile.close();
            activityLog.addLog("Created new empty reminders file: " + filename);
            return;
        }
        
        json j;
        inFile >> j;
        
        reminders.clear();
        for (const auto& item : j) {
            try {
                auto dl = new Deadline(item["deadline"]["dueDate"], item["deadline"]["examID"]);
                
                if (item.contains("priority")) {
                    reminders.emplace_back(
                        new PriorityReminder(
                            item["id"],
                            item["message"],
                            dl,
                            item["priority"]
                        )
                    );
                } else {
                    reminders.emplace_back(
                        new Reminder(
                            item["id"],
                            item["message"],
                            dl
                        )
                    );
                }
            } catch (...) {
                activityLog.addLog("Warning: Failed to parse reminder from JSON");
                continue;
            }
        }
        activityLog.addLog("Loaded reminders from " + filename);
    } catch (const exception& e) {
        activityLog.addLog(string("Load failed: ") + e.what());
        throw;
    }
}
    
    void displayAll() const {
        if (reminders.empty()) {
            cout << "No reminders available" << endl;
            return;
        }
        
        for (const auto& r : reminders) {
            cout << *r << endl;
        }
    }
    
    void displayActivityLog() const {
        try {
            activityLog.displayLogs();
        } catch (const ReminderException& e) {
            cerr << "Log display error: " << e.what() << endl;
        }
    }
    
    void clearAll() {
        reminders.clear();
        sentStatus.clear();
        activityLog.clearLogs();
    }
};

// Initialize static singleton instance
ReminderManager* ReminderManager::instance = nullptr;

// ==================== FRIEND TESTER CLASS ====================
class ReminderTester {
public:
    static void testReminder(const Reminder& r) {
        cout << "\nTESTING REMINDER (friend class access):\n";
        cout << "ID: " << r.reminderID << "\nMessage: " << r.message 
             << "\nSent: " << (r.isSent ? "Yes" : "No") << endl;
    }
};

#endif // REMINDER_MODULE_H
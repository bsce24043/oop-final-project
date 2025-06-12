#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <limits>
#include "24034.h" // User Management
#include "24043.h" // Exam Management
#include "24052.h" // Exam Session
#include "24002.h" // Grading System
#include "24050.h" // Reminder Module

using namespace std;
using namespace GradingSystemNS;

// Helper functions
void clearScreen() {
    cout << "\033[2J\033[1;1H"; // ANSI escape codes to clear screen
}

void pressEnterToContinue() {
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Global variables to track current user
static int currentUserID = -1;
static string currentUserRole = "";

// Login function

bool loginWithRoleCheck(const string& requiredRole) {
    // If already logged in with appropriate role, no need to login again
    if (currentUserID != -1) {
        if (requiredRole == "Any" || 
            (requiredRole == "Admin/Teacher" && (currentUserRole == "Admin" || currentUserRole == "Teacher")) ||
            (requiredRole == "Teacher" && currentUserRole == "Teacher") ||
            (requiredRole == "Student" && currentUserRole == "Student")) {
            return true;
        }
    }

    UserManager* userManager = UserManager::getInstance();
    // Ensure users are loaded
    userManager->loadUsersFromFile();
    
    // DEBUG: Print all users to verify they are loaded
    cout << "\n[DEBUG] Available users in the system:" << endl;
    int userCount = 0;
    for (auto& user : userManager->getAllUsers()) {
        cout << "  User ID: " << user->getUserID() 
             << ", Username: " << user->getUsername() 
             << ", Role: " << user->getRole() << endl;
        userCount++;
    }
    
    if (userCount == 0) {
        cout << "  [WARNING] No users found in the system! Please create a user first." << endl;
    }
    cout << endl;
    
    string username, password;
    
    cout << "Enter username: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Always clear the input buffer
    getline(cin, username);
    cout << "Enter password: ";
    getline(cin, password);
    
    // DEBUG: Print entered credentials
    cout << "[DEBUG] Attempting login with: Username=" << username << ", Password=" << password << endl;
    
    // Find user in the user list
    for (auto& user : userManager->getAllUsers()) {
        if (user->getUsername() == username) {
            cout << "[DEBUG] Username match found for: " << username << endl;
            
            if (user->verifyPassword(password)) {
                cout << "[DEBUG] Password verified successfully" << endl;
                currentUserID = user->getUserID();
                currentUserRole = user->getRole();
                
                if (requiredRole == "Any" || 
                    (requiredRole == "Admin/Teacher" && (currentUserRole == "Admin" || currentUserRole == "Teacher"))) {
                    cout << "Access granted. Welcome, " << username << "!" << endl;
                    return true;
                } else if (requiredRole == "Teacher" && currentUserRole == "Teacher") {
                    cout << "Access granted. Welcome, Teacher " << username << "!" << endl;
                    return true;
                } else if (requiredRole == "Student" && currentUserRole == "Student") {
                    cout << "Access granted. Welcome, Student " << username << "!" << endl;
                    return true;
                } else {
                    cout << "Access denied. This module requires " << requiredRole << " privileges." << endl;
                    currentUserID = -1;
                    currentUserRole = "";
                    return false;
                }
            } else {
                cout << "[DEBUG] Password verification failed" << endl;
            }
        }
    }
    
    cout << "Invalid username or password.\n";
    return false;
}

// Login function (for main menu)
void loginUser() {
    if (currentUserID != -1) {
        cout << "You are already logged in as " << currentUserRole << " (ID: " << currentUserID << ")" << endl;
        pressEnterToContinue();
        return;
    }
    
    loginWithRoleCheck("Any");
    pressEnterToContinue();
}

// Logout function
void logoutUser() {
    if (currentUserID == -1) {
        cout << "You are not logged in." << endl;
    } else {
        cout << "Logging out..." << endl;
        currentUserID = -1;
        currentUserRole = "";
        cout << "Logged out successfully." << endl;
    }
    pressEnterToContinue();
}

// User Management Menu (accessible to anyone - modified to remove admin-only restriction)
void userManagementMenu() {
    UserManager* userManager = UserManager::getInstance();
    userManager->loadUsersFromFile();

    // Removed admin-only restriction

    while (true) {
        clearScreen();
        cout << "=== USER MANAGEMENT ===" << endl;
        
        // Display logged-in status if someone is logged in
        if (currentUserID != -1) {
            cout << "Logged in as: " << currentUserRole << " (ID: " << currentUserID << ")" << endl;
        } else {
            cout << "No user logged in. You can create users before logging in." << endl;
        }
        
        cout << "1. Register User" << endl;
        cout << "2. Update User" << endl;
        cout << "3. Delete User" << endl;
        cout << "4. Display All Users" << endl;
        cout << "5. Back to Main Menu" << endl;
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;

        switch (choice) {
            case 1: {
                string name, role, username, password;
                cout << "Enter name: ";
                cin.ignore();
                getline(cin, name);
                cout << "Enter role (Admin/Teacher/Student): ";
                getline(cin, role);
                cout << "Enter username: ";
                getline(cin, username);
                cout << "Enter password: ";
                getline(cin, password);
                
                userManager->registerUser(name, role, username, password);
                userManager->saveUsersToFile();
                cout << "User registered successfully." << endl;
                pressEnterToContinue();
                break;
            }
            case 2: {
                int userID;
                string newName;
                cout << "Enter User ID to update: ";
                cin >> userID;
                cout << "Enter new name: ";
                cin.ignore();
                getline(cin, newName);
                
                userManager->updateUser(userID, newName);
                userManager->saveUsersToFile();
                pressEnterToContinue();
                break;
            }
            case 3: {
                int userID;
                cout << "Enter User ID to delete: ";
                cin >> userID;
                
                userManager->deleteUser(userID);
                userManager->saveUsersToFile();
                pressEnterToContinue();
                break;
            }
            case 4: {
                userManager->displayUserInfo();
                pressEnterToContinue();
                break;
            }
            case 5: {
                userManager->saveUsersToFile();
                return;
            }
            default: {
                cout << "Invalid choice. Try again." << endl;
                pressEnterToContinue();
            }
        }
    }
}

// Exam Management Menu (only accessible when logged in as Admin or Teacher)
void examManagementMenu() {
    ExamManager* examManager = ExamManager::getInstance();
    examManager->loadExamsFromFile();

    // Check login and role
    if (!loginWithRoleCheck("Admin/Teacher")) {
        pressEnterToContinue();
        return;
    }

    while (true) {
        clearScreen();
        cout << "=== EXAM MANAGEMENT ===" << endl;
        cout << "Logged in as: " << currentUserRole << " (ID: " << currentUserID << ")" << endl;
        cout << "1. Create Exam" << endl;
        cout << "2. Add Question to Exam" << endl;
        cout << "3. Modify Question" << endl;
        cout << "4. Remove Question" << endl;
        cout << "5. Delete Exam" << endl;
        cout << "6. Display Exam" << endl;
        cout << "7. Display All Exams" << endl;
        cout << "8. Back to Main Menu" << endl;
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;

        switch (choice) {
            case 1: {
                string subject;
                int duration;
                cout << "Enter subject: ";
                cin.ignore();
                getline(cin, subject);
                cout << "Enter duration (minutes): ";
                cin >> duration;
                
                int examID = examManager->createExam(subject, duration);
                cout << "Exam created with ID: " << examID << endl;
                examManager->saveExamsToFile();
                pressEnterToContinue();
                break;
            }
            case 2: {
                int examID;
                string questionText, type, answer;
                cout << "Enter Exam ID: ";
                cin >> examID;
                cout << "Enter question text: ";
                cin.ignore();
                getline(cin, questionText);
                cout << "Enter question type (MCQ/Descriptive): ";
                getline(cin, type);
                cout << "Enter correct answer: ";
                getline(cin, answer);
                
                int qID = examManager->addQuestion(examID, questionText, type, answer);
                cout << "Question added with ID: " << qID << endl;
                examManager->saveExamsToFile();
                pressEnterToContinue();
                break;
            }
            case 3: {
                int examID, questionID;
                string newText;
                cout << "Enter Exam ID: ";
                cin >> examID;
                cout << "Enter Question ID: ";
                cin >> questionID;
                cout << "Enter new question text: ";
                cin.ignore();
                getline(cin, newText);
                
                examManager->modifyQuestion(examID, questionID, newText);
                examManager->saveExamsToFile();
                pressEnterToContinue();
                break;
            }
            case 4: {
                int examID, questionID;
                cout << "Enter Exam ID: ";
                cin >> examID;
                cout << "Enter Question ID: ";
                cin >> questionID;
                
                examManager->removeQuestion(examID, questionID);
                examManager->saveExamsToFile();
                pressEnterToContinue();
                break;
            }
            case 5: {
                int examID;
                cout << "Enter Exam ID to delete: ";
                cin >> examID;
                
                examManager->deleteExam(examID);
                examManager->saveExamsToFile();
                pressEnterToContinue();
                break;
            }
            case 6: {
                int examID;
                cout << "Enter Exam ID to display: ";
                cin >> examID;
                
                examManager->displayExam(examID);
                pressEnterToContinue();
                break;
            }
            case 7: {
                examManager->displayAllExams();
                pressEnterToContinue();
                break;
            }
            case 8: {
                examManager->saveExamsToFile();
                currentUserID = -1;
                currentUserRole = "";
                return;
            }
            default: {
                cout << "Invalid choice. Try again." << endl;
                pressEnterToContinue();
            }
        }
    }
}

// Exam Session Menu (only accessible when logged in as Student)
void examSessionMenu() {
    SessionManager* sessionManager = SessionManager::getInstance();
    ExamManager* examManager = ExamManager::getInstance();
    examManager->loadExamsFromFile();

    // Check login and role
    if (!loginWithRoleCheck("Student")) {
        pressEnterToContinue();
        return;
    }

     while (true) {
        clearScreen();
        cout << "=== EXAM SESSION ===" << endl;
        cout << "1. Start Exam Session" << endl;
        cout << "2. Submit Answer" << endl;
        cout << "3. View Remaining Time" << endl;
        cout << "4. View Exam Questions" << endl;
        cout << "5. Finish Exam" << endl;
        cout << "6. View Exam Results" << endl;
        cout << "7. Display Active Sessions" << endl;
        cout << "8. Back to Main Menu" << endl;
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;

        switch (choice) {
            case 1: {
                int studentID, examID;
                cout << "Enter Student ID: ";
                cin >> studentID;
                cout << "Enter Exam ID: ";
                cin >> examID;
                
                sessionManager->startSession(studentID, examID);
                pressEnterToContinue();
                break;
            }
            case 2: {
                int studentID, examID, questionID;
                string answer;
                cout << "Enter Student ID: ";
                cin >> studentID;
                cout << "Enter Exam ID: ";
                cin >> examID;
                cout << "Enter Question ID: ";
                cin >> questionID;
                cout << "Enter your answer: ";
                cin.ignore();
                getline(cin, answer);
                
                ExamSession* session = sessionManager->getSession(studentID, examID);
                if (session) {
                    session->submitAnswer(questionID, answer);
                } else {
                    cout << "Session not found!" << endl;
                }
                pressEnterToContinue();
                break;
            }
            case 3: {
                int studentID, examID;
                cout << "Enter Student ID: ";
                cin >> studentID;
                cout << "Enter Exam ID: ";
                cin >> examID;
                
                ExamSession* session = sessionManager->getSession(studentID, examID);
                if (session) {
                    session->viewRemainingTime();
                } else {
                    cout << "Session not found!" << endl;
                }
                pressEnterToContinue();
                break;
            }
            case 4: {
                int studentID, examID;
                cout << "Enter Student ID: ";
                cin >> studentID;
                cout << "Enter Exam ID: ";
                cin >> examID;
                
                ExamSession* session = sessionManager->getSession(studentID, examID);
                if (session) {
                    session->displayExamQuestions();
                } else {
                    cout << "Session not found!" << endl;
                }
                pressEnterToContinue();
                break;
            }
            case 5: {
                int studentID, examID;
                cout << "Enter Student ID: ";
                cin >> studentID;
                cout << "Enter Exam ID: ";
                cin >> examID;
                
                sessionManager->endSession(studentID, examID);
                pressEnterToContinue();
                break;
            }
            case 6: {
                int studentID, examID;
                cout << "Enter Student ID: ";
                cin >> studentID;
                cout << "Enter Exam ID: ";
                cin >> examID;
                
                ExamSession* session = sessionManager->getSession(studentID, examID);
                if (session) {
                    session->displayExamResults();
                } else {
                    cout << "Session not found!" << endl;
                }
                pressEnterToContinue();
                break;
            }
            case 7: {
                sessionManager->displayActiveExamSessions();
                pressEnterToContinue();
                break;
            }
            case 8: {
                sessionManager->saveAllSessions();
                return;
            }
            default: {
                cout << "Invalid choice. Try again." << endl;
                pressEnterToContinue();
            }
        }
    }
}

// Grading System Menu (only accessible when logged in as Teacher)
void gradingSystemMenu() {
    GradingSystem<shared_ptr<Result>>* gradingSystem = GradingSystem<shared_ptr<Result>>::getInstance();
    ExamGrader examGrader;

    // Check login and role
    if (!loginWithRoleCheck("Teacher")) {
        pressEnterToContinue();
        return;
    }

    while (true) {
        clearScreen();
        cout << "=== GRADING SYSTEM ===" << endl;
        cout << "Logged in as: Teacher (ID: " << currentUserID << ")" << endl;
        cout << "1. Grade Exam Session" << endl;
        cout << "2. Grade All Completed Sessions" << endl;
        cout << "3. View All Grades" << endl;
        cout << "4. View Student Grades" << endl;
        cout << "5. Generate Report Card" << endl;
        cout << "6. View Report Card" << endl;
        cout << "7. View Exam Statistics" << endl;
        cout << "8. Back to Main Menu" << endl;
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;

        switch (choice) {
            case 1: {
                int studentID, examID;
                cout << "Enter Student ID: ";
                cin >> studentID;
                cout << "Enter Exam ID: ";
                cin >> examID;
                
                SessionManager* sessionManager = SessionManager::getInstance();
                ExamSession* session = sessionManager->getSession(studentID, examID);
                if (session) {
                    examGrader.gradeExamSession(session);
                } else {
                    cout << "Session not found!" << endl;
                }
                pressEnterToContinue();
                break;
            }
            case 2: {
                examGrader.gradeAllCompletedSessions();
                pressEnterToContinue();
                break;
            }
            case 3: {
                gradingSystem->displayGrades();
                pressEnterToContinue();
                break;
            }
            case 4: {
                int studentID;
                cout << "Enter Student ID: ";
                cin >> studentID;
                
                try {
                    auto results = gradingSystem->getStudentResults(studentID);
                    cout << "\n--- Grades for Student " << studentID << " ---\n";
                    for (const auto& result : results) {
                        result->displayDetails();
                    }
                } catch (const GradingException& e) {
                    cout << "Error: " << e.what() << endl;
                }
                pressEnterToContinue();
                break;
            }
            case 5: {
                int studentID;
                cout << "Enter Student ID: ";
                cin >> studentID;
                
                try {
                    gradingSystem->generateReportCard(studentID);
                    auto reportCard = gradingSystem->getReportCard(studentID);
                    reportCard->saveReportToFile();
                    cout << "Report card generated successfully." << endl;
                } catch (const GradingException& e) {
                    cout << "Error: " << e.what() << endl;
                }
                pressEnterToContinue();
                break;
            }
            case 6: {
                int studentID;
                cout << "Enter Student ID: ";
                cin >> studentID;
                
                try {
                    auto reportCard = examGrader.getStudentReportCard(studentID);
                    reportCard->displayReport();
                } catch (const GradingException& e) {
                    cout << "Error: " << e.what() << endl;
                }
                pressEnterToContinue();
                break;
            }
            case 7: {
                int examID;
                cout << "Enter Exam ID: ";
                cin >> examID;
                
                examGrader.displayExamStatistics(examID);
                pressEnterToContinue();
                break;
            }
            case 8: {
                return;
            }
            default: {
                cout << "Invalid choice. Try again." << endl;
                pressEnterToContinue();
            }
        }
    }
}

// Reminder Management Menu (only accessible when logged in as Student)
void reminderManagementMenu() {
    ReminderManager* reminderManager = ReminderManager::getInstance();

    // Check login and role
    if (!loginWithRoleCheck("Student")) {
        pressEnterToContinue();
        return;
    }

    try {
        reminderManager->loadFromFile();
    } catch (const ReminderException& e) {
        cout << "Warning: " << e.what() << endl;
        cout << "Starting with empty reminders." << endl;
    }

    while (true) {
        clearScreen();
        cout << "=== REMINDER MANAGEMENT ===" << endl;
        cout << "Logged in as: Student (ID: " << currentUserID << ")" << endl;
        cout << "1. Add Regular Reminder" << endl;
        cout << "2. Add Priority Reminder" << endl;
        cout << "3. Send All Reminders" << endl;
        cout << "4. Display All Reminders" << endl;
        cout << "5. Display Activity Log" << endl;
        cout << "6. Back to Main Menu" << endl;
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;

        switch (choice) {
            case 1: {
                int id, examID;
                string message, dueDate;
                cout << "Enter Reminder ID: ";
                cin >> id;
                cout << "Enter Exam ID: ";
                cin >> examID;
                cout << "Enter Due Date (YYYY-MM-DD): ";
                cin.ignore();
                getline(cin, dueDate);
                cout << "Enter Message: ";
                getline(cin, message);
                
                try {
                    Deadline* dl = new Deadline(dueDate, examID);
                    Reminder* r = new Reminder(id, message, dl);
                    reminderManager->addReminder(r);
                    cout << "Regular reminder added successfully." << endl;
                } catch (const ReminderException& e) {
                    cout << "Error: " << e.what() << endl;
                }
                pressEnterToContinue();
                break;
            }
            case 2: {
                int id, examID, priority;
                string message, dueDate;
                cout << "Enter Reminder ID: ";
                cin >> id;
                cout << "Enter Exam ID: ";
                cin >> examID;
                cout << "Enter Due Date (YYYY-MM-DD): ";
                cin.ignore();
                getline(cin, dueDate);
                cout << "Enter Message: ";
                getline(cin, message);
                cout << "Enter Priority (1-5): ";
                cin >> priority;
                
                try {
                    Deadline* dl = new Deadline(dueDate, examID);
                    PriorityReminder* pr = new PriorityReminder(id, message, dl, priority);
                    reminderManager->addReminder(pr);
                    cout << "Priority reminder added successfully." << endl;
                } catch (const ReminderException& e) {
                    cout << "Error: " << e.what() << endl;
                }
                pressEnterToContinue();
                break;
            }
            case 3: {
                try {
                    reminderManager->sendReminders();
                    cout << "All reminders sent successfully." << endl;
                } catch (const ReminderException& e) {
                    cout << "Error: " << e.what() << endl;
                }
                pressEnterToContinue();
                break;
            }
            case 4: {
                reminderManager->displayAll();
                pressEnterToContinue();
                break;
            }
            case 5: {
                try {
                    reminderManager->displayActivityLog();
                } catch (const ReminderException& e) {
                    cout << "Error: " << e.what() << endl;
                }
                pressEnterToContinue();
                break;
            }
            case 6: {
                try {
                    reminderManager->saveToFile();
                } catch (const ReminderException& e) {
                    cout << "Error saving reminders: " << e.what() << endl;
                }
                currentUserID = -1;
                currentUserRole = "";
                return;
            }
            default: {
                cout << "Invalid choice. Try again." << endl;
                pressEnterToContinue();
            }
        }
    }
}

// Main Menu
int main() {
    // Initialize managers
    UserManager* userManager = UserManager::getInstance();
    userManager->loadUsersFromFile();
    
    ExamManager* examManager = ExamManager::getInstance();
    examManager->loadExamsFromFile();
    
    SessionManager* sessionManager = SessionManager::getInstance();
    
    GradingSystem<shared_ptr<Result>>* gradingSystem = GradingSystem<shared_ptr<Result>>::getInstance();
    
    ReminderManager* reminderManager = ReminderManager::getInstance();
    try {
        reminderManager->loadFromFile();
    } catch (const ReminderException& e) {
        cout << "Warning: " << e.what() << endl;
        cout << "Starting with empty reminders." << endl;
    }

    while (true) {
        clearScreen();
        cout << "=== EXAM MANAGEMENT SYSTEM ===" << endl;
        if (currentUserID != -1) {
            cout << "Logged in as: " << currentUserRole << " (ID: " << currentUserID << ")" << endl;
        } else {
            cout << "No user logged in" << endl;
        }
        
        // Updated menu description to reflect that User Management is available to anyone
        cout << "1. User Management (Available to everyone)" << endl;
        cout << "2. Login" << endl;
        cout << "3. Logout" << endl;
        cout << "4. Exam Management (Admin/Teacher)" << endl;
        cout << "5. Exam Session (Student)" << endl;
        cout << "6. Grading System (Teacher)" << endl;
        cout << "7. Reminder Management (Student)" << endl;
        cout << "8. Exit" << endl;
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;

        switch (choice) {
            case 1:
                userManagementMenu();
                break;
            case 2:
                loginUser();
                break;
            case 3:
                logoutUser();
                break;
            case 4:
                examManagementMenu();
                break;
            case 5:
                examSessionMenu();
                break;
            case 6:
                gradingSystemMenu();
                break;
            case 7:
                reminderManagementMenu();
                break;
            case 8:
                // Save all data before exiting
                userManager->saveUsersToFile();
                examManager->saveExamsToFile();
                sessionManager->saveAllSessions();
                try {
                    reminderManager->saveToFile();
                } catch (const ReminderException& e) {
                    cout << "Error saving reminders: " << e.what() << endl;
                }
                cout << "All data saved. Exiting..." << endl;
                return 0;
            default:
                cout << "Invalid choice. Try again." << endl;
                pressEnterToContinue();
        }
    }

    return 0;
}
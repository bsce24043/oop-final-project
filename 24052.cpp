#include "24052.h"
#include "24043.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

// Timer class implementation
void Timer::startTimer(int durationMinutes) {
    duration = chrono::minutes(durationMinutes);
    startTime = chrono::steady_clock::now();
    isRunning = true;
}

int Timer::getRemainingTime() {
    if (!isRunning) return 0;
    auto now = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::minutes>(now - startTime);
    int remaining = duration.count() - elapsed.count();
    return remaining > 0 ? remaining : 0;
}

void Timer::pauseTimer() {
    if (isRunning) {
        pausedTime = chrono::steady_clock::now();
        isRunning = false;
    }
}

void Timer::resumeTimer() {
    if (!isRunning) {
        auto now = chrono::steady_clock::now();
        auto pausedDuration = now - pausedTime;
        startTime += pausedDuration;
        isRunning = true;
    }
}

// AnswerSheet class implementation
void AnswerSheet::addAnswer(int questionID, string answer) {
    answers[questionID] = answer;
}

string AnswerSheet::getAnswer(int questionID) const {
    auto it = answers.find(questionID);
    if (it != answers.end()) {
        return it->second;
    }
    return "";
}

void AnswerSheet::updateAnswer(int questionID, string newAnswer) {
    auto it = answers.find(questionID);
    if (it != answers.end()) {
        it->second = newAnswer;
    }
}

void AnswerSheet::removeAnswer(int questionID) {
    answers.erase(questionID);
}

map<int, string> AnswerSheet::getAllAnswers() const {
    return answers;
}

// ExamSession class implementation
ExamSession::ExamSession() 
    : studentID(0), examID(0), sheet(nullptr), timer(nullptr), isFinished(false) {}

ExamSession::ExamSession(int sid, int eid) 
    : studentID(sid), examID(eid), isFinished(false) {
    // Create new answer sheet and timer for this session
    sheet = new AnswerSheet(sid, eid);
    timer = new Timer();
    
    // Load questions from ExamManager
    ExamManager* examManager = ExamManager::getInstance();
    if (examManager->getExam(eid)) {
        examQuestions = examManager->getExamQuestions(eid);
    }
}

ExamSession::~ExamSession() {
    if (sheet) {
        delete sheet;
        sheet = nullptr;
    }
    if (timer) {
        delete timer;
        timer = nullptr;
    }
}

void ExamSession::startExam(int sid, int eid) {
    if (studentID == 0 && examID == 0) {
        studentID = sid;
        examID = eid;
        
        // Create new answer sheet and timer if not already created
        if (!sheet) sheet = new AnswerSheet(sid, eid);
        if (!timer) timer = new Timer();
        
        // Get the exam duration from ExamManager
        ExamManager* examManager = ExamManager::getInstance();
        int duration = examManager->getExamDuration(eid);
        
        // Load questions from ExamManager
        examQuestions = examManager->getExamQuestions(eid);
        
        // Start the timer
        timer->startTimer(duration);
        
        cout << "Exam started for student " << studentID 
             << " with exam ID " << examID 
             << " (Duration: " << duration << " minutes)" << endl;
    } else {
        cout << "This session is already assigned to a student or exam." << endl;
    }
}

void ExamSession::submitAnswer(int questionID, string answer) {
    if (isFinished) {
        cout << "Cannot submit answer: Exam is already finished." << endl;
        return;
    }
    
    if (sheet) {
        sheet->addAnswer(questionID, answer);
        cout << "Answer submitted for question " << questionID << endl;
    } else {
        cout << "Answer sheet not initialized." << endl;
    }
}

void ExamSession::finishExam() {
    if (!isFinished) {
        isFinished = true;
        if (timer) {
            timer->pauseTimer();
        }
        cout << "Exam finished for student " << studentID << endl;
        
        // Automatically save the session when finished
        saveSessionToFile();
    } else {
        cout << "Exam is already finished." << endl;
    }
}

void ExamSession::viewRemainingTime() {
    if (timer) {
        int remaining = timer->getRemainingTime();
        cout << "Remaining time: " << remaining << " minutes" << endl;
    } else {
        cout << "Timer not set." << endl;
    }
}

void ExamSession::displayExamQuestions() {
    cout << "\n--- Exam Questions ---\n";
    for (const auto& question : examQuestions) {
        question->displayQuestion();
        
        // Show current answer if one exists
        if (sheet) {
            string currentAnswer = sheet->getAnswer(question->getQuestionID());
            if (!currentAnswer.empty()) {
                cout << "Your current answer: " << currentAnswer << endl;
            }
        }
        cout << "------------------------\n";
    }
}

void ExamSession::displayExamResults() {
    cout << "\n--- Exam Results for Student " << studentID << " ---\n";
    if (sheet) {
        auto allAnswers = sheet->getAllAnswers();
        for (const auto& question : examQuestions) {
            int qID = question->getQuestionID();
            cout << "Question " << qID << ": " << question->getQuestionText() << endl;
            
            auto it = allAnswers.find(qID);
            if (it != allAnswers.end()) {
                cout << "Your answer: " << it->second << endl;
                cout << "Correct answer: " << question->getCorrectAnswer() << endl;
                
                bool isCorrect = question->checkAnswer(it->second);
                cout << "Result: " << (isCorrect ? "Correct" : "Incorrect") << endl;
            } else {
                cout << "No answer provided" << endl;
            }
            cout << "------------------------\n";
        }
    } else {
        cout << "No answers available." << endl;
    }
}

void ExamSession::saveSessionToFile() const {
    json j;
    j["studentID"] = studentID;
    j["examID"] = examID;
    j["isFinished"] = isFinished;
    
    if (sheet) {
        j["answers"] = sheet->getAllAnswers();
    }
    
    std::ofstream file("session_" + std::to_string(studentID) + "_" + std::to_string(examID) + ".json");
    if (file.is_open()) {
        file << j.dump(4);
        file.close();
        cout << "Session saved to file." << endl;
    } else {
        cout << "Failed to save session to file." << endl;
    }
}

void ExamSession::loadSessionFromFile(int sid, int eid) {
    std::ifstream file("session_" + std::to_string(sid) + "_" + std::to_string(eid) + ".json");
    if (file.is_open()) {
        json j;
        file >> j;
        file.close();
        
        studentID = j["studentID"];
        examID = j["examID"];
        isFinished = j["isFinished"];
        
        // Create answer sheet if needed
        if (!sheet) {
            sheet = new AnswerSheet(studentID, examID);
        }
        
        // Load answers
        auto answersJson = j["answers"];
        for (auto it = answersJson.begin(); it != answersJson.end(); ++it) {
            int qID = stoi(it.key());
            string ans = it.value();
            sheet->addAnswer(qID, ans);
        }
        
        // Load questions from ExamManager
        ExamManager* examManager = ExamManager::getInstance();
        examQuestions = examManager->getExamQuestions(examID);
        
        cout << "Session loaded from file." << endl;
    } else {
        cout << "Failed to load session from file. Creating new session." << endl;
        studentID = sid;
        examID = eid;
        
        if (!sheet) {
            sheet = new AnswerSheet(studentID, examID);
        }
        if (!timer) {
            timer = new Timer();
        }
        
        // Load questions from ExamManager
        ExamManager* examManager = ExamManager::getInstance();
        examQuestions = examManager->getExamQuestions(examID);
    }
}

bool operator==(const ExamSession &s1, const ExamSession &s2) {
    return s1.studentID == s2.studentID && s1.examID == s2.examID;
}

ostream& operator<<(ostream &out, const ExamSession &session) {
    out << "Student ID: " << session.studentID << ", Exam ID: " << session.examID;
    if (session.isFinished) {
        out << " (Finished)";
    } else {
        out << " (In Progress)";
    }
    out << endl;
    
    if (session.sheet) {
        auto answers = session.sheet->getAllAnswers();
        out << "Questions answered: " << answers.size() << endl;
    }
    return out;
}

// SessionManager implementation
SessionManager* SessionManager::instance = nullptr;

void SessionManager::startSession(int studentID, int examID) {
    // Check if a session already exists for this student and exam
    for (auto session : sessions) {
        if (session->getStudentID() == studentID && session->getExamID() == examID) {
            cout << "Session already exists for student " << studentID << " and exam " << examID << endl;
            return;
        }
    }
    
    // Create new session
    ExamSession* newSession = new ExamSession(studentID, examID);
    newSession->startExam(studentID, examID);
    sessions.push_back(newSession);
}

void SessionManager::endSession(int studentID, int examID) {
    for (auto it = sessions.begin(); it != sessions.end(); ++it) {
        if ((*it)->getStudentID() == studentID && (*it)->getExamID() == examID) {
            (*it)->finishExam();
            // We don't delete the session yet as it might be needed for grading
            return;
        }
    }
    cout << "No active session found for student " << studentID << " and exam " << examID << endl;
}

ExamSession* SessionManager::getSession(int studentID, int examID) {
    for (auto session : sessions) {
        if (session->getStudentID() == studentID && session->getExamID() == examID) {
            return session;
        }
    }
    
    // If no existing session, try to load from file
    ExamSession* newSession = new ExamSession();
    newSession->loadSessionFromFile(studentID, examID);
    
    // Add to sessions if successfully loaded
    if (newSession->getStudentID() == studentID && newSession->getExamID() == examID) {
        sessions.push_back(newSession);
        return newSession;
    } else {
        delete newSession;
        return nullptr;
    }
}

void SessionManager::saveAllSessions() {
    for (auto session : sessions) {
        session->saveSessionToFile();
    }
    cout << "All sessions saved." << endl;
}

bool SessionManager::doesSessionExist(int studentID, int examID) {
    for (auto session : sessions) {
        if (session->getStudentID() == studentID && session->getExamID() == examID) {
            return true;
        }
    }
    return false;
}

void SessionManager::displayActiveExamSessions() const {
    cout << "\n--- Active Exam Sessions ---" << endl;
    if (sessions.empty()) {
        cout << "No active sessions." << endl;
        return;
    }
    
    for (auto session : sessions) {
        cout << *session;
    }
}
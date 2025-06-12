#ifndef EXAM_SESSION_H
#define EXAM_SESSION_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <memory>
#include "json.hpp"
#include "24043.h" // Include Exam Module

using namespace std;
using json = nlohmann::json;

// Forward declaration
class ExamSession;

// ---------- ITimer Interface ----------
class ITimer {
public:
    virtual ~ITimer() {}
    virtual void startTimer(int duration) = 0;
    virtual int getRemainingTime() = 0;
    virtual void pauseTimer() = 0;
    virtual void resumeTimer() = 0;
};

// ---------- Timer ----------
class Timer : public ITimer {
private:
    chrono::steady_clock::time_point startTime, pausedTime;
    chrono::minutes duration;
    bool isRunning = false;

public:
    void startTimer(int duration) override;
    int getRemainingTime() override;
    void pauseTimer() override;
    void resumeTimer() override;
};

// ---------- IAnswerSheet Interface ----------
class IAnswerSheet {
public:
    virtual ~IAnswerSheet() {}
    virtual void addAnswer(int questionID, string answer) = 0;
    virtual string getAnswer(int questionID) const = 0;
    virtual void updateAnswer(int questionID, string newAnswer) = 0;
    virtual void removeAnswer(int questionID) = 0;
    virtual map<int, string> getAllAnswers() const = 0;
    virtual int getStudentID() const = 0;
    virtual int getExamID() const = 0;
};

// ---------- AnswerSheet ----------
class AnswerSheet : public IAnswerSheet {
private:
    map<int, string> answers;
    int studentID;
    int examID;

public:
    AnswerSheet(int sid, int eid) : studentID(sid), examID(eid) {}
    
    void addAnswer(int questionID, string answer) override;
    string getAnswer(int questionID) const override;
    void updateAnswer(int questionID, string newAnswer) override;
    void removeAnswer(int questionID) override;
    map<int, string> getAllAnswers() const override;
    int getStudentID() const override { return studentID; }
    int getExamID() const override { return examID; }
};

// ---------- ISession Interface ----------
class ISession {
public:
    virtual ~ISession() {}
    virtual void startExam(int studentID, int examID) = 0;
    virtual void submitAnswer(int questionID, string answer) = 0;
    virtual void finishExam() = 0;
    virtual void viewRemainingTime() = 0;
    virtual void displayExamQuestions() = 0;
    virtual void displayExamResults() = 0;
    virtual void saveSessionToFile() const = 0;
    virtual void loadSessionFromFile(int studentID, int examID) = 0;
    virtual IAnswerSheet* getAnswerSheet() const = 0;
};

// ---------- ExamSession ----------
class ExamSession : public ISession {
private:
    int studentID;
    int examID;
    IAnswerSheet* sheet;
    ITimer* timer;
    vector<shared_ptr<Question>> examQuestions;
    bool isFinished;

public:
    ExamSession();
    ExamSession(int studentID, int examID);
    ~ExamSession();

    void startExam(int studentID, int examID) override;
    void submitAnswer(int questionID, string answer) override;
    void finishExam() override;
    void viewRemainingTime() override;
    void displayExamQuestions() override;
    void displayExamResults() override;
    void saveSessionToFile() const override;
    void loadSessionFromFile(int studentID, int examID) override;
    IAnswerSheet* getAnswerSheet() const override { return sheet; }
    
    bool isExamFinished() const { return isFinished; }
    int getStudentID() const { return studentID; }
    int getExamID() const { return examID; }

    friend bool operator==(const ExamSession &s1, const ExamSession &s2);
    friend ostream& operator<<(ostream &out, const ExamSession &session);
};

// ---------- Singleton Template SessionManager ----------
class SessionManager {
private:
    static SessionManager* instance;
    vector<ExamSession*> sessions;

    SessionManager() {}

public:
    static SessionManager* getInstance() {
        if (!instance) instance = new SessionManager();
        return instance;
    }
    
    void startSession(int studentID, int examID);
    void endSession(int studentID, int examID);
    ExamSession* getSession(int studentID, int examID);
    void saveAllSessions();
    bool doesSessionExist(int studentID, int examID);
    void displayActiveExamSessions() const;
    vector<ExamSession*> getAllSessions() const { return sessions; }
};

#endif // EXAM_SESSION_H
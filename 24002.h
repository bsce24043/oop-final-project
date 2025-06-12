#ifndef GRADING_SYSTEM_H
#define GRADING_SYSTEM_H

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <stdexcept>
#include "json.hpp"
#include "24043.h" // Include Exam Module
#include "24052.h" // Include Exam Session Module

using json = nlohmann::json;
using namespace std;

namespace GradingSystemNS {
    // Custom exception class
    class GradingException : public exception {
        string message;
    public:
        GradingException(const string& msg) : message(msg) {}
        const char* what() const noexcept override {
            return message.c_str();
        }
    };

    // Forward declarations
    class Result;
    class ReportCard;

    // Result class declaration (only the interface)
    class Result {
    protected:
        int studentID;
        int examID;
        int score;
        string examType;

    public:
        Result(int sid, int eid, int sc, string type = "Standard") 
            : studentID(sid), examID(eid), score(sc), examType(type) {}

        virtual ~Result() = default;

        // Pure virtual function making this abstract (polymorphism)
        virtual void displayDetails() const = 0;

        // Getters
        int getStudentID() const { return studentID; }
        int getExamID() const { return examID; }
        int getScore() const { return score; }
        string getExamType() const { return examType; }

        void updateScore(int newScore) {
            if (newScore < 0 || newScore > 100) {
                throw GradingException("Invalid score value");
            }
            score = newScore;
        }

        void saveResultToFile() const;
        void loadResultFromFile(int sid, int eid);

        // Operator overloading
        friend bool operator==(const Result& r1, const Result& r2) {
            return r1.studentID == r2.studentID && r1.examID == r2.examID;
        }

        friend ostream& operator<<(ostream& out, const Result& result) {
            result.displayDetails();
            return out;
        }
    };

    // Derived class for MCQ results (inheritance)
    class MCQResult : public Result {
        int correctAnswers;
        int totalQuestions;
    public:
        MCQResult(int sid, int eid, int correct, int total) 
            : Result(sid, eid, 0, "MCQ"), correctAnswers(correct), totalQuestions(total) {
            score = (correct * 100) / total; // Calculate percentage
        }

        // Function overriding
        void displayDetails() const override {
            cout << "MCQ Exam Result - Student ID: " << studentID 
                 << ", Exam ID: " << examID 
                 << ", Score: " << score << "% (" 
                 << correctAnswers << "/" << totalQuestions << ")" << endl;
        }
        
        int getCorrectAnswers() const { return correctAnswers; }
        int getTotalQuestions() const { return totalQuestions; }
    };

    // Derived class for Descriptive results (inheritance) - renamed from EssayResult
    class DescriptiveResult : public Result {
        string comments;
        map<int, string> detailedFeedback; // Question ID to feedback mapping
    public:
        DescriptiveResult(int sid, int eid, int sc, string cmts = "") 
            : Result(sid, eid, sc, "Descriptive"), comments(cmts) {}

        // Function overriding
        void displayDetails() const override {
            cout << "Descriptive Exam Result - Student ID: " << studentID 
                 << ", Exam ID: " << examID 
                 << ", Score: " << score << "%" << endl
                 << "Teacher's Comments: " << comments << endl;
                 
            if (!detailedFeedback.empty()) {
                cout << "Question-wise Feedback:" << endl;
                for (const auto& [qid, feedback] : detailedFeedback) {
                    cout << "Question " << qid << ": " << feedback << endl;
                }
            }
        }
        
        void addQuestionFeedback(int questionID, const string& feedback) {
            detailedFeedback[questionID] = feedback;
        }
        
        string getComments() const { return comments; }
        void setComments(const string& cmts) { comments = cmts; }
        
        map<int, string> getDetailedFeedback() const { return detailedFeedback; }
    };

    // Implement Result methods after derived classes are defined
    void Result::saveResultToFile() const {
        json j;
        j["studentID"] = studentID;
        j["examID"] = examID;
        j["score"] = score;
        j["examType"] = examType;
        
        // Add additional data based on result type
        if (examType == "MCQ") {
            auto mcqResult = dynamic_cast<const MCQResult*>(this);
            if (mcqResult) {
                j["correctAnswers"] = mcqResult->getCorrectAnswers();
                j["totalQuestions"] = mcqResult->getTotalQuestions();
            }
        } else if (examType == "Descriptive") {
            auto descResult = dynamic_cast<const DescriptiveResult*>(this);
            if (descResult) {
                j["comments"] = descResult->getComments();
                
                // Save detailed feedback
                json feedback = json::object();
                for (const auto& [qid, fb] : descResult->getDetailedFeedback()) {
                    feedback[to_string(qid)] = fb;
                }
                j["detailedFeedback"] = feedback;
            }
        }

        string filename = "result_" + to_string(studentID) + "_" + to_string(examID) + ".json";
        ofstream file(filename);
        if (file.is_open()) {
            file << j.dump(4);
            file.close();
        } else {
            throw GradingException("Failed to save result to file");
        }
    }

    void Result::loadResultFromFile(int sid, int eid) {
        string filename = "result_" + to_string(sid) + "_" + to_string(eid) + ".json";
        ifstream file(filename);
        if (file.is_open()) {
            json j;
            file >> j;
            studentID = j["studentID"];
            examID = j["examID"];
            score = j["score"];
            examType = j["examType"];
            file.close();
        } else {
            throw GradingException("Failed to load result from file");
        }
    }

    // ReportCard class
    class ReportCard {
        int studentID;
        vector<shared_ptr<Result>> results;
        double averageScore;
    public:
        ReportCard(int sid) : studentID(sid), averageScore(0) {}

        void generateReport(int sid);

        void saveReportToFile() const {
            json j;
            j["studentID"] = studentID;
            j["averageScore"] = averageScore;
            
            json resultsJson = json::array();
            for (const auto& res : results) {
                json r;
                r["examID"] = res->getExamID();
                r["score"] = res->getScore();
                r["examType"] = res->getExamType();
                
                if (res->getExamType() == "MCQ") {
                    auto mcqRes = dynamic_pointer_cast<MCQResult>(res);
                    if (mcqRes) {
                        r["correctAnswers"] = mcqRes->getCorrectAnswers();
                        r["totalQuestions"] = mcqRes->getTotalQuestions();
                    }
                } else if (res->getExamType() == "Descriptive") {
                    auto descRes = dynamic_pointer_cast<DescriptiveResult>(res);
                    if (descRes) {
                        r["comments"] = descRes->getComments();
                    }
                }
                
                resultsJson.push_back(r);
            }
            j["results"] = resultsJson;

            string filename = "report_" + to_string(studentID) + ".json";
            ofstream file(filename);
            if (file.is_open()) {
                file << j.dump(4);
                file.close();
            } else {
                throw GradingException("Failed to save report to file");
            }
        }

        void displayReport() const {
            cout << "\n--- Report Card ---\n";
            cout << "Student ID: " << studentID << endl;
            cout << "Average Score: " << averageScore << "%\n";
            cout << "Exam Results:\n";
            for (const auto& res : results) {
                res->displayDetails();
            }
        }
        
        void addResult(shared_ptr<Result> result) {
            results.push_back(result);
            
            // Recalculate average
            double total = 0;
            for (const auto& res : results) {
                total += res->getScore();
            }
            averageScore = results.empty() ? 0 : total / results.size();
        }
        
        vector<shared_ptr<Result>> getResults() const {
            return results;
        }
    };

    // Singleton GradingSystem class
    template <typename T>
    class GradingSystem {
    private:
        static GradingSystem* instance;
        map<int, vector<T>> results; // studentID to results mapping
        map<int, shared_ptr<ReportCard>> reportCards; // studentID to report card

        GradingSystem() {} // Private constructor

    public:
        // Delete copy constructor and assignment operator
        GradingSystem(const GradingSystem&) = delete;
        GradingSystem& operator=(const GradingSystem&) = delete;

        static GradingSystem* getInstance() {
            if (!instance) {
                instance = new GradingSystem();
            }
            return instance;
        }

        void gradeExam(T result) {
            // Handle both Result* and shared_ptr<Result>
            int studentID = result->getStudentID();
            results[studentID].push_back(result);
            
            // Add to report card if it exists
            auto it = reportCards.find(studentID);
            if (it != reportCards.end()) {
                it->second->addResult(result);
            }
            
            cout << "Exam graded for student ID: " << studentID << endl;
        }

        void displayGrades() const {
            cout << "\n--- All Grades ---\n";
            for (const auto& pair : results) {
                cout << "Student ID: " << pair.first << endl;
                for (const auto& result : pair.second) {
                    result->displayDetails();
                }
            }
        }

        vector<T> getStudentResults(int studentID) const {
            auto it = results.find(studentID);
            if (it == results.end()) {
                throw GradingException("Student not found");
            }
            return it->second;
        }

        void generateReportCard(int studentID) {
            auto it = reportCards.find(studentID);
            if (it == reportCards.end()) {
                reportCards[studentID] = make_shared<ReportCard>(studentID);
            }
            reportCards[studentID]->generateReport(studentID);
        }

        shared_ptr<ReportCard> getReportCard(int studentID) {
            auto it = reportCards.find(studentID);
            if (it == reportCards.end()) {
                throw GradingException("Report card not found");
            }
            return it->second;
        }
        
        void clearAllResults() {
            results.clear();
            reportCards.clear();
        }
    };

    // Initialize static member
    template <typename T>
    GradingSystem<T>* GradingSystem<T>::instance = nullptr;

    // Implement ReportCard::generateReport after GradingSystem is defined
    void ReportCard::generateReport(int sid) {
        try {
            auto& gradingSystem = *GradingSystem<shared_ptr<Result>>::getInstance();
            auto studentResults = gradingSystem.getStudentResults(sid);
            
            results.clear();
            double total = 0;
            for (const auto& res : studentResults) {
                results.push_back(res);
                total += res->getScore();
            }
            averageScore = results.empty() ? 0 : total / results.size();
        } catch (const exception& e) {
            throw GradingException(string("Failed to generate report: ") + e.what());
        }
    }

    // ExamGrader class - Primary interface for evaluating exam sessions
    class ExamGrader {
    public:
        // Function to grade an ExamSession directly
        void gradeExamSession(ExamSession* session) {
            if (!session) {
                throw GradingException("Invalid exam session provided for grading");
            }
            
            int studentID = session->getStudentID();
            int examID = session->getExamID();
            
            // Get session answer sheet
            IAnswerSheet* sheet = session->getAnswerSheet();
            if (!sheet) {
                throw GradingException("No answer sheet found in session");
            }
            
            // Get user answers from the answer sheet
            map<int, string> userAnswers = sheet->getAllAnswers();
            
            // Get exam and questions from ExamManager
            ExamManager* examManager = ExamManager::getInstance();
            Exam* exam = examManager->getExam(examID);
            if (!exam) {
                throw GradingException("Exam not found for grading");
            }
            
            // Check answers against correct answers
            map<int, bool> results = exam->checkAnswers(userAnswers);
            
            // Calculate score
            int correctCount = 0;
            int totalQuestions = results.size();
            
            for (const auto& [qID, isCorrect] : results) {
                if (isCorrect) correctCount++;
            }
            
            int percentScore = totalQuestions > 0 ? (correctCount * 100) / totalQuestions : 0;
            
            // Create appropriate result object
            shared_ptr<Result> result;
            
            // Check exam type based on first question
            vector<shared_ptr<Question>> questions = examManager->getExamQuestions(examID);
            bool hasMCQ = false;
            
            for (const auto& q : questions) {
                if (dynamic_cast<MCQ*>(q.get())) {
                    hasMCQ = true;
                    break;
                }
            }
            
            if (hasMCQ) {
                // Create MCQ result
                result = make_shared<MCQResult>(studentID, examID, correctCount, totalQuestions);
            } else {
                // Create Descriptive result
                auto descResult = make_shared<DescriptiveResult>(studentID, examID, percentScore);
                
                // Add detailed feedback for each question
                for (const auto& q : questions) {
                    int qID = q->getQuestionID();
                    string feedback;
                    
                    auto answerIt = userAnswers.find(qID);
                    if (answerIt != userAnswers.end()) {
                        bool isCorrect = q->checkAnswer(answerIt->second);
                        feedback = isCorrect ? 
                            "Correct answer. Full points awarded." : 
                            "Incorrect answer. Expected: " + q->getCorrectAnswer();
                    } else {
                        feedback = "No answer provided.";
                    }
                    
                    descResult->addQuestionFeedback(qID, feedback);
                }
                
                result = descResult;
            }
            
            // Add to grading system
            auto& gradingSystem = *GradingSystem<shared_ptr<Result>>::getInstance();
            gradingSystem.gradeExam(result);
            
            // Save result to file
            result->saveResultToFile();
            
            cout << "Exam graded successfully. Score: " << percentScore << "%" << endl;
        }
        
        // Function to grade all completed sessions
        void gradeAllCompletedSessions() {
            SessionManager* sessionManager = SessionManager::getInstance();
            vector<ExamSession*> allSessions = sessionManager->getAllSessions();
            
            int gradedCount = 0;
            
            for (auto session : allSessions) {
                if (session->isExamFinished()) {
                    try {
                        gradeExamSession(session);
                        gradedCount++;
                    } catch (const exception& e) {
                        cerr << "Error grading session for student " << session->getStudentID() 
                             << ", exam " << session->getExamID() << ": " << e.what() << endl;
                    }
                }
            }
            
            cout << "Graded " << gradedCount << " completed exam sessions." << endl;
        }
        
        // Generate report cards for all students
        void generateAllReportCards() {
            auto& gradingSystem = *GradingSystem<shared_ptr<Result>>::getInstance();
            
            // Collect all student IDs from results
            set<int> allStudentIDs;
            
            for (auto session : SessionManager::getInstance()->getAllSessions()) {
                allStudentIDs.insert(session->getStudentID());
            }
            
            // Generate report cards
            for (int studentID : allStudentIDs) {
                try {
                    gradingSystem.generateReportCard(studentID);
                    auto reportCard = gradingSystem.getReportCard(studentID);
                    reportCard->saveReportToFile();
                } catch (const exception& e) {
                    cerr << "Error generating report for student " << studentID 
                         << ": " << e.what() << endl;
                }
            }
            
            cout << "Generated report cards for " << allStudentIDs.size() << " students." << endl;
        }
        
        // Get a student's report card
        shared_ptr<ReportCard> getStudentReportCard(int studentID) {
            try {
                auto& gradingSystem = *GradingSystem<shared_ptr<Result>>::getInstance();
                return gradingSystem.getReportCard(studentID);
            } catch (const exception& e) {
                throw GradingException(string("Could not retrieve report card: ") + e.what());
            }
        }
        
        // Display exam statistics
        void displayExamStatistics(int examID) {
            auto& gradingSystem = *GradingSystem<shared_ptr<Result>>::getInstance();
            
            // Collect all results for this exam
            vector<shared_ptr<Result>> examResults;
            
            for (auto session : SessionManager::getInstance()->getAllSessions()) {
                if (session->getExamID() == examID && session->isExamFinished()) {
                    try {
                        auto studentResults = gradingSystem.getStudentResults(session->getStudentID());
                        for (auto& result : studentResults) {
                            if (result->getExamID() == examID) {
                                examResults.push_back(result);
                            }
                        }
                    } catch (const exception&) {
                        // Skip if student has no results
                    }
                }
            }
            
            if (examResults.empty()) {
                cout << "No results found for exam ID " << examID << endl;
                return;
            }
            
            // Calculate statistics
            double totalScore = 0;
            int highestScore = 0;
            int lowestScore = 100;
            
            for (const auto& result : examResults) {
                int score = result->getScore();
                totalScore += score;
                
                if (score > highestScore) highestScore = score;
                if (score < lowestScore) lowestScore = score;
            }
            
            double averageScore = totalScore / examResults.size();
            
            // Get exam details
            ExamManager* examManager = ExamManager::getInstance();
            string examSubject = examManager->getExamSubject(examID);
            
            // Display statistics
            cout << "\n--- Exam Statistics for Exam ID " << examID << " ---" << endl;
            cout << "Subject: " << examSubject << endl;
            cout << "Number of Students: " << examResults.size() << endl;
            cout << "Average Score: " << averageScore << "%" << endl;
            cout << "Highest Score: " << highestScore << "%" << endl;
            cout << "Lowest Score: " << lowestScore << "%" << endl;
        }
    };
};

#endif // GRADING_SYSTEM_H
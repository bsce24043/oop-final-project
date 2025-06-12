#ifndef EXAM_MODULE_H
#define EXAM_MODULE_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <memory>
#include <iomanip>
#include <stdexcept>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

class ExamException : public exception {
    string message;
public:
    inline ExamException(const string& msg) : message(msg) {}
    inline const char* what() const noexcept override {
        return message.c_str();
    }
};

class Question {
protected:
    int questionID;
    string questionText;
    string answer;
public:
    Question(int id = 0, string text = "", string ans = "") 
        : questionID(id), questionText(text), answer(ans) {}

    virtual void displayQuestion() const = 0;
    virtual bool checkAnswer(string userAnswer) const = 0;
    virtual json toJson() const = 0;
    
    virtual ~Question() {}
    int getQuestionID() const { return questionID; }
    string getQuestionText() const { return questionText; }
    string getCorrectAnswer() const { return answer; }
    void setQuestionText(string newText) { questionText = newText; }
    void setAnswer(string newAnswer) { answer = newAnswer; }
};

class MCQ : public Question {
    vector<string> options;
public:
    MCQ(int id = 0, string text = "", string ans = "", vector<string> opts = {})
        : Question(id, text, ans), options(opts) {}

    void displayQuestion() const override {
        cout << "Q" << questionID << ": " << questionText << endl;
        for (size_t i = 0; i < options.size(); ++i)
            cout << char('A' + i) << ") " << options[i] << endl;
    }

    bool checkAnswer(string userAnswer) const override {
        return userAnswer == answer;
    }

    json toJson() const override {
        return {
            {"type", "MCQ"},
            {"questionID", questionID},
            {"questionText", questionText},
            {"answer", answer},
            {"options", options}
        };
    }

    explicit operator string() const {
        return "MCQ: " + questionText + " (" + to_string(options.size()) + " options)";
    }

    void setOptions(vector<string> opts) {
        options = opts;
    }
    
    vector<string> getOptions() const {
        return options;
    }
};

class Descriptive : public Question {
public:
    Descriptive(int id = 0, string text = "", string ans = "") 
        : Question(id, text, ans) {}

    void displayQuestion() const override {
        cout << "Q" << questionID << ": " << questionText << " [Descriptive]" << endl;
    }

    bool checkAnswer(string userAnswer) const override {
        return userAnswer == answer;
    }

    json toJson() const override {
        return {
            {"type", "Descriptive"},
            {"questionID", questionID},
            {"questionText", questionText},
            {"answer", answer}
        };
    }
};

class Exam {
    int examID;
    string subject;
    int duration;
    vector<unique_ptr<Question>> questions;
public:
    Exam(int id = 0, string subj = "", int dur = 0) 
        : examID(id), subject(subj), duration(dur) {}

    Exam(Exam&& other) noexcept 
        : examID(other.examID), 
          subject(move(other.subject)),
          duration(other.duration),
          questions(move(other.questions)) {}

    Exam& operator=(Exam&& other) noexcept {
        if (this != &other) {
            examID = other.examID;
            subject = move(other.subject);
            duration = other.duration;
            questions = move(other.questions);
        }
        return *this;
    }

    void addQuestion(unique_ptr<Question> question) {
        if (!question) throw ExamException("Null pointer passed to addQuestion()");
        questions.push_back(move(question));
    }

    void removeQuestion(int questionID) {
        questions.erase(
            remove_if(questions.begin(), questions.end(),
                [questionID](const unique_ptr<Question>& q) {
                    return q->getQuestionID() == questionID;
                }),
            questions.end());
    }

    void modifyQuestion(int questionID, string newText) {
        for (auto& q : questions) {
            if (q->getQuestionID() == questionID) {
                q->setQuestionText(newText);
                return;
            }
        }
        throw ExamException("Question ID not found");
    }

    void displayExam() const {
        cout << "Exam ID: " << examID << ", Subject: " << subject 
             << ", Duration: " << duration << " mins\n";
        for (const auto& q : questions)
            q->displayQuestion();
    }

    int getExamID() const { return examID; }
    string getSubject() const { return subject; }
    int getDuration() const { return duration; }

    vector<shared_ptr<Question>> getQuestionsCopy() const {
        vector<shared_ptr<Question>> questionsCopy;
        for (const auto& q : questions) {
            if (dynamic_cast<MCQ*>(q.get())) {
                auto mcq = dynamic_cast<MCQ*>(q.get());
                questionsCopy.push_back(make_shared<MCQ>(
                    mcq->getQuestionID(), 
                    mcq->getQuestionText(), 
                    mcq->getCorrectAnswer(), 
                    mcq->getOptions()
                ));
            } else if (dynamic_cast<Descriptive*>(q.get())) {
                auto desc = dynamic_cast<Descriptive*>(q.get());
                questionsCopy.push_back(make_shared<Descriptive>(
                    desc->getQuestionID(), 
                    desc->getQuestionText(), 
                    desc->getCorrectAnswer()
                ));
            }
        }
        return questionsCopy;
    }

    map<int, bool> checkAnswers(const map<int, string>& userAnswers) const {
        map<int, bool> results;
        for (const auto& q : questions) {
            auto it = userAnswers.find(q->getQuestionID());
            if (it != userAnswers.end()) {
                results[q->getQuestionID()] = q->checkAnswer(it->second);
            } else {
                results[q->getQuestionID()] = false;
            }
        }
        return results;
    }

    json toJson() const {
        json jQuestions = json::array();
        for (const auto& q : questions)
            jQuestions.push_back(q->toJson());

        return {
            {"examID", examID},
            {"subject", subject},
            {"duration", duration},
            {"questions", jQuestions}
        };
    }

    void loadFromJson(const json& jExam) {
        examID = jExam["examID"];
        subject = jExam["subject"];
        duration = jExam["duration"];
        questions.clear();
        for (auto& jQ : jExam["questions"]) {
            unique_ptr<Question> q;
            if (jQ["type"] == "MCQ") {
                q = make_unique<MCQ>(jQ["questionID"], jQ["questionText"], 
                                   jQ["answer"], jQ["options"].get<vector<string>>());
            } else {
                q = make_unique<Descriptive>(jQ["questionID"], jQ["questionText"], jQ["answer"]);
            }
            questions.push_back(move(q));
        }
    }

    friend ostream& operator<<(ostream& out, const Exam& exam) {
        out << "Exam ID: " << exam.examID << ", Subject: " << exam.subject 
            << ", Duration: " << exam.duration << " mins\n";
        for (const auto& q : exam.questions)
            q->displayQuestion();
        return out;
    }
};

class ExamStatistics {
public:
    void analyzeExam(const Exam& exam) {
        cout << "Analyzing exam ID: " << exam.getExamID() << endl;
    }
};

template <typename T>
class ExamContainer {
    map<int, T> exams;
public:
    void addExam(T&& exam) {
        exams.emplace(exam.getExamID(), move(exam));
    }

    void removeExam(int id) {
        exams.erase(id);
    }

    const map<int, T>& getExams() const {
        return exams;
    }

    T& getExam(int id) {
        auto it = exams.find(id);
        if (it == exams.end())
            throw ExamException("Exam ID not found in container");
        return it->second;
    }

    bool hasExam(int id) const {
        return exams.find(id) != exams.end();
    }
};

class ExamManager {
private:
    static inline ExamManager* instance = nullptr;  // Fixed: inline static member
    
    ExamContainer<Exam> container;
    int currentExamID = 1000;
    int currentQuestionID = 1;

    ExamManager() = default;
    ExamManager(const ExamManager&) = delete;
    ExamManager& operator=(const ExamManager&) = delete;

public:
    static ExamManager* getInstance() {
        if (!instance)
            instance = new ExamManager();
        return instance;
    }

    int getCurrentExamID() const {
        return currentExamID;
    }

    // CONCEPT: Function Overloading
    int createExam(string subject, int duration) {
        int examID = currentExamID++;
        container.addExam(Exam(examID, subject, duration));
        return examID;
    }
    
    int createExam(int teacherID, string subject, int duration) {
        int examID = currentExamID++;
        container.addExam(Exam(examID, subject, duration));
        return examID;
    }

    int addQuestion(int examID, string questionText, string type, string answer) {
        Exam& exam = container.getExam(examID);
        int qID = currentQuestionID++;
        if (type == "MCQ") {
            vector<string> options;
            string opt;
            cin.ignore();
            cout << "Enter 4 options:\n";
            for (int i = 0; i < 4; ++i) {
                cout << "Option " << char('A' + i) << ": ";
                getline(cin, opt);
                options.push_back(opt);
            }
            exam.addQuestion(make_unique<MCQ>(qID, questionText, answer, options));
        } else {
            exam.addQuestion(make_unique<Descriptive>(qID, questionText, answer));
        }
        return qID;
    }

    int addMCQuestion(int examID, string questionText, string answer, vector<string> options) {
        Exam& exam = container.getExam(examID);
        int qID = currentQuestionID++;
        exam.addQuestion(make_unique<MCQ>(qID, questionText, answer, options));
        return qID;
    }

    int addDescriptiveQuestion(int examID, string questionText, string answer) {
        Exam& exam = container.getExam(examID);
        int qID = currentQuestionID++;
        exam.addQuestion(make_unique<Descriptive>(qID, questionText, answer));
        return qID;
    }

    void removeQuestion(int examID, int questionID) {
        Exam& exam = container.getExam(examID);
        exam.removeQuestion(questionID);
    }

    void modifyQuestion(int examID, int questionID, string newText) {
        Exam& exam = container.getExam(examID);
        exam.modifyQuestion(questionID, newText);
    }

    void deleteExam(int examID) {
        container.removeExam(examID);
    }

    void displayExam(int examID) const {
        const Exam& exam = container.getExams().at(examID);
        exam.displayExam();
    }

    void displayAllExams() const {
        for (const auto& [id, exam] : container.getExams())
            cout << exam << endl;
    }

    vector<shared_ptr<Question>> getExamQuestions(int examID) const {
        const Exam& exam = container.getExams().at(examID);
        return exam.getQuestionsCopy();
    }

    map<int, bool> checkExamAnswers(int examID, const map<int, string>& userAnswers) const {
        const Exam& exam = container.getExams().at(examID);
        return exam.checkAnswers(userAnswers);
    }

    Exam* getExam(int examID) {
        try {
            Exam& exam = container.getExam(examID);
            return &exam;
        } catch (const ExamException&) {
            return nullptr;
        }
    }

    int getExamDuration(int examID) const {
        const Exam& exam = container.getExams().at(examID);
        return exam.getDuration();
    }

    string getExamSubject(int examID) const {
        const Exam& exam = container.getExams().at(examID);
        return exam.getSubject();
    }

    void saveExamsToFile() const {
        ofstream outFile("exams.json");
        if (!outFile)
            throw ExamException("Unable to open exams.json for writing");

        json j;
        for (const auto& [id, exam] : container.getExams())
            j.push_back(exam.toJson());

        outFile << setw(4) << j;
        outFile.close();
    }

    void loadExamsFromFile() {
        ifstream inFile("exams.json");
        if (!inFile)
            throw ExamException("Unable to open exams.json for reading");

        json j;
        inFile >> j;
        inFile.close();

        container = ExamContainer<Exam>(); // Clear existing exams
        for (auto& examData : j) {
            Exam exam;
            exam.loadFromJson(examData);
            container.addExam(move(exam));
        }
    }

};

#endif // EXAM_MODULE_H
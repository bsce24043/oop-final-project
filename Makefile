CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I.

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
TARGET = exam_system

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run:
	exam_system_env/bin/python3 exam_system_gui/pyqt_app.py

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean run

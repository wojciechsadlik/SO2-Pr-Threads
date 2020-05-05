CXX = g++
CXXFLAGS = -Wall -g -std=c++17 -lncurses -pthread

main: main.cpp
	$(CXX) $(CXXFLAGS) -o main main.cpp -I.

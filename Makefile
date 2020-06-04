CXX = g++
CXXFLAGS = -Wall -g -std=c++17 -lncurses -pthread

CircSys: main.cpp *.hpp
	$(CXX) $(CXXFLAGS) -o CircSys main.cpp -I.

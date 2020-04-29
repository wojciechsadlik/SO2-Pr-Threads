CXX = g++
CXXFLAGS = -Wall -g -std=c++11 -lncurses -pthread

CirculatorySystem: CirculatorySystem.o
	$(CXX) $(CXXFLAGS) -o CirculatorySystem CirculatorySystem.o

CirculatorySystem.o: CirculatorySystem.cpp
	$(CXX) $(CXXFLAGS) -c CirculatorySystem.cpp

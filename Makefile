CXXFLAGS = -Wall -Werror -Wpedantic -O3 --std=c++11
CXX = g++

all:


wikigrep.out: src/wikigrep.cc
	$(CXX) $(CXXFLAGS) -o wikigrep.out src/wikigrep.cc

fastwikigrep.out: src/fastwikigrep.cc
	$(CXX) $(CXXFLAGS) -o fastwikigrep.out src/fastwikigrep.cc


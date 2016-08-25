CXXFLAGS = -Wall -Werror -Wpedantic -O3 --std=c++11
CXX = g++

all:


wikigrep.out: src/wikigrep.cc
	$(CXX) $(CXXFLAGS) -o wikigrep.out src/wikigrep.cc

fwikigrep.out: src/fwikigrep.cc
	$(CXX) $(CXXFLAGS) -o fwikigrep.out src/fwikigrep.cc


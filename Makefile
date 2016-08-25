.PHONY: all
CXXFLAGS = -Wall -Werror -Wpedantic -O3 --std=c++11
CXX = g++

all:

prod/filtered.xml: prod filterwiki.out raw/enwiki-latest-pages-articles.xml
	time ./filterwiki.out raw/enwiki-latest-pages-articles.xml prod/filtered.xml

prod:
	mkdir prod

wikigrep.out: src/wikigrep.cc
	$(CXX) $(CXXFLAGS) -o wikigrep.out src/wikigrep.cc

fwikigrep.out: src/fwikigrep.cc
	$(CXX) $(CXXFLAGS) -o fwikigrep.out src/fwikigrep.cc

filterwiki.out: src/filterwiki.cc
	$(CXX) $(CXXFLAGS) -o filterwiki.out src/filterwiki.cc

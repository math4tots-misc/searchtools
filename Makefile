.PHONY: all
CXXFLAGS = -Wall -Werror -Wpedantic -O3 --std=c++11
CXX = g++
START = raw/enwiki-latest-pages-articles.xml

all:

# Don't want to accidentally mess up, and have to rebuild.
# Because building can be really slow.
all2: prod/companies.xml wikigrep.out

prod/companies.xml: fwikigrep.out prod/filtered.xml $(START)
	time ./fwikigrep.out \
			"{{Infobox company" \
			raw/enwiki-latest-pages-articles.xml \
			prod/companies.xml

prod/filtered.xml: prod filterwiki.out $(START)
	time ./filterwiki.out raw/enwiki-latest-pages-articles.xml prod/filtered.xml

prod:
	mkdir prod

wikigrep.out: src/wikigrep.cc
	$(CXX) $(CXXFLAGS) -o wikigrep.out src/wikigrep.cc

fwikigrep.out: src/fwikigrep.cc
	$(CXX) $(CXXFLAGS) -o fwikigrep.out src/fwikigrep.cc

filterwiki.out: src/filterwiki.cc
	$(CXX) $(CXXFLAGS) -o filterwiki.out src/filterwiki.cc

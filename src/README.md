# Wiki tools

Some tools for playing with wikipedia xml dumps.

## Sample build and usage
g++ -Wall -Werror --std=c++11 -Wpedantic src/wikigrep.cc -o wikigrep && time ./wikigrep "\b[Ss]oftware [Ee]ngineer\b" summaries/filtered-articles.xml summaries/grep-software-engineers.xml



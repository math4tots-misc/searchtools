# Search Tools

I've been messing with the wikipedia data XML dump.

The file won't fit in memory on my machine, and it's also a bit large to
search with tools like grep.

This is just a collection of misc tools to extract data from large text
files.


## Expected files and directories

Files and directories expected to be available, but not included

raw/enwiki-latest-pages-articles.xml
summaries/



## Interesting results

### flat organization

    Kyumins-MBP:searchtools math4tots$ g++ -O3 -Wall -Werror --std=c++11 -Wpedantic src/wikigrep.cc -o wikigrep.out && time ./wikigrep.out "[Ff]lat organization" summaries/grep-companies.xml bb.xml
    match          0: title = Valve Corporation
    match          1: title = SAS Institute
    match          2: title = Google

    real	0m46.934s
    user	0m46.588s
    sys	0m0.284s

Hmm, so of all wikipedia pages about companies, there are only 3
that mention 'flat organization'.


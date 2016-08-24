// filter-wiki.cc
/*
g++ \
  -Wall -Werror -Wpedantic --std=c++11 -O3 \
  filter-wiki.cc -o filter-wiki && \
time \
./filter-wiki \
  enwiki-latest-pages-articles.xml \
  summaries/filtered-articles.xml

infile = enwiki-latest-pages-articles.xml
outfile = summaries/filtered-articles.xml

real	4m16.598s
user	3m0.644s
sys	1m11.184s
*/

// Unfortunately this doesn't seem to be as effective as I'd hoped.
// The original was 52GB, and the new version is 45GB.

// Hacked together program to extract just the data I care about
// from wikipedia xml dumps.
// -- only keep pages, and of those only keep those that aren't redirects,
// -- for each page, only keep the <title> and <text>.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define INBUF_SIZE (2L << 30)
#define OUTBUF_SIZE (3L << 30)
#define MILLION 1000000

#define STATE_TOP 0
#define STATE_PAGE 1
#define STATE_TEXT 2

static char *linebuf;
static char *titlebuf;

static FILE *fin;
static FILE *fout;

static void run();

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <infile> <outfile>", argv[0]);
  }

  const char *const infn = argv[1];
  const char *const outfn = argv[2];

  fprintf(stderr, "infile = %s\noutfile = %s\n", infn, outfn);

  fin = fopen(infn, "r");
  fout = fopen(outfn, "w");
  setvbuf(fin, nullptr, _IOFBF, INBUF_SIZE);
  setvbuf(fout, nullptr, _IOFBF, OUTBUF_SIZE);

  linebuf = static_cast<char*>(malloc(INBUF_SIZE));
  titlebuf = static_cast<char*>(malloc(INBUF_SIZE));

  run();

  // Not necessary
  // free(linebuf);
  // free(titlebuf);
  // fclose(fin);
  // fclose(fout);
}

static void run() {
  int state = STATE_TOP;
  bool isRedir = false;

  fputs("<root>\n", fout);
  while (!feof(fin)) {
    fgets(linebuf, INBUF_SIZE-1, fin);
    switch(state) {
      case STATE_TOP:
        if (strstr(linebuf, "<page>")) {
          state = STATE_PAGE;
          isRedir = false;
          titlebuf[0] = '\0';
        }
        break;
      case STATE_PAGE:
        if (titlebuf[0] == '\0' && strstr(linebuf, "<title>")) {
          strcpy(titlebuf, linebuf);
        } else if (!isRedir && strstr(linebuf, "<redirect title=")) {
          isRedir = true;
        } else if (!isRedir && strstr(linebuf, "<text xml:space=")) {
          // fprintf(stderr, "Title = %s", titlebuf);
          fputs("  <page>\n", fout);
          fputs(titlebuf, fout);
          fputs(linebuf, fout);
          state = STATE_TEXT;
        } else if (strstr(linebuf, "</page>")) {
          if (!isRedir) {
            fputs("  </page>\n", fout);
          }
          state = STATE_TOP;
        }
        break;
      case STATE_TEXT:
        fputs(linebuf, fout);
        if (strstr(linebuf, "</text>")) {
          state = STATE_PAGE;
        }
        break;
      default: break;
    }
  }
  fputs("</root>", fout);
}

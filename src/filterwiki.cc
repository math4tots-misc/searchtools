// filterwiki.cc
/*
g++ \
  -Wall -Werror -Wpedantic --std=c++11 -O3 \
  filterwiki.cc -o filterwiki && \
time \
./filterwiki \
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

static bool str_starts_with(const char *str, const char *prefix) {
  return strncmp(str, prefix, strlen(prefix)) == 0;
}

static bool is_wikispecial(const char *title) {
  return
      str_starts_with(title, "Media:") ||
      str_starts_with(title, "Special:") ||
      str_starts_with(title, "Talk:") ||
      str_starts_with(title, "User:") ||
      str_starts_with(title, "User talk:") ||
      str_starts_with(title, "Wikipedia:") ||
      str_starts_with(title, "Wikipedia talk:") ||
      str_starts_with(title, "File:") ||
      str_starts_with(title, "File talk:") ||
      str_starts_with(title, "MediaWiki:") ||
      str_starts_with(title, "MediaWiki talk:") ||
      str_starts_with(title, "Template:") ||
      str_starts_with(title, "Template talk:") ||
      str_starts_with(title, "Help:") ||
      str_starts_with(title, "Help talk:") ||
      str_starts_with(title, "Category:") ||
      str_starts_with(title, "Category talk:") ||
      str_starts_with(title, "Portal:") ||
      str_starts_with(title, "Portal talk:") ||
      str_starts_with(title, "Book:") ||
      str_starts_with(title, "Book talk:") ||
      str_starts_with(title, "Draft:") ||
      str_starts_with(title, "Draft talk:") ||
      str_starts_with(title, "Education Program:") ||
      str_starts_with(title, "Education Program talk:") ||
      str_starts_with(title, "TimedText:") ||
      str_starts_with(title, "TimedText talk:") ||
      str_starts_with(title, "Module:") ||
      str_starts_with(title, "Module talk:") ||
      str_starts_with(title, "Gadget:") ||
      str_starts_with(title, "Gadget talk:") ||
      str_starts_with(title, "Gadget definition:") ||
      str_starts_with(title, "Gadget definition talk:") ||
      str_starts_with(title, "Topic:");
}

static void run() {
  int state = STATE_TOP;
  bool isRedir = false;
  int npages = 0, nwritten = 0, nclosed = 0;

  fputs("<root>\n", fout);
  while (!feof(fin)) {
    fgets(linebuf, INBUF_SIZE-1, fin);
    switch(state) {
      case STATE_TOP:
        if (strstr(linebuf, "<page>")) {
          state = STATE_PAGE;
          if (npages%100000 == 0) {
            fprintf(stderr, "npages = %dk\n", npages/1000);
          }
          npages++;
          isRedir = false;
          titlebuf[0] = '\0';
        }
        break;
      case STATE_PAGE:
        if (titlebuf[0] == '\0' && strstr(linebuf, "<title>")) {
          strcpy(titlebuf, linebuf);
          if (is_wikispecial(titlebuf)) {
            state = STATE_TOP;
          }
        } else if (strstr(linebuf, "<redirect title=")) {
          state = STATE_TOP;
        } else if (strstr(linebuf, "<text xml:space=")) {
          fputs("  <page>\n", fout);
          fputs(titlebuf, fout);
          fputs(linebuf, fout);
          nwritten++;
          state = STATE_TEXT;
        } else if (strstr(linebuf, "</page>")) {
          fputs("  </page>\n", fout);
          state = STATE_TOP;
          nclosed++;
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
  fprintf(stderr, "total npages = %d\n", npages);
  fprintf(stderr, "total written = %d\n", nwritten);
  fprintf(stderr, "total closed  = %d\n", nclosed);
  if (nwritten != nclosed) {
    fprintf(stderr, "WARNING: nwritten != nclosed\n");
  }
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <infile> <outfile>", argv[0]);
    return 1;
  }

  const char *const infn = argv[1];
  const char *const outfn = argv[2];

  fprintf(stderr, "infile = %s\noutfile = %s\n", infn, outfn);

  fin = fopen(infn, "r");
  if (!fin) {
    fprintf(stderr, "Could not open file '%s'\n", infn);
    return 1;
  }
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

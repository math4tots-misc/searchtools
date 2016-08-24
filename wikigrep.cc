// wikigrep.cc
// like 'grep' but search Wikipedia articles.
// g++ -Wall -Werror --std=c++11 -Wpedantic src/wikigrep.cc -o wikigrep
// time ./wikigrep "Google" summaries/filtered-articles.xml out.xml
// You can use enwiki-latest-pages-articles.xml
// instead of filtered-articles.xml if you'd like.
// filtered-articles.xml is just a smaller file, so would probably run
// faster.
#include <regex>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

#define INBUF_SIZE (2L << 30)
#define OUTBUF_SIZE (2L << 30)
#define MAX_TEXT_SIZE (1L << 30)
#define MAX_LINE_SIZE (1L << 30)

#define STATE_TOP 0
#define STATE_TEXT 1

static char *linebuf;
static char *titlebuf;
static char *textbuf;

static bool str_starts_with(const char *str, const char *prefix) {
  return strncmp(str, prefix, strlen(prefix)) == 0;
}

static bool is_wikispecial(const char *title) {
  return str_starts_with(title, "Wikipedia:") ||
         str_starts_with(title, "Portal:") ||
         str_starts_with(title, "Category:") ||
         str_starts_with(title, "Template:");
}

static void run(const regex& re, FILE *fin, FILE *fout) {
  int state = STATE_TOP, nmatch = 0;
  char *p, *q, *textbufp;
  while (!feof(fin)) {
    fgets(linebuf, MAX_LINE_SIZE, fin);
    if (state == STATE_TOP) {
      if ((p = strstr(linebuf, "<title>"))) {
        p += 7;  // strlen("<title>");
        q = strstr(p, "</title>");
        *q = '\0';
        strcpy(titlebuf, p);
      } else if ((p = strstr(linebuf, "<text"))) {
        if (!is_wikispecial(titlebuf)) {
          while (*p != '>') {
            p++;
          }
          p++;
          strcpy(textbuf, p);
          textbufp = textbuf + strlen(textbuf);
          state = STATE_TEXT;
        }
      }
    } else {
      if ((p = strstr(linebuf, "</text>"))) {
        *p = '\0';
        state = STATE_TOP;
        strcpy(textbufp, linebuf);
        if (regex_search(textbuf, re)) {
          fprintf(stderr, "match %10d: title = %s\n", nmatch, titlebuf);
          fprintf(fout, "<title>%s</title>\n", titlebuf);
          fprintf(fout, "<text>\n%s\n</text>\n\n", textbuf);
          nmatch++;
        }
      } else {
        strcpy(textbufp, linebuf);
        textbufp += strlen(linebuf);
      }
    }
  }
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <pattern> <infile> <outfile>", argv[0]);
  }
  regex re(argv[1]);
  FILE *fin = fopen(argv[2], "r");
  setvbuf(fin, nullptr, _IOFBF, INBUF_SIZE);
  FILE *fout = fopen(argv[3], "w");
  setvbuf(fout, nullptr, _IOFBF, OUTBUF_SIZE);
  linebuf = static_cast<char*>(malloc(MAX_LINE_SIZE));
  titlebuf = static_cast<char*>(malloc(MAX_TEXT_SIZE));
  textbuf = static_cast<char*>(malloc(MAX_TEXT_SIZE));
  run(re, fin, fout);
}


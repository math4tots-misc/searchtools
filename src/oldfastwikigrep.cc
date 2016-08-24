// fastwikigrep.cc
// like 'grep' but search Wikipedia articles.
// g++ -Wall -Werror --std=c++11 -Wpedantic src/fastwikigrep.cc \
//   -o fastwikigrep
// time ./fastwikigrep "Google" summaries/filtered-articles.xml out.xml
// You can use enwiki-latest-pages-articles.xml
// instead of filtered-articles.xml if you'd like.
// filtered-articles.xml is just a smaller file, so would probably run
// faster.
// Unfortunately, in practice, fastwikigrep doesn't really seem to be
// significantly faster than wikigrep.
#include <regex>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

#define INBUF_SIZE (2L << 30)
#define OUTBUF_SIZE (2L << 30)

#define STATE_TOP 0
#define STATE_TEXT 1

static char *inbuf;
static size_t inbuflen;

static bool str_starts_with(const char *str, const char *prefix) {
  return strncmp(str, prefix, strlen(prefix)) == 0;
}

static bool is_wikispecial(const char *title) {
  return str_starts_with(title, "Wikipedia:") ||
         str_starts_with(title, "Portal:") ||
         str_starts_with(title, "Category:") ||
         str_starts_with(title, "Template:");
}

static void shift(FILE *fin) {
  if (inbuflen < INBUF_SIZE/2) {
    inbuflen = 0;
    inbuf[0] = '\0';
    return;
  }
  if (inbuflen < INBUF_SIZE) {
    inbuflen -= INBUF_SIZE/2;
    memcpy(inbuf, inbuf + INBUF_SIZE/2, inbuflen);
    inbuf[inbuflen] = '\0';
    return;
  }
  memcpy(inbuf, inbuf + INBUF_SIZE/2, INBUF_SIZE/2);
  inbuflen = INBUF_SIZE/2 + fread(inbuf + INBUF_SIZE/2, 1, INBUF_SIZE/2, fin);
  inbuf[inbuflen] = '\0';
}

static void run(const regex& re, FILE *fin, FILE *fout) {
  const char *p;
  int nmatch = 0;

  inbuflen = fread(inbuf, 1, INBUF_SIZE, fin);
  inbuf[inbuflen] = '\0';

  p = inbuf;

  while (1) {
    // We assume no article is so big that they are ever going to
    // be bigger than INBUF_SIZE/2. So as long as we find
    // that the 'title' part of the string starts before
    // inbuf + INBUF_SIZE/2, we'll assume that all of the rest of
    // the article is in the buffer.
    p = strstr(p, "<title>");
    if (p && p >= inbuf + INBUF_SIZE/2) {
      shift(fin);
      p = strstr(inbuf, "<title>");
    }

    if (!p) {
      break;
    }

    p += 7;  // strlen("<title>");
    const char *const begin_title = p;
    const char *const end_title = strstr(p, "</title>");
    p = end_title + 8;  // strlen("</title>");

    if (is_wikispecial(begin_title)) {
      continue;
    }

    p = strstr(p, "<text ");
    while (*p != '>') {
      p++;
    }
    p++;
    const char *const begin_text = p;
    const char *const end_text = strstr(p, "</text>");

    if (regex_search(begin_text, end_text, re)) {
      fprintf(stderr, "match %10d: title = ", nmatch);
      fwrite(begin_title, 1, end_title - begin_title, stderr);
      fputs("\n", stderr);

      fputs("<title>", fout);
      fwrite(begin_title, 1, end_title - begin_title, fout);
      fputs("</title>\n", fout);
      fputs("<text>", fout);
      fwrite(begin_text, 1, end_text - begin_text, fout);
      fputs("</text>", fout);
      nmatch++;
    }
  }
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <pattern> <infile> <outfile>", argv[0]);
  }
  regex re(argv[1]);
  FILE *fin = fopen(argv[2], "r");
  FILE *fout = fopen(argv[3], "w");
  setvbuf(fout, nullptr, _IOFBF, OUTBUF_SIZE);
  inbuf = static_cast<char*>(malloc(INBUF_SIZE+1));
  run(re, fin, fout);
}


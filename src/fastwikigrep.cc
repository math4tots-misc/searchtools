// fastwikigrep.cc
// Huh, this actually seems slower than wikigrep.cc... Not sure why...
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
static char *outbuf;
static char *outbufp;

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

static void flush(FILE *fout) {
  fwrite(outbuf, 1, outbufp - outbuf, fout);
  outbufp = outbuf;
}

static void run(const regex& re, FILE *fin, FILE *fout) {
  const char *p;
  int nmatch = 0;
  int nshift = 0;

  inbuflen = fread(inbuf, 1, INBUF_SIZE, fin);
  inbuf[inbuflen] = '\0';

  outbufp = outbuf;

  p = inbuf;

  while (1) {
    // We assume no article is so big that they are ever going to
    // be bigger than INBUF_SIZE/2. So as long as we find
    // that the 'title' part of the string starts before
    // inbuf + INBUF_SIZE/2, we'll assume that all of the rest of
    // the article is in the buffer.
    p = strstr(p, "<title>");
    if (p && p >= inbuf + INBUF_SIZE/2) {
      fprintf(stderr, "Shift %d\n", nshift);
      nshift++;
      shift(fin);
      p = strstr(inbuf, "<title>");
    }

    if (!p) {
      break;
    }

    if (is_wikispecial(p)) {
      continue;
    }

    p += 7;  // strlen("<title>");
    const char *const begin_title = p;
    const char *const end_title = strstr(p, "</title>");

    p = strstr(p, "<text ");
    while (*p != '>') {
      p++;
    }
    p++;
    const char *const begin_text = p;
    const char *const end_text = strstr(p, "</text>");

    if (regex_search(begin_text, end_text, re)) {
      fprintf(stderr, "match %10d: title = %.*s\n",
              nmatch,
              static_cast<int>(end_title - begin_title), begin_title);

      // write out <title>...</title>
      memcpy(outbufp, "<title>", 7 /*strlen("<title>")*/);
      outbufp += 7;  // strlen("<title>")
      memcpy(outbufp, begin_title, end_title - begin_title);
      outbufp += end_title - begin_title;
      memcpy(outbufp, "</title>", 8 /*strlen("</title>")*/);
      outbufp += 8;  // strlen("</title>")
      // write out <text>...</text>
      memcpy(outbufp, "<text>", 6 /*strlen("<text>")*/);
      outbufp += 6;  // strlen("<text>")
      memcpy(outbufp, begin_text, end_text - begin_text);
      outbufp += end_text - begin_text;
      memcpy(outbufp, "</text>", 7 /*strlen("</text>")*/);
      outbufp += 7;  // strlen("</text>")

      if (outbufp > outbuf + OUTBUF_SIZE/2) {
        flush(fout);
      }

      nmatch++;
    }
  }
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <pattern> <infile> <outfile>\n", argv[0]);
    return 1;
  }
  regex re(argv[1]);
  FILE *fin = fopen(argv[2], "r");
  FILE *fout = fopen(argv[3], "w");
  inbuf = static_cast<char*>(malloc(INBUF_SIZE+1));
  outbuf = static_cast<char*>(malloc(OUTBUF_SIZE+1));
  run(re, fin, fout);
}


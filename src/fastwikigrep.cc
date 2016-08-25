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

static char inbuf[INBUF_SIZE];
static constexpr char *inbufmid = inbuf + INBUF_SIZE/2;
static constexpr char *inbufend = inbuf + INBUF_SIZE;
static char *inbufp;

static bool str_starts_with(const char *str, const char *prefix) {
  return strncmp(str, prefix, strlen(prefix)) == 0;
}

static bool is_wikispecial(const char *title) {
  return str_starts_with(title, "Wikipedia:") ||
         str_starts_with(title, "Portal:") ||
         str_starts_with(title, "Category:") ||
         str_starts_with(title, "Template:") ||
         str_starts_with(title, "Draft:");
}

static void shift(FILE *fin) {
  if (inbufp < inbufmid) {
    inbufp = inbuf;
    *inbufp = '\0';
    return;
  }
  if (inbufp < inbufend) {
    inbufp -= INBUF_SIZE/2;
    memcpy(inbuf, inbuf + INBUF_SIZE/2, inbufp - inbuf);
    *inbufp = '\0';
    return;
  }
  memcpy(inbuf, inbuf + INBUF_SIZE/2, INBUF_SIZE/2);
  inbufp = inbufmid + fread(inbuf + INBUF_SIZE/2, 1, INBUF_SIZE/2, fin);
  *inbufp = '\0';
}

static void run(const regex& re, FILE *fin, FILE *fout) {
  inbufp = inbuf + fread(inbuf, 1, INBUF_SIZE, fin);
  *inbufp = '\0';
  int nchunk = 0, nmatch = 0;
  char *p = inbuf, *title_begin, *text_begin;
  fprintf(stderr, "nchunk = %d\n", nchunk++);

  while (true) {
    p = strstr(p, "<title>");
    if (p > inbufmid) {
      fprintf(stderr, "nchunk = %d\n", nchunk++);
      shift(fin);
      p = strstr(inbuf, "<title>");
    }
    if (p == nullptr) {
      break;
    }
    title_begin = p + 7;  // strlen("<title>");
    p = strstr(p, "<text ") + 6;  // strlen("<text ");
    text_begin = strchr(p, '>') + 1;
    p = strchr(p, '<');
    if (regex_search(text_begin, p, re)) {
      // NOTE: is_wikispecial doesn't really filter out that many entries,
      // and is a relatively expensive operation.
      // Furthermore, generally there are relatively few articles that
      // match the regex search.
      // Putting is_wikispecial in here seems to actually save time
      if (is_wikispecial(title_begin)) {
        continue;
      }
      char *const title_end = strchr(title_begin, '<');
      const int title_len = title_end - title_begin;
      const int text_len = p - text_begin;
      fprintf(fout, "<title>%.*s</title><text>%.*s</text>",
              title_len, title_begin, text_len, text_begin);
      fprintf(stderr, "match %10d: title = %.*s\n",
              nmatch++, title_len, title_begin);
    }
  }

  //// potential alternative way to approach this.
  // match_results<char*> m;
  // while (p < inbufend && regex_search(p, inbufend, m, re)) {
  //   fprintf(stderr, "match = %d\n", nmatch++);
  //   p = m[0].second;
  // }
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <pattern> <infile> <outfile>\n", argv[0]);
    return 1;
  }
  fprintf(stderr, "pattern = %s\ninfile = %s\noutfile = %s\n",
          argv[1], argv[2], argv[3]);
  regex re(argv[1]);
  FILE *fin = fopen(argv[2], "r");
  if (!fin) {
    fprintf(stderr, "Could not open file '%s'\n", argv[2]);
    return 1;
  }
  FILE *fout = fopen(argv[3], "w");
  setvbuf(fout, nullptr, _IOFBF, OUTBUF_SIZE);
  run(re, fin, fout);
}


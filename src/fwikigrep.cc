// fwikigrep.cc
// Like wikigrep, but only accepts fixed strings as patterns
// fwikigrep is WAAAAY fasterr than wikigrep, due to the fact that
// wikigrep relies on C++11's regex, which seems to be pretty slow,
// and fwikigrep just searches using C's strstr.
// What takes wikigrep 50+ min, seems to have taken
// fwikigrep just ~1min20sec
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define INBUF_SIZE (2L << 30)
#define OUTBUF_SIZE (2L << 30)

#define STATE_TOP 0
#define STATE_TEXT 1

static char inbuf[INBUF_SIZE];
static constexpr char *inbufmid = inbuf + INBUF_SIZE/2;
static constexpr char *inbufend = inbuf + INBUF_SIZE;
static char *inbufp;

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

static void run(const char *pattern, FILE *fin, FILE *fout) {
  int nchunk = 0, nmatch = 0, ntitles = 0;
  char *title_begin, *text_begin, *text_end = inbuf;
  inbufp = inbuf + fread(inbuf, 1, INBUF_SIZE, fin);
  *inbufp = '\0';

  fprintf(stderr, "so far: %d chunks (%d titles)\n", nchunk, ntitles);
  nchunk++;
  while (true) {
    title_begin = strstr(text_end, "<title>");
    if (title_begin > inbufmid) {
      fprintf(stderr, "so far: %d chunks (%d titles)\n", nchunk, ntitles);
      nchunk++;
      shift(fin);
      title_begin = strstr(inbuf, "<title>");
    }
    // Assume that the file is dense enough with articles, that
    // if there is no <title> match for an entire GiB, there's no more
    // content.
    if (title_begin == nullptr) {
      fprintf(stderr, "ntitles = %d\n", ntitles);
      break;
    }
    title_begin += 7;  // strlen("<title>")
    ntitles++;
    // Assume '<title>' tags are always followed by '<text>' blocks.
    text_begin = strchr(strstr(title_begin, "<text"), '>') + 1;
    text_end = strchr(text_begin, '<');
    *text_end = '\0';
    if (strstr(text_begin, pattern)) {
      char *title_end = strchr(title_begin, '<');
      *title_end = '\0';
      fprintf(stderr, "match %10d: title = %s\n", nmatch++, title_begin);
      fprintf(fout, "<title>%s</title>\n<text>%s</text>\n",
              title_begin, text_begin);
      *title_end = '<';
    }
    *text_end = '<';
  }
  fprintf(stderr, "total: %d chunks (%d titles)\n", nchunk, ntitles);
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <pattern> <infile> <outfile>\n", argv[0]);
    return 1;
  }
  fprintf(stderr, "pattern = %s\ninfile = %s\noutfile = %s\n",
          argv[1], argv[2], argv[3]);
  FILE *fin = fopen(argv[2], "r");
  if (!fin) {
    fprintf(stderr, "Could not open file '%s'\n", argv[2]);
    return 1;
  }
  FILE *fout = fopen(argv[3], "w");
  setvbuf(fout, nullptr, _IOFBF, OUTBUF_SIZE);
  run(argv[1], fin, fout);
}


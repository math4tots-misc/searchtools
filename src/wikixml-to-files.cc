// wikixml-to-files.cc
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
#include <ctype.h>
#define INBUF_SIZE (2L << 30)
#define MILLION 1000000
#define MAX_PATH_SIZE 1000

#define STATE_TOP 0
#define STATE_PAGE 1
#define STATE_TEXT 2

static char *linebuf;
static char *titlebuf;
static char dirname[MAX_PATH_SIZE];

static FILE *fin;
static FILE *fout;

static int nfiles = 0;

static void run();

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <infile> <outdir>", argv[0]);
  }

  const char *const infn = argv[1];
  const char *const outfn = argv[2];

  fprintf(stderr, "infile = %s\noutfile = %s\n", infn, outfn);

  fin = fopen(infn, "r");
  fprintf(stderr, "%p\n", (void*) fin);
  setvbuf(fin, nullptr, _IOFBF, INBUF_SIZE);

  fprintf(stderr, "argv[2] = %s\n", argv[2]);
  strcpy(dirname, argv[2]);
  fprintf(stderr, "dirname = %s\n", dirname);
  strcat(dirname, "/");
  fprintf(stderr, "dirname = %s\n", dirname);

  linebuf = static_cast<char*>(malloc(INBUF_SIZE));
  titlebuf = static_cast<char*>(malloc(INBUF_SIZE));

  run();

  // Not necessary
  // free(linebuf);
  // free(titlebuf);
  // fclose(fin);
}
//
// static bool is_valid_title_char(char c) {
//   return c == ' ' || c == '_' || c == '.' || c == ',' ||
//          c == '-' || c == '\'' || c == '(' || c == ')' ||
//          isdigit(c) || isalpha(c);
// }

static bool str_starts_with(const char *str, const char *prefix) {
  return strncmp(str, prefix, strlen(prefix)) == 0;
}

static bool is_wikispecial(const char *title) {
  return str_starts_with(title, "Wikipedia:") ||
         str_starts_with(title, "Portal:") ||
         str_starts_with(title, "Category:") ||
         str_starts_with(title, "Template:");
}

// Unfortunately, since '/' is not a valid character to use in a title,
// it's not trivial to map them into filenames. For now, let's just
// ignore any article that has a '/' in its title.
static bool is_valid_title(char *title) {
  if (is_wikispecial(title)) {
    return false;
  }
  char *p;
  for (p = title; *p != '\0'; p++) {
    if (*p == '#') {
      return false;
    }
    if (*p == '/') {
      *p = '#';
    }
  }
  return true;
}

static void run() {
  int state = STATE_TOP;
  bool isRedir = false;
  char filespec[MAX_PATH_SIZE];

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
          char *start = strstr(linebuf, "<title>") + strlen("<title>");
          char *end = strstr(linebuf, "</title>");
          *end = '\0';
          strcpy(titlebuf, start);
        } else if (!isRedir && strstr(linebuf, "<redirect title=")) {
          isRedir = true;
        } else if (!isRedir && strstr(linebuf, "<text xml:space=")) {
          // fprintf(stderr, "Title = %s", titlebuf);
          if (is_valid_title(titlebuf)) {
            strcpy(filespec, dirname);
            strcat(filespec, titlebuf);
            fout = fopen(filespec, "w");
            // fprintf(stderr, "Writing to file '%s' -- ", filespec);
            // fprintf(stderr, "fout = %p\n", (void*) fout);
            fputs(titlebuf, fout);
            fputs(linebuf, fout);
            state = STATE_TEXT;
          } else {
            if (!is_wikispecial(titlebuf)) {
              fprintf(stderr, "Ignoring invalid title '%s'\n", titlebuf);
            }
          }
        } else if (strstr(linebuf, "</page>")) {
          if (!isRedir) {
            fclose(fout);
            nfiles++;
            if (nfiles == 100 || nfiles == 1000 ||
                nfiles == 10000 || nfiles == 100000 || nfiles%1000000 == 0) {
              fprintf(stderr, "Wrote %d files\n", nfiles);
            }
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
}

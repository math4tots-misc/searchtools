#include <iostream>
#include <fstream>
#include <stdio.h>
using namespace std;

// I tried just reading the entire thing into memory (~50GB),
// and letting the OS's paging take care of things.
// Didn't work out so great... took about ~8min20sec to find matches,
// whereas, searching in chunks of 1GiB as done here takes only about
// 1min20sec.

constexpr size_t DEFAULT_CHUNK_SIZE = 1L << 30L; // GiB
constexpr int DISPLAY_RADIUS = 1000;
constexpr int MATCH_FREQUENCY = 1000;

static char buffer[DEFAULT_CHUNK_SIZE+1];

static void displaymatch(char *match) {
  char *start;
  start = match-buffer > DISPLAY_RADIUS ? match-DISPLAY_RADIUS : buffer;
  cout << "####### Match found #######" << endl;
  printf("%.*s\n", 3*DISPLAY_RADIUS+1, start);
}

static void search(FILE *fin, char *pattern) {
  // NOTE: Things can be a bit funny around chunk boundaries.
  // but given chunk large enough, the chance of any given article falling
  // on a chunk boundary is miniscule.
  const size_t patlen = strlen(pattern);
  char *match;
  int nchunk = 0;
  int curnmatch, nmatch = 0;
  while (!feof(fin)) {
    cout << "----- Reading chunk: " << nchunk++ << " -----" << endl;
    size_t count = fread(buffer, 1, DEFAULT_CHUNK_SIZE, fin);
    buffer[count] = '\0';
    cout << "Read: " << count << " bytes" << endl;

    curnmatch = 0;
    match = strstr(buffer, pattern);
    while (match) {
      curnmatch++;
      nmatch++;
      switch(0) {
        // Display every match, with everything DISPLAY_RADIUS around it.
        case 0: {
          displaymatch(match);
          break;
        }
        // Display a match occasionally,
        case 1: {
          if (nmatch%MATCH_FREQUENCY == 0) {
            displaymatch(match);
          }
          break;
        }
        // Don't display any matches
        case 2: break;
        default:break;
      }
      match = strstr(match+patlen, pattern);
    }

    printf("%d matches this chunk, %d matches so far\n", curnmatch, nmatch);
  }
}

int main(int argc, char **argv) {
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " <source-filename> <pattern>" << endl;
    return 1;
  }
  cout << "source-filename = " << argv[1] << endl;
  cout << "pattern = " << argv[2] << endl;
  // ifstream fin(argv[1]);
  // search(fin);
  FILE *fin = fopen(argv[1], "r");
  search(fin, argv[2]);
}

//// This is significantly slower compared to fread
// void search(ifstream& fin) {
//   while (!fin.eof()) {
//     cout << "Reading chunk" << endl;
//     fin.read(buffer, DEFAULT_CHUNK_SIZE);
//     buffer[100] = '\0';
//     cout << buffer << endl;
//     buffer[fin.gcount()] = '\0';
//   }
// }
//// For comparison:
// void search(FILE *fin) {
//   while (!feof(fin)) {
//     cout << "----- Reading chunk -----" << endl;
//     int count = fread(buffer, 1, DEFAULT_CHUNK_SIZE, fin);
//     buffer[100] = '\0';
//     printf("%s", buffer);
//     buffer[count] = '\0';
//   }
// }

#include "huffstuff.h"

/* htable.c takes in a file, parses frequencies, builds
a tree based on frequency, and assigns a huffman code to
each character, printing the ascii values and codes */
int main(int argc, char *argv[]) {
  FILE *fp;
  int i;
  int cur;
  huffnode **nodelist;
  /* starts at 0 to know which characters not used */
  int freqlist[ASCIIVALS] = {0};
  /* check for correct input */
  if (argc != 2) {
    perror("Incorrect number of arguments");
  }
  if ((fp = fopen(argv[1], "r")) == NULL) {
    perror(argv[1]);
  }
  /*creates list of frequencies */
  while ((cur = fgetc(fp)) != EOF) {
    freqlist[cur]++;
  }
  nodelist = allhuff(freqlist);
  /* print codes */
  /*nodelist[0]->size set in all huff for use here */
  for (i = 0; i < nodelist[0]->size; i++) {
    printf("0x%02x: %s\n", nodelist[i]->char_val, nodelist[i]->code);
  }
  return 0;
}



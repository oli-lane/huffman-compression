/* header file for huffstuff.c, has huffnode def and huff functions */

#ifndef _HUFFSTUFF_
#define _HUFFSTUFF_
/* everything includes huffstuff.h so all needed libraries here */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
/* defines ASCIIVALS for all files */
#define ASCIIVALS 256
#define BYTE 8

 /*huffman node struct*/
typedef struct huffnode {
  /* size to make huffnodes and lists of them more portable */
  int size;
  int freq;
  unsigned char char_val;
  char *code;
  struct huffnode *left;
  struct huffnode *right;
  struct huffnode *next;
} huffnode;

int comp(const void *a, const void *b);
huffnode *combine(huffnode *a, huffnode *b);
huffnode *insert(huffnode *list, huffnode *item);
void create_codes(huffnode *node, char codes[], int i);
int comp2(const void *a, const void *b);
huffnode **allhuff(int freqlist[]);
char **codelist(huffnode **nodelist);
huffnode *build_tree(huffnode **nodelist, int size);

#endif

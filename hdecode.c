#include "huffstuff.h"
#include "hdecode.h"

/* Accepts a huffman encoded file (header then body), uses
the header to build frequency list, then a nodelist, and then
the node tree. Uses node tree and mask to go through input
bit by bit and translate the code into characters which is
written to the output file */
int main(int argc, char *argv[]) {
  int *freqlist;
  huffnode **nodelist;
  int fd, fdout;
  huffnode *head;
  int size;
  char *buf;
  int *total;
  int errcheck;
  int i;
  if (argc > 3) {
    perror("Invalid arguments");
    exit(3);
  }
  /* set fd */
  if (argc == 1 || strcmp(argv[1], "-") == 0) {
    fd = STDIN_FILENO;
  } else {
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
      perror(argv[1]);
      /* if invalid input file, get input from stdin */
      fd = STDIN_FILENO;
    }
  }
  /* set fdout */
  if (argc < 3) {
    fdout = STDOUT_FILENO;
  } else {
    fdout = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fdout == -1) {
      perror(argv[2]);
      fdout = STDOUT_FILENO;
    }
  }
  total = malloc(sizeof(int));
  if (total == NULL) {
    perror("Total malloc failed");
  }
  /* build list of frequencies */
  freqlist = parse_header(fd, total);
  /* empty input edge case */
  if (*freqlist == -1) {
    close(fd);
    close(fdout);
    free(total);
    return 0;
  }
  /* build list of nodes */
  nodelist = allhuff(freqlist);
  /* set size */
  size = nodelist[0]->size;
  /* current nodelist is sorted by ascii val, need to resort */
  qsort(nodelist, size, sizeof(huffnode *), comp);
  /* build_tree depends on a linked list */
  for (i = 0; i < size - 1; i++) {
    nodelist[i]->next = nodelist[i + 1];
  }
  nodelist[size - 1]->next = NULL;
  /* build head of huffnode tree */
  head = build_tree(nodelist, size);
  /* parse the codes and write characters to fdout */
  if (size != 1) {
    parse_body(fdout, fd,  head, *total);
  } else {
    /* one character edge case */
    buf = malloc(nodelist[0]->freq);
    if (buf == NULL) {
      perror("buf malloc failed");
      exit(3);
    }
    for (i = 0; i < nodelist[0]->freq; i++) {
      buf[i] = nodelist[0]->char_val;
    }
    errcheck = write(fdout, buf, nodelist[0]->freq);
    if (errcheck == -1) {
      perror("write failed");
      exit(3);
    }
    free(buf);
  }
  free(freqlist);
  free(total);
  close(fd);
  close(fdout);
  return 0;
}

/* reads header of file to build frequency table,
read failure checks curiously caused segfault only
for the 3 reads in this function, so they were excluded */
int *parse_header(int fd, int *total) {
  int *freqlist;
  unsigned int cbuf;
  unsigned int ibuf;
  int size;
  int i;
  freqlist = calloc(ASCIIVALS, sizeof(int));
  if (freqlist == NULL) {
    perror("freqlist calloc failed");
  }
  /* determine num of unique characters */
  if (read(fd, &cbuf, 1) == 0) {
    /* empty input file */
    free(freqlist);
    *freqlist = -1;
    return freqlist;
  } else {
    size = cbuf + 1;
  }
  /* parse characters and frequency to freqlist */
  for (i = 0; i < size; i++) {
    read(fd, &cbuf, 1);
    read(fd, &ibuf, sizeof(int));
    freqlist[cbuf] = ntohl(ibuf);
    *total += ntohl(ibuf);
  }
  return freqlist;
}

/* go through codes, traverses tree each time until leaf node,
prints the nodes' char_val and resets tree, uses mask to read
input bit by bit */
void parse_body(int fdout, int fd, huffnode *head, int total) {
  int remaining = total;
  int mask;
  unsigned int cbuf;
  int i;
  int errcheck;
  huffnode *cur = head;
  while (remaining > 0) {
    errcheck = read(fd, &cbuf, 1);
    if (errcheck == -1) {
      perror("read failed");
      exit(3);
    }
    mask = 128;
    for (i = 0; i < BYTE; i++) {
      if (cur->left == NULL && cur->right == NULL) {
	errcheck = write(fdout, &cur->char_val, 1);
	if (errcheck == -1) {
	  perror("write failed");
	  exit(3);
	}
	cur = head;
	remaining --;
	if (remaining == 0) {
	  return;
	}
      }
      if ((cbuf & mask) == '\0') {
	cur = cur->left;
      } else {
	cur = cur->right;
      }
      mask = mask >> 1;
    }
  }
}

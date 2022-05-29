#include "huffstuff.h"
#include "hencode.h"

/* Compresses a file using huffman encoding 
builds huffman tree, assigns codes, and uses
a mask to walk through a char (1 byte) and assign
1 or 0 to create the code*/
int main(int argc, char *argv[]) {
  int fd;
  int fdout;
  int size = 0;
  huffnode **nodelist;
  int* buf;
  int errcheck;
  /* initialized to 0 for identifying unused chars */ 
  int freqlist[ASCIIVALS] = {0};
  buf = malloc(1);
  if (buf == NULL) {
    perror("buf malloc failed");
    exit(3);
  }
  if (argc > 3 || argc < 2) {
    perror("Invalid args: 2-3 expected");
    exit(3);
  }
  /* open input file */
  fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    perror(argv[1]);
    exit(3);
  }
  if (argc == 3) {
    fdout = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fdout == -1) {
      /*if invalid file, set fdout to stdout */
      perror(argv[2]);
      fdout = STDOUT_FILENO;
    }
  } else {
    /* set fdout to stdout */
    fdout = STDOUT_FILENO;
  }
  if ((errcheck = read(fd, buf, 1)) == 0) {
    /*empty file case */
    return 0;
  } else if (errcheck == -1) {
    perror("read failed");
    exit(3);
  } else {
    freqlist[(int)(*buf)]++;
    size++;
  }
  /* build frequencies */
  while((errcheck = read(fd, buf, 1)) != 0) {
    if (errcheck == -1) {
      perror("read failed");
      exit(3);
    }
    freqlist[(int)(*buf)]++;
    size++;
  }
  /* build codes */
  nodelist = allhuff(freqlist);
  /* write header */  
  make_header(fdout, nodelist);
  /* reset offset to beginning */
  lseek(fd, 0, SEEK_SET);
  /* write body */
  make_body(fdout, fd, nodelist, size);
  
  close(fd);
  close(fdout);
  return 0;
}

/* write header of encode to outfile
FORMAT: size - 1 | char | freq| char | freq| etc */
void make_header(int fdout, huffnode **nodelist) {
  /* size of nodelist in nodelist[0]->size */
  unsigned char hnum = nodelist[0]->size - 1;
  int *freq;
  unsigned int c;
  int i;
  int errcheck;
  freq = malloc(sizeof(int));
  if (freq == NULL) {
    perror("freq malloc failed");
    exit(3);
  }
  errcheck = write(fdout, &hnum, 1);
  if (errcheck == -1) {
    perror("write failed");
    exit(3);
  }
  for (i = 0; i < nodelist[0]->size; i++) {
    c =  nodelist[i]->char_val;
    *freq = htonl(nodelist[i]->freq);
    errcheck = write(fdout, &c, 1);
    if (errcheck == -1) {
      perror("write failed");
      exit(3);
    }
    errcheck = write(fdout, freq, sizeof(int));
    if (errcheck == -1) {
      perror("write failed");
      exit(3);
    }
  }
}

/* write body of encode to outfile using huffman code */
void make_body(int fdout, int fd, huffnode **nodelist, int size) {
  unsigned char mask;
  unsigned char *buf;
  int i, j;
  int errcheck;
  char *cur;
  unsigned char bincur;
  int cssize = 0, cscap = size;
  char *codestring;
  char **codeslist = codelist(nodelist);
  buf = malloc(size);
  if (buf == NULL) {
    perror("Buf malloc failed");
    exit(3);
  }
  codestring = malloc(cscap);
  if (codestring == NULL) {
    perror("Codestring malloc failed");
    exit(3);
  }
  /* read input file into buf */
  errcheck = read(fd, buf, size);
  if (errcheck == -1) {
    perror("read failed");
    exit(3);
  }		   
  /* make a string of all the codes of the file */
  for (i = 0; i < size; i++) {
    cur = codeslist[(int)buf[i]];
    while ((*cur) != '\0') {
      codestring[cssize] = *cur;
      cssize++;
      if (cssize == cscap) {
	cscap *= 2;
	codestring = realloc(codestring, cscap);
	if (codestring == NULL) {
	  perror("Codestring realloc 1 failed");
	  exit(3);
	}
      }
      cur++;
    }
  }
  /*no longer needed */
  free(buf);
  /* if we need to pad with 0s */
  if (cssize % BYTE != 0) {
    /*realloc codestring to size including padding 0s */
    cscap = cssize + (BYTE - (cssize % BYTE));
    /* plus one to null terminate */
    codestring = realloc(codestring, cscap + 1);
    if (codestring == NULL) {
      perror("Codestring realloc 2 failed");
      exit(3);
    }
    for (i = 0; i < BYTE - (cssize % BYTE); i++) {
      codestring[cssize + i] = '0';
    }
    codestring[cscap] = '\0';
    /* no padding needed, realloc down to size */
  } else {
    cscap = cssize;
    codestring = realloc(codestring, cscap + 1);
    if (codestring == NULL) {
      perror("Codestring realloc 3 failed");
      exit(3);
    }
    codestring[cscap] = '\0';
  }
  /*allocate chars to write */
  buf = calloc(cscap / BYTE, 1);
  if (buf == NULL) {
    perror("buf calloc failed");
    exit(3);
  }
  for (i = 0; i < cscap; i += BYTE) {
    bincur = '\0';
    mask = 128;
    for (j = 0; j < BYTE; j++) {
      if (codestring[i + j] == '1') {
	bincur = bincur | mask;
      }
      mask = mask >> 1;
    }
    buf[i / BYTE] = bincur;
  }
  errcheck = write(fdout, buf, cscap / BYTE);
  if (errcheck == -1) {
    perror("write failed");
    exit(3);
  }

  free(buf);
  free(codeslist);
  free(codestring);
}

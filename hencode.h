/* header file for hencode.c */

#ifndef _HENCODE_
#define _HENCODE_

void make_header(int fdout, huffnode **nodelist);
void make_body(int fdout, int fd, huffnode **nodelist, int size);

#endif

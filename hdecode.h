/* header file for hdecode.c */

#ifndef _HDECODE_
#define _HDECODE_

int *parse_header(int fd, int *total);
void parse_body(int fdout, int fd, huffnode *head, int total);

#endif

#ifndef _DEFS_H
#define _DEFS_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

/* life can be harsh */
void die(const char* msg);

/* open file descriptor for input */
int open_fin(const char* fname);

/* open file descriptor for output */
int open_fout(const char* fname);

/* close file descriptor */
void close_fd(int fd);

/* better write: error checking, makes sure it finishes */
int do_write(int fd, char* buf, int size);

/* read fin and write to fout */
void cat_contents(int fin, int fout);

#endif /* _DEFS_H */

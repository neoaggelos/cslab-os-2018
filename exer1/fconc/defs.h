#ifndef _DEFS_H
#define _DEFS_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

/* 0644 */
#define WRITE_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

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

/* EXTRAS */

/* get inode of fname, used to check if input file == output file */
ino_t get_ino(const char* fname);

/* check if A and B are the same file (using their inode) */
void check_if_same(const char* A, const char* B);


#endif /* _DEFS_H */

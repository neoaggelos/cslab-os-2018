#include <stdlib.h>
#include <stdio.h>

#include "defs.h"

void usage(const char* exe) {
  printf("Usage: %s infile1 infile2 [outfile (default:fconf.out)]\n", exe);
  exit(0);
}

int main(int argc, char** argv) {
  /* file descriptors */
  int a, b, out;

  /* output filename */
  const char *outfile = "fconc.out";

  /* bad arguments */
  if (argc != 3 && argc != 4)
    usage(argv[0]);

  /* output file */
  if (argc == 4)
    outfile = argv[3];
  else
    fprintf(stderr, "Using default output filename: fconc.out\n");

  /* do not allow input == output */
  check_if_same(argv[1], outfile);
  check_if_same(argv[2], outfile);

  /* open files */
  a = open_fin(argv[1]);
  b = open_fin(argv[2]);
  out = open_fout(outfile); /* open last, only if inputs are ok */

  /* write */
  cat_contents(a, out);
  cat_contents(b, out);

  /* cleanup */
  close_fd(a);
  close_fd(b);
  close_fd(out);

  return 0;
}

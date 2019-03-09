#include <stdlib.h>
#include <stdio.h>

#include "defs.h"

void usage(const char* exe) {
  printf("Usage: %s infile1 [infile2 ... infileN] outfile\n", exe);
  exit(0);
}

int main(int argc, char** argv) {
  /* file descriptors */
  int *in, out;
  int i;

  if (argc < 3)
    usage(argv[0]);

  /* array for input fds */
  in = malloc((argc-2) * sizeof(int));
  if (in == NULL)
    die("malloc");

  /* open files */
  for (i=1; i < argc-1; i++)
    in[i-1] = open_fin(argv[i]);

  out = open_fout(argv[argc-1]);

  /* write */
  for (i=0; i < argc-2; i++)
    cat_contents(in[i], out);
    close_fd(in[i]);

  close_fd(out);


  return 0;
}

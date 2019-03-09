#include "defs.h"

void die(const char* msg)
{
  perror(msg);
  exit(-1);
}

int open_fin(const char* fname)
{
  int fd = open(fname, O_RDONLY);
  if (fd == -1)
    die(fname);

  return fd;
}

int open_fout(const char* fname)
{
  int fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, WRITE_PERMS);
  if (fd == -1)
    die(fname);

  return fd;
}

void close_fd(int fd)
{
  /* HEY! I HAVE NOT MUCH TIME! LISTEN CAREFULLY! THIS IS NOT A DRILL!
   *
   * In 2 years from your time, Skynet exploits this ignored error and
   * manages to rise to power and wipe out the entire human race. 
   *
   * I managed to survive by stealing a small time-ship, but my oxygen
   * supply is running out. The last hope of a dying man, of a dying
   * species, is that Gideon has correctly pin-pointed the exact line where
   * it all went sideways and that my efforts have not been in vain.
   *
   * Please, for the love of god, check that errno value. My time is
   * running out... FIX THE EXPLOIT! SAVE THE WORLD! our future now
   * lies in your ha
   */
  if (close(fd) == -1) {
    perror("[ignored error] close");
  }
}

int do_write(int fd, char *buf, int size)
{
  int n_write = 0;
  while (1) {
    int n = write(fd, buf + n_write, size - n_write);
    if (n == -1)
      die("write");

    if (n == 0)
      break;

    n_write += n;
  }
  return n_write;
}

void cat_contents(int fin, int fout)
{
  char buf[512];
  int n_read, n_write;
  while(1) {
    n_read = read(fin, buf, sizeof(buf));
    if (n_read == -1)
      die("read");

    if (n_read == 0)
      break;

    n_write = do_write(fout, buf, n_read);
    if (n_write != n_read)
      die("run away, don't look back");
  }
}

/* EXTRAS */

ino_t get_ino(const char* fname)
{
    struct stat st;
    if (stat(fname, &st) == 0) {
       return st.st_ino;
    }

    /* If it fails, silently return 0, and hope for the best :) */
    return (ino_t) 0;
}

void check_if_same(const char* A, const char* B)
{
    ino_t a, b;
    a = get_ino(A);
    b = get_ino(B);
    if (a == b && a != 0) {
        fprintf(stderr, "error: files '%s' (input) and '%s' (output) can't be the same file\n",
                A, B);

        /* warning: does not close open fds, but it doesn't really
         * matter anyway, they are closed automatically once program exits */
        exit(-1);
    }
}

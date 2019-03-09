#include "myincs.h"

int doWrite (int fd, const char *buff, int len)
{
  size_t idx=0;
  ssize_t wcnt;

  do {
    wcnt = write (fd, buff + idx, len - idx);

    if(wcnt==-1){
      //error
      perror("write");
      return 1;
    }

    idx+=wcnt;

  } while (idx < len);

  return 0;
}

int write_file (int fd, const char *infile)
{
   int fdin;
   fdin = open (infile, O_RDONLY);
   if(fdin==-1){
     fprintf(stderr, "Regarding <%s>'s ", infile);
     perror("open");
     return 1;
   }

   char buff[1024];
   ssize_t rcnt;

   for(;;){
     rcnt = read (fdin, buff , sizeof(buff));

     if(rcnt==0)
       return 0;

     if(rcnt==-1){
       //error
       fprintf(stderr, "Regarding <%s>'s ", infile);
       perror("read");
       return 1;
     }

     if (doWrite(fd, buff, rcnt)==1)
       return 1;
   }

   close(fdin);

   return 0;
}

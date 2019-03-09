#include "myincs.h"
#include "myfuns.h"

int main (int argc, char ** argv)
{
  
  // checking for correct num of args
 
  if (argc==1 || argc==2){
    printf("Damn it man, I'm expecting \"%s inputfile1 inputfile2 ... inputfile(n-1) outputfile\"\n", argv[0]);
    exit(1);
  }

   // creating output file


  char * outputfile;

  outputfile=argv[argc-1];
  

  int fd, oflags, mode;
  
  oflags=O_CREAT | O_WRONLY | O_TRUNC ;
  mode = S_IRUSR | S_IWUSR;

  fd = open (outputfile, oflags, mode);
  if(fd==-1){
    fprintf(stderr, "Regarding <%s>s's ", outputfile);
    perror("open");
    exit(1);
  }

  // reading and inputing files

  int i;

  for (i = 1; i < argc-1; i++)
    if(write_file(fd, argv[i])==1){
      close(fd);
      exit(1);
    }
  
  
  close(fd);

  return 0;
}

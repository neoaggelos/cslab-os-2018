#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void zing (void)
{
    char * user = getlogin();
    char * greet = "Bye-Bye ";  //diff message
    char * message;  //actual output
    int out; //err or nah

    /* No need for write fun instead of printf (?) */
    if (user == NULL || (message=malloc(strlen(greet)+strlen(user)+1)) == NULL){
    //malloc or getlogin err
        message = "Error occured\n";                                         
        out=2;
    }
    else{
        out=1;
        message[0]='\0';                                                     
        strcat(message, greet);                                               
        strcat(message, user);                                               
        strcat(message,"\n");                                                
   }                                                                         
    
    size_t len, idx = 0;
    ssize_t wcnt;
    len = strlen(message);
    
    //lemme prin dat out

    do{
      wcnt = write(out, message + idx, len - idx);
      if(wcnt==-1){
        perror("write");
        exit(1);
      }
      idx+=wcnt;
    } while (idx<len);
}

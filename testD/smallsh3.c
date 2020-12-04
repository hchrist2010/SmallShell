#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>

char argument[64];
char **argv;

void prompt(){                       //Modular prompt
    printf ("?:");
}

void mainloop(){                    //Loop that calls the functions
    while(1){
        prompt();
        argv = (char**)malloc(sizeof(char*)*64);  //allocate memory for argv
        getcommand(argument, argv);
        if((strcmp(argv[0],"exit" )) == 0){  //check for exit
            return 0;
        }
        executecommand();
        printcommand();
        //clearcommand();
        //printcommand();
    }
}

void getcommand(char* argument, char** argv){  //Parser for the command
    int i=0,j=0;
    char c;
    char* token;
    while((c = getchar()) != '\n' ){ //gets char and checks for end of line
        argument[i] = c;
        i++;
    }
    argument[i] = '\0';
    token = strtok(argument, " ,.");  //tokenize the command
    while (token != NULL){
        argv[j] = token;   //pass command to array of arguments
        token = strtok(NULL, " ,.");
        j++;
    }
    argv[j] = NULL;
}

void executecommand(){  //Function to call fork and execute with errors
    pid_t childpid = fork();
    int returnStatus;
    if(childpid == -1){                           //Fail to Fork
        printf("failed to fork");
        exit(1);
    }
    else if(childpid == 0){                      //Child process
        if (execvp(*argv, argv) < 0){
            printf("error executing\n");
            exit(1);
        }
        else{                                   //Execute successful
            printf("executed");
        }
    }
    int c=(int)waitpid(childpid, &returnStatus, 0);
    if (returnStatus == 0)  // Verify child process terminated without error.
        {
            printf("The child process terminated normally. \n");
        }

    if (returnStatus == 1)
        {
            printf("The child process terminated with an error!.\n");
        }
    //realloc(argv,64);
}

void printcommand(){  //Test function to print arguments
    int i = 0;
    while(argv[i] != NULL){
        printf("Argv%d: %s \n",i, argv[i] );
        i++;
    }
}

/*void clearcommand(){     //Function to clear up the memory, does not work
  int i=0;
  argv[0] = "       \0";
  argv[1] = "       \0";

  }*/


  int main(int argc, char** argv) {
      mainloop();                      //Loop that calls the commands
      return (EXIT_SUCCESS);
  }

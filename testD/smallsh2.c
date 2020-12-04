#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>

#define CMDMAX 2048

struct Input {
        char **argv;
        char uInput[CMDMAX];
        char *inFileName;
        char *outFileName;
        int argNumb;
        pid_t pid;
        int background;
        int status;
        int savedIn;
        int savedOut;
        int in;
        int out;
};

int x = 0;

void debug(){
        x++;
        printf("%d\n", x);
}

void shift(struct Input *input, int index){
        int i;

        for(i = index; i < input->argNumb; i++){
                input->argv[i] = input->argv[i + 1];
        }

}

int getInput(struct Input *input){
        (": ");
        int i = 0;
        char c;
        char *token;
        while((c = getchar()) != '\n'){
                input->uInput[i] = c;
                i++;
        }
        if(strlen(input->uInput) == 0){
                return 1;
        }
        input->argv[i] = '\0';

        token = strtok(input->uInput, " ");
        while(token != NULL){
                input->argv[input->argNumb] = token;
                token = strtok(NULL, " ");
                input->argNumb++;
        }
        input->argv[input->argNumb] = NULL;

	pidExpansion(input);

        for(i = 0; i < input->argNumb; i++){
                if(input->argv[i] != NULL){
                        if(strcmp(input->argv[i], "<") == 0){
                                input->inFileName = input->argv[i + 1];
                                shift(input, i);
                                shift(input, i);
                        }
                }
                if(input->argv[i] != NULL){
                        if(strcmp(input->argv[i], ">") == 0){
                                input->outFileName = input->argv[i + 1];
                                shift(input, i);
                                shift(input, i);
                        }
                }
                if(input->argv[i] != NULL){
                        if(strcmp(input->argv[i], "&") == 0){
                                shift(input, i);
                                input->argv[input->argNumb] = NULL;
                                input->argNumb--;
                                input->background = 1;
                        }
                }
        }
        return 0;

}

void display(struct Input *input){
        printf("\n\nDisplay\n");
        printf("Command: %s\n", input->argv[0]);
        int i;
        char s[100];
        for(i = 1; i < input->argNumb + 1; i++){
            printf("Arg %d: %s\n", i, input->argv[i]);
        }
        printf("Input File: %s\n", input->inFileName);
        printf("Output File: %s\n", input->outFileName);
        printf("Background: %d\n", input->background);
        printf("Current PID: %d\n", getpid());
        printf("Current Directory: %s\n", getcwd(s, 100));
}

void initialize(struct Input *input){
        input->argv = (char**)malloc(sizeof(char*) * 517);
        input->inFileName = NULL;
        input->outFileName = NULL;
        input->background = 0;
        input->argNumb = 0;
        input->savedIn = dup(0);
        input->savedOut = dup(1);
}

void deconCommand(struct Input *input){
        int i;
        for(i = 0; i < CMDMAX; i++){
                input->uInput[i] = '\0';
        }
        for(i = 0; i < input->argNumb + 1; i++){
                input->argv[i] = NULL;
        }
        free(input->argv);
        free(input);
        input = NULL;
}

void execute(struct Input *input){
        pid_t childpid = fork();
        int returnStatus;
        if(childpid == -1){                           //Fail to Fork
            printf("failed to fork");
            exit(1);
        }
        else if(childpid == 0){                      //Child process
            if (execvp(input->argv[0], input->argv) < 0){
                printf("error executing\n");
                exit(1);
            }
            else{                                   //Execute successful
                printf("executed");
            }

        }
        else{
                if(input->background == 1){
                        printf("Background PID is %d\n", childpid);
                }
                else{
                        pid_t wait = waitpid(childpid, &returnStatus, 0);
                        if(WIFEXITED(returnStatus)){

                        }
                }

        }

        if (returnStatus == 0 && input->background == 1)  // Verify child process terminated without error.
            {
                printf("The child process terminated normally. \n");
            }

        if (returnStatus == 1)
            {
                printf("The child process terminated with an error!.\n");
            }
        // input->pid = fork();
        // int returnStatus;
        // if(input->pid == -1){
        //         printf("failed to fork\n");
        //         exit(1);
        // }
        // else if(input->pid == 0){
        //         if(execvp(input->argv[0], input->argv) < 0){
        //                 printf("error executing\n");
        //                 exit(1);
        //         }
        //         else{
        //                 if(input->background == 0){
        //                         int c = (int)waitpid(input->pid, &returnStatus, 0);
        //                         printf("c: %d\n", c);
        //                 }
        //         }
        // }
        // if(returnStatus == 0){
        //         printf("The child process terminated normally. \n");
        //     }
        //
        // if (returnStatus == 1)
        //     {
        //         printf("The child process terminated with an error!.\n");
        //     }
}

void cd(struct Input *input){
    chdir(input->argv[1]);
}

void stat(struct Input *input){
    waitpid(input->pid, &input->status, 0);
    if(WIFEXITED(input->status)){
        int exit_status = WEXITSTATUS(input->status);
        printf("Exit value %d\n", exit_status);
    }
}

void openInOut(struct Input *input){
    if(input->inFileName){
        input->savedIn = dup(0);
        input->in = open(input->inFileName, O_RDONLY);
        dup2(input->in, 0);
    }
    if(input->outFileName){
        input->savedOut = dup(1);
        input->out = open(input->outFileName, O_WRONLY | O_CREAT, 0640 );
        dup2(input->out, 1);
    }
}

void closeInOut(struct Input *input){
    if(input->inFileName){
        close(input->in);
        dup2(input->savedIn, 0);
    }
    if(input->outFileName){
        close(input->out);
        dup2(input->savedOut, 1);
    }
}

int checkComment(struct Input *input){
        char temp1 = input->argv[0][0];
        char temp2 = '#';
        if(temp1 == temp2){
                return 1;
        }

    return 0;
}

void sig_int_handler(){
        kill();
}

void replace(struct Input *input, int p, int q){
        char *temp;
        char *token1;
        char *token2;
        int i, j;

        char expand[256];

        temp = input->argv[p];


        if(q > 1){
                token1 = strtok(temp, "$$");
                token2 = strtok(NULL, "\n");
                temp = NULL;
                j = strlen(token2);
                for(i = 0; i < j - 1; i++){
                        token2[i] = token2[i + 1];
                }
                token2[j - 1] = NULL;
                sprintf(expand, "%s%d%s", token1, getpid(), token2);
        }
        else{
                token1 = strtok(temp, "$$");
                sprintf(expand, "%d%s", getpid(), token1);
        }
	input->argv[p] = expand;
}

void pidExpansion(struct Input *input){
        int i = 0;
        int j = 0;
        int q = 0;
        int p = 0;
        char temp1;
        char temp2 = '$';

        for(i = 0; i < input->argNumb; i++){
                if(strcmp(input->argv[i], "$$") == 0){
                        char sTemp[8];
                        sprintf(sTemp, "%d", getpid());
                        input->argv[i] = sTemp;
//                        printf("%s\n", input->argv[i]);
                        return;
                }
        }
        for(i = 0; i < input->argNumb; i++){
                for(j = 0; j < strlen(input->argv[i]); j++){
                        temp1 = input->argv[i][j];
                        if(temp1 == temp2){
                                temp1 = input->argv[i][j+1];
                                if(temp1 == temp2){
                                        p = i;
                                        q = j + 1;
                                        replace(input, p, q);
                                }
                        }
                }
        }
}



int main(){
        signal(SIGCHLD, SIG_IGN);
//        signal(SIGINT, sig_int_handler);
        struct Input *input;
        int i = 0;
        while(1){
                input = malloc(sizeof(struct Input));
                initialize(input);
                i = getInput(input);

                if(i == 0){
                        if(checkComment(input) == 1){
                                i = 1;
                        }
                }
                if(i == 0){
                        if((strcmp(input->argv[0],"exit" )) == 0){  //check for exit
                                deconCommand(input);
                                return 0;
                        }
                        if(strcmp(input->argv[0], "cd") == 0){
                                cd(input);
                                i = 1;
                                deconCommand(input);
                        }

                        if(strcmp(input->argv[0], "status") == 0){
                                stat(input);
                                i = 1;
                                deconCommand(input);
                        }

                        if(i == 0){
                                openInOut(input);
                                execute(input);
                                closeInOut(input);
                                deconCommand(input);
                        }

                }

                i = 0;
        }

        return 0;
}

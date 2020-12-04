#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>

#define CMDMAX 2048

struct Input {
    char uInput[CMDMAX];
//    char command[CMDMAX];
    char *argument[512];
    char *inFileName;
    char *outFileName;
    int background;
    int numbArg;
    int savedIn;
    int savedOut;
    int in;
    int out;
    int inFlag;
    int outFlag;
    pid_t pid;
};

void sigintHandler(){
        kill(-1, SIGKILL);
}

struct Input * commandInit(struct Input *input){
    input = (struct Input *)malloc(sizeof(struct Input));
    input->background = 0;
    input->numbArg = 0;
    input->inFlag = 0;
    input->outFlag = 0;
    input->inFileName = NULL;
    input->outFileName = NULL;
    input->savedIn = 0;
    input->savedOut = 0;
    input->in = 0;
    input->out = 0;
    return(input);
}

struct Input * getInput(struct Input *input){
    input->background = 0;
    fflush(stdout);
//    printf("%d\n", getpid());
    printf(": ");
    fflush(stdin);
    fgets(input->uInput, CMDMAX, stdin);

    return(input);
}

struct Input *parseCommand(struct Input *input){
        char *temp;
        char *token;
        int i = 0;

        //copy command to a temp variable
        temp = malloc(sizeof(input->uInput));
        strcpy(temp, input->uInput);

        token = strtok(temp, " ");
        while(token != NULL){
//                debug();
                if(i == 0){
                        if(strcmp(token, "<") == 0|| strcmp(token, ">") == 0 || token[strlen(token) - 1] == '&'){
                                i = 1;
                        }
                        if(i == 0){
                                input->argument[input->numbArg] = token;
//                                input->argument[input->numbArg] = malloc(sizeof(token) + 2);
//                                input->argument[input->numbArg] = strdup(token);
//                                strcpy(input->argument[input->numbArg], token + '\n');
//				input->argument[input->numbArg][strlen(token) + 1] = '\0';
                                input->numbArg++;
                        }
                }
                if(i == 1){
                        if(strcmp(token, "<") == 0){
                                token = strtok(NULL, " ");
//                                input->inFileName = malloc(sizeof(token) + 2);
//                                strcpy(input->inFileName, token + '\n');
				input->inFileName[strlen(input->inFileName) + 1] = '\0';
                                input->inFileName = strdup(token);
                        }
                        if(strcmp(token, ">") == 0){
                                token = strtok(NULL, " ");
//				fflush(stdout);Z
//                                printf("%s\n", token);
//                                input->outFileName = malloc(sizeof(token) + 2);
//                                strcpy(input->outFileName, token + '\n');
				input->outFileName[strlen(input->outFileName) + 1] = '\0';
                                input->outFileName = strdup(token);
                        }

                }

                token = strtok(NULL, " ");
        }
                if(input->uInput[strlen(input->uInput) - 2] == '&'){
                    input->background = 1;
                }
//        	input->argument[0][strlen(input->argument[0]) - 1] = '\0';

                if(strcmp(input->argument[input->numbArg - 1], "&\n") == 0){
                        free(input->argument[input->numbArg - 1]);
                        input->numbArg--;
                }

        	free(temp);
        	free(token);

    // char *token;
    // char *temp;
    // temp = malloc(sizeof(input->uInput));
    // strcpy(temp, input->uInput);
    // token = strtok(temp, "\n");
    // token = strtok(token, "&");
    // token = strtok(token, ">");
    // token = strtok(token, "<");
    //
    // strcpy(temp, token);
    //
    // token = strtok(temp, " ");
    // // strcpy(input->command, token);
    // // token = strtok(NULL, " ");
    // int i = 0;
    // while(token != NULL){
    //     input->argument[input->numbArg] = malloc(sizeof(token));
    //     strcpy(input->argument[i], token);
    //     input->numbArg++;
    //     token = strtok(NULL, " ");
    // }
    // free(temp);
    // free(token);
    //
    // temp = malloc(sizeof(input->uInput));
    // strcpy(temp, input->uInput);
    //
    // token = strtok(temp, " ");
    // for(i = 0; i < input->numbArg; i++){
    //     token = strtok(NULL, " ");
    // }
    // i = 0;
    // token = strtok(NULL, " ");
    // while(i < 3 && token != NULL){
    //     if(strcmp(token, "<") == 0){
    //         input->inFlag = 1;
    //         token = strtok(NULL, " ");
    //         input->inFileName = (char*)malloc(sizeof(token));
    //         strcpy(input->inFileName, token);
    //         input->inFileName[strlen(input->inFileName) - 1] = '\0';
    //     }
    //     if(strcmp(token, ">") == 0){
    //         input->outFlag = 1;
    //         token = strtok(NULL, " ");
    //         input->outFileName = (char*)malloc(sizeof(token));
    //         strcpy(input->outFileName, token);
    //         input->outFileName[strlen(input->outFileName) - 1] = '\0';
    //     }
    //
    //     token = strtok(NULL, " ");
    //     i++;
    // }
    // if(input->uInput[strlen(input->uInput) - 2] == '&'){
    //     input->background = 1;
    // }
    // free(temp);
    return input;
}

void deconCommand(struct Input *input){
    int i;
    for(i = 0; i < input->numbArg; i++){
        free(input->argument[i]);
    }

    memset(input->uInput, 0, sizeof(input->uInput));
//    memset(input->command, 0, sizeof(input->command));

//    input->argument = NULL;
    free(input->outFileName);
    free(input->inFileName);
    input->inFileName = NULL;
    input->outFileName = NULL;
    input->numbArg = 0;
    input->background = 0;
    input->inFlag = 0;
    input->outFlag = 0;

    fflush(stdin);
}

void displayCommand(struct Input *input){
    printf("\n\nDisplay\n");
    printf("Command: %s\n", input->argument[0]);
    int i;
    char s[100];
    for(i = 1; i < input->numbArg; i++){
        printf("Arg %d: %s\n", i, input->argument[i]);
    }
    printf("Input File: %s\n", input->inFileName);
    printf("Output File: %s\n", input->outFileName);
    printf("Background: %d\n", input->background);
    printf("Current PID: %d\n", getpid());
    printf("Current Directory: %s\n", getcwd(s, 100));
}

int checkComment(struct Input *input){
//    char temp1 = input->command[0];
        char temp1 = input->argument[0][0];

    char temp2 = '#';

    if(temp1 == temp2){
        return 1;
    }

    return 0;
}

int checkBlank(struct Input *input){
    if(strlen(input->uInput) == 1){
        deconCommand(input);
        return 1;
    }
    return 0;
}

void cd(struct Input *input){
    chdir(input->argument[0]);
}

void stat(struct Input *input){
    int status;
    waitpid(input->pid, &status, 0);
    if(WIFEXITED(status)){
        int exit_status = WEXITSTATUS(status);
        printf("Exit value %d\n", exit_status);
    }
}

void openInOut(struct Input *input){
    if(input->inFlag == 1){
        input->savedIn = dup(0);
        input->in = open(input->inFileName, O_RDONLY);
        dup2(input->in, 0);
    }
    if(input->outFlag == 1){
        input->savedOut = dup(1);
        input->out = open(input->outFileName, O_WRONLY | O_TRUNC | O_CREAT);
        dup2(input->out, 1);
    }
}

void closeInOut(struct Input *input){
    if(input->inFlag == 1){
        close(input->in);
        dup2(input->savedIn, 0);
        input->inFlag = 0;
    }
    if(input->outFlag == 1){
        close(input->out);
        dup2(input->savedOut, 1);
        input->outFlag = 0;
	fflush(stdin);
	fflush(stdout);
    }
}

int execute(struct Input *input){
//    if(strcmp(input->command, "cd") == 0){
        if(strcmp(input->argument[0], "cd") == 0){
                cd(input);
                return;
    }

//    if(strcmp(input->command, "status") == 0){
        if(strcmp(input->argument[0], "status") == 0){
            stat(input);
            return;
    }

    int numbArg = input->numbArg;
//    char *argv[input->numbArg + 1];
    int i;
//    char command[256];
    // strcpy(command, input->command);
//

//    for(i = 0; i < numbArg; i++){
//        argv[i] = malloc(sizeof(input->argument[i]));
//	strcpy(argv[i], input->argument[i]);
//    }

//    pid_t pid;
//    argv[0] = command;
//    if(input->numbArg > 0){
//       for(i = 0; i < input->numbArg + 1; i++){
//            argv[i] = args[i - 1];
//        }
//    }
//    else{
//        argv[1] = NULL;
//    }
        input->argument[input->numbArg] = NULL;
        int status;
        if((input->pid =  fork()) == 0){
            signal(SIGINT, sigintHandler);
            execvp(input->argument[0], input->argument);
            printf("Execvp sucks\n");
            exit(1);
        }
        else{
//                waitpid(input->pid, &status, 0);
                wait(NULL);
        }
/*
    input->pid = fork();

    if(input->pid == 0){
//	deconCommand(input);
/*	for(i = 0; i < input->numbArg + 1; i++){
		printf("%s, ", argv[i]);
	}
	printf("\n");

	if(input->background == 1){
		printf("Child PID: %d\n", getpid());
		if(!input->inFlag){
			int devNull = open("/dev/null", O_WRONLY);
			dup2(devNull, input->savedIn);
		}
		if(!input->outFlag){
			int devNull = open("/dev/null", O_WRONLY);
			dup2(devNull, input->savedOut);
		}	printf("child PID: %d\n", getpid());
	}
        i = execvp(argv[0], argv);

        if(i < 0){
            printf("Error execvp\n");
            exit(1);
        }
    }

    if(input->pid < 0){
	printf("pid < 0\n");
        perror("fork");
        exit(1);
    }
    if(input->background == 0){
	int the_status;
        waitpid(input->pid,&the_status,0);
    }
*/
//    input->background = 0;
//    printf("check\n");
    return 0;
}

int newCommand(){
    struct Input *input;
    input = commandInit(input);
    input = getInput(input);
    int i;
    if(checkBlank(input)){
        deconCommand(input);
        return 0;
    }
    input = parseCommand(input);

    if(strcmp(input->argument[0], "exit") == 0){
        deconCommand(input);
        return 1;
    }

    if(checkComment(input)){
        deconCommand(input);
        return 0;
    }
//    openInOut(input);
    i = execute(input);
//    closeInOut(input);
    displayCommand(input);
    deconCommand(input);
    return i;
}

int main(){
    int i = 0;
    while(i == 0){
        i = newCommand();
    }
    return 0;
}

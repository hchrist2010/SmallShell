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
        char command[CMDMAX];
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
	wait(NULL);
	printf("\n");
}

struct Input * commandInit(struct Input *input){
        input = (struct Input *)malloc(sizeof(struct Input));
	input->background = 0;
	input->numbArg = 0;
	input->inFlag = 0;
	input->outFlag = 0;
	return(input);
}

struct Input * getInput(struct Input *input){
//	xx++;
//	fflush(stdout);
//	fflush(stdin);
//	fflush(stderr);
	
        printf(": ");
        fflush(stdin);
	fgets(input->uInput, CMDMAX, stdin);

        return(input);
}

struct Input *parseCommand(struct Input *input){
        char *token;
        char *temp;
        temp = malloc(sizeof(input->uInput));
        strcpy(temp, input->uInput);
        token = strtok(temp, "\n");
        token = strtok(token, "&");
        token = strtok(token, ">");
        token = strtok(token, "<");

        strcpy(temp, token);

        token = strtok(temp, " ");
        strcpy(input->command, token);
        token = strtok(NULL, " ");
        int i = 0;
        while(token != NULL){
                input->argument[i] = malloc(sizeof(token));
                strcpy(input->argument[i], token);
                i++;
                token = strtok(NULL, " ");
        }
        input->numbArg = i;
        free(temp);
        free(token);

        temp = malloc(sizeof(input->uInput));
        strcpy(temp, input->uInput);

        token = strtok(temp, " ");
        for(i = 0; i < input->numbArg; i++){
                token = strtok(NULL, " ");
        }
        i = 0;
        token = strtok(NULL, " ");
        while(i < 3 && token != NULL){
//		printf("%s\n", token);
                if(strcmp(token, "<") == 0){
			input->inFlag = 1;
                        token = strtok(NULL, " ");
                        input->inFileName = (char*)malloc(sizeof(token));
                        strcpy(input->inFileName, token);
			input->inFileName[strlen(input->inFileName) - 1] = '\0';
			input->savedIn = dup(0);
			input->in = open(input->inFileName, O_RDONLY);	
			dup2(input->in, 0);
                }
                if(strcmp(token, ">") == 0){
			input->outFlag = 1;
                        token = strtok(NULL, " ");
                        input->outFileName = (char*)malloc(sizeof(token));
                        strcpy(input->outFileName, token);
			input->outFileName[strlen(input->outFileName) - 1] = '\0';
			input->savedOut = dup(1);
 	                input->out = open(input->outFileName, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        	        dup2(input->out, 1);
               }

		token = strtok(NULL, " ");
                i++;
        }
	if(input->uInput[strlen(input->uInput) - 2] == '&'){
		input->background = 1;
	}
        free(temp);
        return input;
}

void deconCommand(struct Input *input){
/*	int i;
	for(i = 0; i < input->numbArg; i++){
		free(input->argument[i]);
	}

	memset(input->uInput, 0, sizeof(input->uInput));
	memset(input->command, 0, sizeof(input->command));

	free(input->outFileName);
	free(input->inFileName);
	input->inFileName = NULL;
	input->outFileName = NULL;
	free(input);
	input = NULL;
	fflush(stdin);
*/
}


void displayCommand(struct Input *input){
        printf("\n\nDisplay\n");
        printf("Command: %s\n", input->command);
        int i;
	char s[100];
        for(i = 0; i < input->numbArg; i++){
                printf("Arg %d: %s\n", i, input->argument[i]);
        }
        printf("Input File: %s\n", input->inFileName);
        printf("Output File: %s\n", input->outFileName);
        printf("Background: %d\n", input->background);
	printf("Current Directory: %s\n", getcwd(s, 100));
}

int checkComment(struct Input *input){
	char temp1 = input->command[0];
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

int execute(struct Input *input){
	if(strcmp(input->command, "cd") == 0){
		cd(input);
		return;
	}

	if(strcmp(input->command, "status") == 0){
		stat(input);
		return;
	}

	int numbArg = input->numbArg;
        char *argv[input->numbArg + 2];
        int i;
	char command[256];
	pid_t pid;
	strcpy(command, input->command);
        argv[0] = input->command;

	if(input->numbArg > 0){
	        for(i = 1; i < input->numbArg + 1; i++){
	                argv[i] = input->argument[i - 1];
	        }
	        argv[input->numbArg + 2] = NULL;
	}
	else{
		argv[1] = NULL;
	}

	input->	pid = fork();

        if(input->pid == 0){
		deconCommand(input);
//		printf("%d\n", getpid());
                i = execvp(command, argv);
		if(i < 0){
			printf("Error\n");
			exit(1);
		}
        }
	if(input->pid < 0){
		perror("fork");
		exit(1);
	}
        if(input->background == 0){
		wait(NULL);
	}

	if(input->inFlag == 1){
                close(input->in);
                dup2(input->savedIn, 0);
                input->inFlag = 0;
        }
        if(input->outFlag == 1){
                close(input->out);
                dup2(input->savedOut, 1);
                input->outFlag = 0;
        }
//	wait(NULL);
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

	if(strcmp(input->command, "exit") == 0){
		deconCommand(input);
		return 1;
	}

	if(checkComment(input)){
		deconCommand(input);
		return 0;
	}

	deconCommand(input);
	i = execute(input);
//	printf("2\n");
	return i;
}

int main(){
	signal(SIGINT, sigintHandler);
//	char s[100];
//	getcwd(s, 100);
	int i = 0;
	while(i == 0){
		i = newCommand();
	}
//	chdir(s);
//	printf("3\n");
	return 0;
}

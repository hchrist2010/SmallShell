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
};

struct Input * commandInit(struct Input *input){
	input = (struct Input *)malloc(sizeof(struct Input));
//	input->uInput = (char *)malloc(sizeof((char *) CMDMAX));
//	input->command = (char *)malloc(sizeof((char *) CMDMAX));
//	printf("Init Complete\n");
	input->numbArg = 0;
	return(input);
}

struct Input * getInput(struct Input *input){
	printf(": ");
	fflush(stdin);
	scanf(" %[^\n]s", input->uInput);
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
		if(strcmp(token, "<") == 0){
			token = strtok(NULL, " ");
			input->inFileName = (char*)malloc(sizeof(token));
			strcpy(input->inFileName, token);
		}
		if(strcmp(token, ">") == 0){
			token = strtok(NULL, " ");
			input->outFileName = (char*)malloc(sizeof(token));
			strcpy(input->outFileName, token);
		}
		if(strcmp(token, "&") == 0){
			input->background = 1;
		}
		token = strtok(NULL, " ");
		i++;		
	}
	free(temp);
	return input;
}

void displayCommand(struct Input *input){
	printf("\n\nDisplay\n");
	printf("Command: %s\n", input->command);
	int i;
	for(i = 0; i < input->numbArg; i++){
		printf("Arg %d: %s\n", i, input->argument[i]);
	}
	printf("Input File: %s\n", input->inFileName);
	printf("Output File: %s\n", input->outFileName);
	printf("Background: %d\n", input->background);
}

void deconCommand(struct Input *input){
	int i;

//	fflush(input->uInput);
//	fflush(input->command);

//	strcpy(input->uInput, NULL);
//	strcpy(input->command, NULL);
//	free(input->uInput);
//	free(input->command);


	for(i = 0; i < input->numbArg; i++){
		free(input->argument[i]);
	}
	free(input->outFileName);
	free(input->inFileName);
	input->inFileName = NULL;
	input->outFileName = NULL;
	free(input);	
	fflush(stdin);
}

void ls(struct Input *input){
	if(!fork()){
		execlp(input->command, input->command, NULL);
	}
	else{
		wait(NULL);
	}
}

void cat(struct Input *input){
	char *argv[input->numbArg+2];
	int i;
	argv[0] = input->command;
	for(i = 1; i < input->numbArg + 1; i++){
		argv[i] = input->argument[i - 1];
	}
	argv[input->numbArg + 1] = NULL;
	if(!fork()){
		execvp(input->command, argv);
	}
	else{
		wait(NULL);
	}
}

void execute(struct Input *input){
/*	if(strcmp(input->command[0], "#") == 0){
		return;
	}
	if(strcmp(input->command, "") == 0){
		return;
	}
*/

	int inFlag = 0;
	int outFlag = 0;
	int in, out, savedIn, savedOut;
	if(input->inFileName){
		savedIn = dup(0);
		inFlag = 1;
		in = open(input->inFileName, O_RDONLY);
		dup2(in, 0);
	}
	if(input->outFileName){
		savedOut = dup(1);
		outFlag = 1;
		out = open(input->outFileName, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);

		dup2(out, 1);
	}

/*
	if(strcmp(input->command, "ls") == 0){
		ls(input);
	}
	if(strcmp(input->command, "cat") == 0){
		cat(input);
	}
*/

	char *argv[input->numbArg + 2];
	int i;
	argv[0] = input->command;
	for(i = 1; i < input->numbArg + 1; i++){
		argv[i] = input->argument[i - 1];
	}
	argv[input->numbArg + 2] = NULL;

	if(input->numbArg > 0){
		char *argv[input->numbArg + 2];
		int i;
		argv[0] = input->command;
		for(i = 1; i < input->numbArg + 1; i++){
			argv[i] = input->argument[i - 1];
		}
		argv[input->numbArg + 2] = NULL;
		if(!fork()){
			execvp(input->command, argv);
		}
		else{
			wait(NULL);
		}
	}

	else{
		if(!fork()){
			execlp(input->command, input->command, NULL);
		}
		else{
			wait(NULL);
		}
	}
//	if(!fork()){
//		execvp(input->command, argv);
//	}
//	else{
//		wait(NULL);
//	}
	if(inFlag == 1){
//		in = NULL;
//		if(close(in) < 0){
//			printf("Fuck in\n");
//			perror("c1");
//			exit(1);
//		}
		close(in);
		dup2(savedIn, 0);
		inFlag = 0;
	}
	if(outFlag == 1){
//		out = NULL;
//		if(close(out) < 0){
//			printf("fuck out\n");
//			perror("c2");
//			exit(1);
//		}
		close(out);
		dup2(savedOut, 1);
		outFlag = 0;
	}
//	close(in);
//	close(out);
//	printf("Args: %d\n", input->numbArg);
}

int newCommand(){
	int i;
	struct Input *input;
	input = commandInit(input);
	input = getInput(input);
	input = parseCommand(input);
	printf("%s\n", input->uInput);
	if(strcmp(input->command, "exit") == 0){
		return 1;
	}
/*	if(strcmp((char)input->uInput[0], "\n") == 0){
		return 0;
	}
	if(strcmp((char)input->uInput[0], "#") == 0){
		return 0;
	}
*/	else{
//		displayCommand(input);
		execute(input);
		fflush(stdin);
		deconCommand(input);
	}
	return 0;
}

int main(){
	int i = 0;
	while(i != 1){
		i = newCommand();
	}
	return 0;
}

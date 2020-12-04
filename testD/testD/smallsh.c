//Hunter Christiansen
//ID: G932211550
//email: christhu@oregonstate.edu
//OS1 assignment3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>

#define CMDMAX 2048     //max size of a command

struct Input {                  //I  chose to create my command as a struct so I could pass it arournd
        char **argv;            //as a single object
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

pid_t parentPid;                //Global holding pid of smallsh
pid_t childPid[CMDMAX];         //array holds the pid of all processes launched
int numbChild = 0;              //holds the number of child processes created
int backFlag = 0;               //flag used to determine if in forground mode or not
int backFlag2 = 0;              //used for SIGINT to determine if process can be killed or not
int skipFlag = 0;               //used to skip over a bunch of commands in main, used for signals
int lastStatus = 0;             //holds status of last executed command
char cwd[CMDMAX];               //holds the location smallsh is launched from
pid_t lastChild;                //holds the pid of last child as a global for SIGINT

//Function prototypes
void sigquit();
void sigintHandler();
void shift(struct Input *input, int index);
int getInput(struct Input *input);
void display(struct Input *input);
void initialize(struct Input *input);
void deconCommand(struct Input *input);
void execute(struct Input *input);
void cd(struct Input *input);
int openInOut(struct Input *input);
void closeInOut(struct Input *input);
int checkComment(struct Input *input);
int checkp(struct Input *input);
void pidExpansion(struct Input *input);
void getParent();
void backstop(int p);

//used to shift all of the arguments down through the arguments array.
//this occures when an input file or output file is called and there is still data behind the file name
void shift(struct Input *input, int index){
        int i;
        for(i = index; i < input->argNumb; i++){        //simply loops through all arguments from index on and shifts them down one
                input->argv[i] = input->argv[i + 1];    //index in argv array
        }

}

//Gets the command from the user and stores in uInput.
//Then checks for $$ to do pid expansion
//after that, a null character is added to the end of the uInput string
//string is then tokenized using a space delimeter, all tokens are stored in argv array
//looks for <, >, or &, if any are found, collects the relevant information for file i/o then shifts all arguments down argv
int getInput(struct Input *input){
        printf(": ");
        int i = 0;
        char *token;
                fgets(input->uInput, CMDMAX, stdin);
        if(skipFlag == 0){
                pidExpansion(input);
        }
        input->uInput[strlen(input->uInput) - 1] = NULL;
        if(strlen(input->uInput) == 0){
                return 1;
        }

        token = strtok(input->uInput, " ");
        while(token != NULL){
                input->argv[input->argNumb] = token;
                token = strtok(NULL, " ");
                input->argNumb++;
        }
        input->argv[input->argNumb] = NULL;

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
				if(backFlag == 0){
                                	input->background = 1;
				}
                        }
                }
        }
        return 0;
}

//simply displays the command, used in debugging string parsing
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

//initializes input struct. 517 is used so command can take 512 arguments plus the file i/o and background &
void initialize(struct Input *input){
        input->argv = (char**)malloc(sizeof(char*) * 517);
        input->inFileName = NULL;
        input->outFileName = NULL;
        input->background = 0;
        input->argNumb = 0;
        input->savedIn = dup(0);
        input->savedOut = dup(1);
}

//deconstructs the command after it is has been executed
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

//return status is used locally for obtaining the exit status of last commands
//updates array holding the PID of all children
//forks process, checks for error then moves on to child processes
void execute(struct Input *input){
	int returnStatus;
        pid_t pid = fork();
	childPid[numbChild] = pid;
	numbChild++;

        //child process checks for any input or output redirection then executes the command.
        //will return error if execution fails
        if(pid == -1){
		perror("fork failed");
                exit(1);
        }
	else if(pid == 0){
		if(input->inFileName || input->outFileName){
			openInOut(input);
		}
                if(input->background == 1){
			int devNull = open("/dev/null", O_WRONLY);
			dup2(devNull, 0);
		}


            if (execvp(input->argv[0], input->argv) < 0){
                printf("error executing\n");
                exit(1);
                }
        }

        //parent process sets the global for the lastChild pid, if command is not run in background, will remove the child pid from child pid array
        //if not in background, will wait for child to finish and then update the status of lastStatus
        //if child is run in background, prints the pid of child and will print the end of child process and child pid upon the execution of the next command after child has finished
        else{
                lastChild = pid;
                if(input->background == 0){
			childPid[numbChild] = 0;
			numbChild--;

			waitpid(pid, &returnStatus, 0);
			if(WIFEXITED(returnStatus)){
				lastStatus = WEXITSTATUS(returnStatus);
			}

              }
		if(input->background == 1){
			printf("background pid is %d\n", pid);
                        int w = waitpid(pid, &returnStatus, WNOHANG);
                        if (w > 0){
                                printf("process %d has finished\n", w);
                        }
		}
        }
}

//if just cd is called, this will change working directory to home, otherwise it will change the directory to whatever is in the first argument if it exists
void cd(struct Input *input){
	if(input->argNumb == 1){
		char HOME[256];
		strcpy(HOME, getenv("HOME"));
		chdir(HOME);
	}
	else{
	    chdir(input->argv[1]);
	}
}

//if file i/o is needed, this will open file and redirect input or output to/from it.
//if file cannot be opened or does not exist, this will throw an error
int openInOut(struct Input *input){
        if(input->inFileName){
                input->savedIn = dup(0);
                input->in = open(input->inFileName, O_RDONLY);
                if(input->in == -1){
                        perror("Could not open file");
                        exit(1);
	        }
                dup2(input->in, 0);
        }
        if(input->outFileName){
                input->savedOut = dup(1);
                input->out = open(input->outFileName, O_WRONLY | O_CREAT, 0640 );
	        if(input->out == -1){
                        perror("Could not open file");
                        exit(1);
                }
                dup2(input->out, 1);
        }
	return 0;
}

//closes file i/o if needed and resets stdin/stdout
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

//checks if command is comment, retruns 1 if it is so nothing else will be executed on comment.
//otherwise, returns 0 so command can continue to execute
int checkComment(struct Input *input){
        char temp1 = input->argv[0][0];
        char temp2 = '#';
        if(temp1 == temp2){
                return 1;
        }

        return 0;
}

//searches all uInput for '$$', if found, returns 1 or 0 if not found
int checkp(struct Input *input){
        int i;
        for(i = 0; i < strlen(input->uInput) - 1; i++){
                if(input->uInput[i] == '$' && input->uInput[i+1] == '$'){
                        return 1;
                }
        }
        return 0;
}

//if checkp returns 1, replaces '$$' with '%d', then uses sprintf to expand into pid in newArg string
//coppies newArg to uInput
void pidExpansion(struct Input *input){
        if(checkp(input) == 1){
                int pid = getpid();
                char newArg[CMDMAX];
                char *rep = strstr(input->uInput, "$$");
                rep[0] = '%';
                rep[1] = 'd';
                sprintf(newArg, input->uInput, pid);
                strcpy(input->uInput, newArg);
        }
}

//used to display the parent pid and pid of any children still running
void getParent(){
	int i;
	printf("\n\nParent PID: %d\n", parentPid);
	for(i = 0; i < numbChild; i++){
                if(childPid[i] > 0){
		               printf("Children: %d\n", childPid[i]);
                }
	}
}

//if SIGTSTP is signalled, will toggle the backFlag flag then sets skip flag so main loop will restart
void backstop(int p){
	if(backFlag == 0){
		printf("\nEntering foreground-only mode (& is now ignored)\n");
		backFlag = 1;
	}
	else{
		printf("\nExiting foreground-only mode (& function now restored)\n");
		backFlag = 0;
	}
	skipFlag = 1;

	fflush(stdin);
	fflush(stdout);
}

//if SIGINT is signalled will print the pid of child being executed, kill the child the set the status to SIGINT
void sigintHandler(int sig){
        skipFlag = 1;
        printf("\nEnding Process %d\n", lastChild);
        if(backFlag == 1){
                kill(lastChild, SIGINT);
        }
        lastStatus = SIGINT;
}

//gets the address of current directory so it can be reset after smallsh is done
//sets parentpid Global
//sets main loop
//checks for any changes in background processes ran, will display the exit status if they have finished
//sets the signal handlers SIGINT and SIGTSTP
int main(){
	getcwd(cwd, sizeof(cwd));
	parentPid = getpid();
        struct Input *input;
        int i = 0;
        while(1){
		pid_t child;
		int childStatus;
		while((child = waitpid(-1, &childStatus, WNOHANG)) > 0){
			if(WIFEXITED(childStatus)){
				printf("Process %d exited with status %d\n", child, WEXITSTATUS(childStatus));
			}
                        if(WIFSIGNALED(childStatus)){
                                printf("Process %d exited with signal %d\n", child, WTERMSIG(childStatus));
                        }
		}
                signal(SIGINT, sigintHandler);
		signal(SIGTSTP, backstop);

                //creates struct for new command and initializes it.
                //if nothing on the previous command has caused the need to skip any portion of execution, will get user Input
                //checks if there are any comments and that input is valid
                //looks to see if user entered exit so program can exit properly
                //checks for CD Input
                //checks for status input and will display the last exit status if called
                //sets background2 flag then executes command.
                //finally, it resets the skipFlag, deconstructs the user input and resets working directory before exiting
                input = malloc(sizeof(struct Input));
                initialize(input);
		if(skipFlag == 0)
                        i = getInput(input);
                if(i == 0 && skipFlag == 0){
                        if(checkComment(input) == 1){
                                i = 1;
                        }
                }
                if(i == 0 && skipFlag == 0){
                        backFlag2 = 0;
                        if((strcmp(input->argv[0],"exit" )) == 0){  //check for exit
				i = 1;
                                return 0;
                        }
                        if((strcmp(input->argv[0], "parent" )) == 0){  //check for exit
                                getParent();
				i = 1;
                        }
                        if(strcmp(input->argv[0], "cd") == 0){
                                cd(input);
                                i = 1;
                        }

                        if(strcmp(input->argv[0], "status") == 0){
                                printf("exit value: %d\n", lastStatus);
                                i = 1;
                        }
                        if(i == 0){
                                if(input->background == 1)
                                        backFlag2 = 1;
                                execute(input);
                        }
                }
                deconCommand(input);
		skipFlag = 0;
               i = 0;
        }
	chdir(cwd);
        return 0;
}

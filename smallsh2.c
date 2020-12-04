#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define stringMax 2048

struct Input{
	char uInput[stringMax];
	char command[stringMax];
	char arguments[512][stringMax];
	char fileInput[stringMax];
	char fileOutput[stringMax];
	int argNumb;
	int background;
};

void construct(struct Input *input){
	input = malloc(sizeof(struct Input));
	printf("input: ");
	scanf("%[^\n]s",input->uInput);
	printf("%s\n", input->uInput);
	int i;
	int count = 0;
	for(i = 0; i < sizeof(input->uInput); i++){
		if(strcmp(input->uInput[i],  " ") == 0){
			count++;
		}
	}
	printf("%d\n", count);
}

int main(){
	struct Input *input;

	construct(input);


	free(input);
	return 0;
}

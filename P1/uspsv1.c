#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "p1fxns.h"
#include <sys/wait.h>
#include <sys/types.h>

void free_args(char **args, int arg_count){
	if(args != NULL){
		for(int i = 0; i < arg_count; i++){
			if(args[i] != NULL){
				free(args[i]);
			}
		}
		free(args);
	}
}

int main(int argc, char* argv[]){
	
	int file = open(argv[1], O_RDONLY);
	char buf[BUFSIZ];

	int nprograms = 0;
	int linelength;
	
	while((linelength = p1getline(file, buf, BUFSIZ)) != 0){
		nprograms++;
		if(buf[linelength - 1] == '\n'){
			buf[linelength - 1] = '\0';
		}
		
		char prog[24];
		char **args;
		char tmp[1024];
		
		//Program name
		p1getword(buf, 0, prog);
		
		int i = 0;
		int firstit = 0;
		int arg_count = 0;
		
		//Count the number of args
		while((i = p1getword(buf, i, tmp)) != -1){	
			if(firstit != 0){
				arg_count++;
			}
			firstit = 1;
		}

		//Allocate memory for args 
		args = (char **) malloc(sizeof(char *) * (arg_count + 1));
		for(int i = 0; i < arg_count; i++){
			args[i] = malloc(sizeof(char) * 24);
		}
		
		//Fill in the **args
		int arg = 0;
		i = 0;
		firstit = 0;
		while((i = p1getword(buf, i, args[arg])) != -1){
			if(firstit != 0){
				arg++;
			}
			firstit = 1;
		}
		args[arg_count] = NULL;
		
		/*
		printf("LINE: %s\n", buf);
		printf("PROGRAM: %s\n", prog);
		printf("ARG COUNT: %d\n" , arg_count);
		printf("ARGUMENTS: \n");	
		
		for(int i = 0; i <= arg_count; i++){
			printf("args[%d]: %s\n", i, args[i]);
		}
		
		*/

		pid_t pid = fork();
		if(pid == 0){
			execvp(prog, args);
		}
		free_args(args, arg_count);
	
	}


	int status;
	for(int i = 0; i < nprograms; i++){
		wait(NULL);	
	}
	

}

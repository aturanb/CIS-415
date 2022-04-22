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

	
	int file;
	
	//Process Quantum
	char *p = getenv("USPS_QUANTUM_MSEC");
	/*
	for(int i = 0; i < argc; i++){
		//If the argument starts with "-q"
		if(p1strneq(argv[i], "-q", 2)){
			p = (argv[i] + 2);	
		}
		//If the argument is not a -q and argc is 1, use stdin
		else if(argc == 1){
			file = 0;
		}
		//Else assume it is a file
		else{ 
			file = open(argv[i], O_RDONLY);
		}
	}
	*/
	if(argc == 1){
		file = 0;
	}
	else if(argc == 2){
		if(p1strneq(argv[1], "-q", 2)){
			printf("wrong usage");	
		}
		else{
			file = open(argv[1], O_RDONLY);
		}
	}
	else if(argc == 3){
		
		if(p1strneq(argv[1], "-q", 2)){
			p = argv[2];
			file = 0;	
		}
		else{
			printf("Wrong Usage\n");
			exit(1);

		}
	}
	else if(argc == 4){
		if(p1strneq(argv[1], "-q", 2)){
			p = argv[2];
			file = open(argv[3], O_RDONLY);	
		}
	} 


	//No -q supplied and no Quantum env variable
	if(p == NULL){
		printf("NO QUANTUM");
		exit(1);
	
	}
	
	char buf[BUFSIZ];

	int nprograms = 0;
	int linelength;
	
	while((linelength = p1getline(file, buf, BUFSIZ)) != 0){
		nprograms++;
		if(buf[linelength - 1] == '\n'){
			buf[linelength - 1] = '\0';
		}
		
		char **args;
		char prog[32];
		char tmp[1024];
		int i = 0;
		int arg_count = 0;
		
		p1getword(buf, 0, prog);

		//Count the number of args
		while((i = p1getword(buf, i, tmp)) != -1){	
			arg_count++;
		}

		//Allocate memory for args 
		args = (char **) malloc(sizeof(char *) * (arg_count + 1));
		for(int i = 0; i < arg_count; i++){
			args[i] = malloc(sizeof(char) * 24);
		}
		
		//Fill in the **args
		int arg = 0;
		i = 0;
		while((i = p1getword(buf, i, args[arg])) != -1){
			arg++;
		}
		args[arg_count] = NULL;
		
		
		printf("LINE: %s\n", buf);
		printf("PROGRAM: %s\n", prog);
		printf("ARG COUNT: %d\n" , arg_count);
		printf("ARGUMENTS: \n");	
		
		for(int i = 0; i <= arg_count; i++){
			printf("args[%d]: %s\n", i, args[i]);
		}
		
		
		int status;
		pid_t pid = fork();
		if(pid < 0){
			printf("Fork Failed \n");
			free_args(args, arg_count);
			exit(1);
		}
		if(pid == 0){
			execvp(prog, args);
		}
		
		
		else{
			wait(&status);
			printf("Child Completed \n");
		}

		free_args(args, arg_count);
	
	}
	

}

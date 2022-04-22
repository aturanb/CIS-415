#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "p1fxns.h"
#include <sys/wait.h>
#include <sys/types.h>

typedef struct process{
	int pid;
	char **args;
} Process;	

Process *processes = NULL;

int nprograms = 0;


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

void add_to_processes(int nprogram, int pid, char *buf){
	//If it is the first program use malloc, else expand the size with realloc
	if(nprogram == 1){ processes = (Process *) malloc(sizeof(Process)); }
	else{ processes = (Process *) realloc(processes, (sizeof(Process) * nprogram)); }
	
	int tmp[128];
	int arg_count = 0;
	int i = 0;

	//count the number of arguments
	while((i = p1getword(buf, i, tmp)) != -1){ arg_count++; }
	
	
	//Allocate memory for args 
	processes[nprogram - 1].args = (char **) malloc(sizeof(char *) * (arg_count + 1));
	for(int i = 0; i < arg_count; i++){
		processes[nprogram - 1].args[i] = malloc(sizeof(char) * 24);
	}

	//Fill in the **args
	int arg = 0;
	i = 0;
	while((i = p1getword(buf, i, processes[nprogram - 1].args[arg])) != -1){
		arg++;
	}
	processes[nprogram -1].args[arg_count] = NULL;
	processes[nprogram - 1].pid = pid;
	
		

	
		
	
	}
	

int main(int argc, char* argv[]){
 
		
	
	
	
	
	
	int file;
	
	//Process Quantum
	char *p = getenv("USPS_QUANTUM_MSEC");

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
		p1perror(2, "Quantum undefined\n");
		exit(1);
	
	}
	
	char buf[BUFSIZ];

	//int nprograms = 0;
	int linelength;
	
	while((linelength = p1getline(file, buf, BUFSIZ)) != 0){
		nprograms++;
		if(buf[linelength - 1] == '\n'){
			buf[linelength - 1] = '\0';
		}
		
		
		char prog[32];	
		p1getword(buf, 0, prog);

		add_to_processes(nprograms, 0, buf);
		
		/*
		printf("LINE: %s\n", buf);
		printf("PROGRAM: %s\n", prog);
		printf("ARG COUNT: %d\n" , arg_count);
		printf("ARGUMENTS: \n");	
		
		for(int i = 0; i <= arg_count; i++){
			printf("args[%d]: %s\n", i, args[i]);
		}
		*/
		

		

		free_args(args, arg_count);
	
	}
	
	int status;
	for(int i = 0; i < nprograms; i++){
		for(int j = 0; j < 
		printf("args[i]: %s\n", processes[i].args
	}

}

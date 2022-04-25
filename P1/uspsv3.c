#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>

#include "p1fxns.h"
#include "ADTs/queue.h"

#define MAX_PCB 100
#define MAX_Q 2000
#define MIN_Q 20
#define MS_PER_TICK 20

#define UNUSED __attribute__((unused))

typedef struct pcb {
	pid_t pid;
	int ticks;
	bool isalive;
	bool usr1;
} PCB;

volatile int usr1 = 0;
volatile int active_processes = 0;

int num_procs = 0;
int ticks_in_quantum = 0;

pid_t parent;

PCB pcb_arr[MAX_PCB];
PCB *current;

const Queue *proc_queue; 

static int pid2index(pid_t pid) {
	int i;
	for(i = 0; i < num_procs; ++i) {
		if(pcb_arr[i].pid == pid)
			return i;
	}
	return -1;
}

static void usr1_handler(UNUSED int sig) {
	usr1++;
}

static void usr2_handler(UNUSED int sig) {
}

static void chld_handler(UNUSED int sig) { 

	int status;
	int i;
	pid_t pid;
	while((pid = waitpid(-1, &status, WNOHANG)) > 0){
		if(WIFEXITED(status) || WIFSIGNALED(status)){
			i = pid2index(pid);
			active_processes--;
			pcb_arr[i].isalive = false;
			kill(parent, SIGUSR2);
		}
	}
}

static void alrm_handler(UNUSED int sig) {
	if(current != NULL){
		if(current->isalive == true){
			current->ticks = current->ticks - 1;
			if(current->ticks > 0){
				return;
			}
			kill(current->pid, SIGSTOP);
			proc_queue->enqueue(proc_queue, ADT_VALUE(current));
		}
		current = NULL;
	}

	while(proc_queue->dequeue(proc_queue, ADT_ADDRESS(&current))){
		if(current->isalive == false)
			continue;
		current->ticks = ticks_in_quantum;
		if(current->usr1 == true){
			current->usr1 = false;
			kill(current->pid, SIGUSR1);
		}
		else{
			kill(current->pid, SIGCONT);
		}
		return;

	}
}

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

	//Set input to stdin, if file specified it will get overwritten later
	int file = 0;

	char *filename = NULL;

	//Get quantum the environment variable
	//It will be null if it does not exist
	char *q_char = getenv("USPS_QUANTUM_MSEC");
	
	for(int i = 1; i < argc; i++){
		//Overwrite q_char
		if(p1strneq(argv[i], "-q", 2) && (argc >= 3)){
			q_char = argv[i+1];
		}
		//Get the file
		else if (argc != 1 && (i != 2)){ 
			filename = argv[i];
		}
	}

	//No -q supplied and no Quantum Env
	if(q_char == NULL){
		p1perror(2, "Quantum undefined\n");
		return 1;
	}

	int q_int = p1atoi(q_char);

	//Check if quantum is a valid number
	if(q_int < MIN_Q || q_int > MAX_Q) {
		printf("Unreasonable quantum\n");
		return 1;
	}

	q_int = MS_PER_TICK * ((q_int + 1) / MS_PER_TICK);
	ticks_in_quantum = q_int / MS_PER_TICK;

	//Check if valid input supplied
	//If file specified, use that instead of stdin
	if(filename != NULL){  
		if((file = open(filename, O_RDONLY)) < 0){
			p1perror(2, "File cannot be opened\n");
			p1perror(2, "USAGE: ./uspsv3 [-q <quantum in msec>] [workload_file]\n");
			return 1;
		} 
	}

	/*
	//Initialize signals
	if(signal(SIGUSR1, usr1_handler) == SIG_ERR){
		p1perror(2, "SIGUSR1: could not initialize signal\n");
		return 1;
	}

	if(signal(SIGUSR2, usr2_handler) == SIG_ERR){
		p1perror(2, "SIGUSR2: could not initialize signal\n");
		return 1;
	}

	if(signal(SIGCHLD, chld_handler) == SIG_ERR){
		p1perror(2, "SIGCHLD: could not initialize signal\n");
		return 1;
	}

	if(signal(SIGALRM, alrm_handler) == SIG_ERR){
		p1perror(2, "SIGALRM: could not initialize signal\n");
		return 1;
	}
	*/
	signal(SIGUSR1, usr1_handler);
	signal(SIGUSR2, usr2_handler);
	signal(SIGCHLD, chld_handler);
	signal(SIGALRM, alrm_handler);
	
	parent = getpid();

	char buf[BUFSIZ];
	int linelength;
	struct itimerval interval;
	proc_queue = Queue_create(doNothing);
	while((linelength = p1getline(file, buf, BUFSIZ)) != 0){
		
		//Get rid of the newline character
		if(buf[linelength - 1] == '\n'){
			buf[linelength - 1] = '\0';
		}
	
		char **args;
		char tmp[64];
		int i = 0;
		int arg_count = 0;

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
		
		//Start fork process
		PCB *curr_proc = pcb_arr+num_procs;
		/*pid_t pid = fork();
		//If parent process
		if(pid > 0){
			curr_proc->pid = pid;
			curr_proc->isalive = true;
			curr_proc->usr1 = true;
			proc_queue->enqueue(proc_queue, ADT_VALUE(curr_proc));
		}
		//Failed to fork
		if(pid < 0){
			p1perror(2, "FORK FAILED\n");
			goto cleanup;
			return 1;
		}
		//Child process
		if(pid == 0){
			while(!usr1){
				pause();
			}
			execvp(args[0], args);
			break;
		}*/
		pid_t pid;
		switch(pid=fork()) {
			case -1:
				p1perror(2, "FORK FAILED\n");
				goto cleanup;
				return 1;
			case 0:
				while(!usr1)
					pause();
				execvp(args[0], args);
				break;
			default:
				curr_proc->pid = pid;
				curr_proc->isalive = true;
				curr_proc->usr1 = true;
				proc_queue->enqueue(proc_queue, ADT_VALUE(curr_proc));
				break;
		}

		num_procs++;

		free_args(args, arg_count);
	}

	active_processes = num_procs;

	

	interval.it_value.tv_sec = MS_PER_TICK/1000;
	interval.it_value.tv_usec = (MS_PER_TICK*1000) % 1000000;
	interval.it_interval = interval.it_value;
	if(setitimer(ITIMER_REAL, &interval, NULL) == -1) {
		p1perror(2,"bad timer\n");
		for(int i = 0; i < num_procs; ++i)
			kill(pcb_arr[i].pid, SIGKILL);
		goto cleanup;
	}

	alrm_handler(SIGALRM);	

	while(active_processes > 0)
		pause();

	goto cleanup;

cleanup:
	close(file);
	proc_queue->destroy(proc_queue);
	return 0;
}
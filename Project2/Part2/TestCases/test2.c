//Nathan Walzer - nwalzer

#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define __NR__cs3013_syscall2 378

struct ancestry{
	pid_t ancestors[10];
	pid_t siblings[100];
	pid_t children[100];
};

long callSys2(unsigned short *target, struct ancestry *storage){ //call modified syscall2
	return (long) syscall(__NR__cs3013_syscall2, target, storage);
}

int main(int argc, char* argv[]){
	struct ancestry *lineage = (struct ancestry*) malloc(sizeof(struct ancestry));
	long result;//the result of the callsys2 function
	unsigned short upid;
	//printf("Result of 0 indicates SUCCESS\n");
	
	int pid = fork(); //fork into parent and child
	
	if(pid == 0){ //if child
		unsigned short ppid = getppid(); //increases chances of getting forked parent process
		upid = getpid(); //get the pid
		result = callSys2(&upid, lineage);
		printf("Result of CHILD system call: %ld\n", result); //0 if successful
		if(!result){//if a valid result
			printf("This Process is: %d\n", upid);
			printf("FIRST Parent Process should be either: %d or 1, was actually %d\n", ppid, lineage->ancestors[0]);
			printf("This process should have no siblings: %d\n", lineage->siblings[0]);
			printf("This process should have no children: %d\n", lineage->children[0]);
		}
	} else { //if parent
		upid = getpid(); //get the pid
		result = callSys2(&upid, lineage);
		wait(NULL);
		printf("Result of PARENT system call: %ld\n", result); //0 if successful
		if(!result){//if a valid result
			printf("This Process is: %d\n", upid);
			printf("FIRST Parent Process should be: %d, was actually %d\n", getppid(), lineage->ancestors[0]);
			printf("This process may or may not have siblings: %d\n", lineage->siblings[0]);
			printf("This process may or may not have a child, depending on order or execution: %d\n", lineage->children[0]);
		}
	}
	
	return 0;
}

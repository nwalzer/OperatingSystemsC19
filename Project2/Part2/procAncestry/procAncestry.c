#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define __NR__cs3013_syscall2 378

struct ancestry{
	pid_t ancestors[10];
	pid_t children[100];
	pid_t siblings[100];
};

long callSys2(unsigned short *target, struct ancestry *storage){ //call modified syscall2
	printf("Starting test\n");
	return (long) syscall(__NR__cs3013_syscall2, target, storage);
}

int main(int argc, char* argv[]){
	if(argc < 2){
		printf("Invalid Usage\n");
		printf("./procAncestry <pid>\n");
		return 1;
	}
	unsigned short tmp = atoi(argv[1]);
	unsigned short *target = &tmp; //convert pid to pid pointer
	printf("Target: %u\n", *target);
	struct ancestry *lineage = (struct ancestry*) malloc(sizeof(struct ancestry));;
	long result;
	
	result = callSys2(target, lineage); //call modified system call using inputted pid
	
	printf("Result of system call: %ld\n", result); //0 if successful
	return 0;
}

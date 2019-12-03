//Nathan Walzer - nwalzer

#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define __NR__cs3013_syscall1 377

long testSys1(){ //call modified syscall2
	return (long) syscall(__NR__cs3013_syscall1);
}

int main(int argc, char* argv[]){
	printf("Result of Syscall1 test is %ld\n", testSys1());
	
	return 0;
}

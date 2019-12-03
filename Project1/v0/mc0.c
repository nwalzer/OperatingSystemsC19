//Nathan Walzer - nwalzer

#include <stdio.h>
#include <unistd.h>
#include <ctype.h> //used for fgets
#include <sys/types.h> //wait
#include <sys/wait.h> //wait
#include <sys/time.h> //getrusage and gettimeofday
#include <sys/resource.h> //getrusage

#define TRUE 1
#define BUFFSIZE 129

void genericPrompt(){
	printf("G'Day, Commander! What command would you like to run?\n");
	printf("\t0. whoami  : Prints out the result of the whoami command\n");
	printf("\t1. last    : Prints out the result of the last command\n");
	printf("\t2. ls      : Prints out the result of a listing on a user-specified path\n");
	printf("Option?: ");
	fflush(stdout);
} //prints generic menu

void runLast(){
	char* args[] = {"./last", NULL};
	printf("-- Last Logins --\n");
	execvp("last", args);
} //runs the "last" command

void runWhoAmI(){
	char* args[] = {"./whoami", NULL};
	printf("-- Who Am I? --\n");
	execvp("whoami", args);
} //runs the "whoami" command

void runLs(){
	char* args[] = {"./ls", NULL, NULL, NULL};
	char arg1[BUFFSIZE]; //ls does not take many arguments
	char path[BUFFSIZE]; //specified path can be very long
	printf("-- Directory Listing --\n");
	printf("Arguments?: ");
	fgets(arg1, sizeof(arg1), stdin);
	printf("\n");
	fflush(stdout);
	sscanf(arg1, "%s", arg1);
	//set arg1 to first string found ("\n" if no string found)
	printf("Path?: ");
	fgets(path, sizeof(path), stdin);
	
	printf("\n");
	fflush(stdout);
	sscanf(path, "%s", path);
	//set path to first string found ("\n" if no string found)
	if(!isspace(arg1[0]) && arg1[0] != '\n'){
	//if first character of arg1 is not whitespace
	//NOTE: first character is whitespace ONLY if no input given
		args[1] = arg1;
		if(!isspace(path[0]) && arg1[0] != '\n'){
		//if first character of path is not whitespace
		//NOTE: first character is whitespace ONLY if no input given
		args[2] = path;
	}
	} else {
		if(!isspace(path[0]) && arg1[0] != '\n'){
		//if first character of path is not whitespace
		//NOTE: first character is whitespace ONLY if no input given
		args[1] = path;
		}
	}
	
	execvp("ls", args);
} //run "ls" command

int main(int argc, char* argv[]){
	int option; //user option
	char optionBuff[BUFFSIZE]; //input buffer
	//input should not exceed 1 char. Set to 50 just in case
	struct rusage resrcUse; //used for page faults
	struct timeval start, end; //used for raw time of day
	long int startMili, endMili; //used for generating elapsed time
	int pgFault, recPgFault, prevPF = 0, prevRPF = 0; //used for generating page faults and reclaimed page faults

	printf("===== Mid-Day Commander, v0 =====\n");
	while(TRUE){ //run until user manually terminates
		genericPrompt(); //generic menu prompt
		fgets(optionBuff, BUFFSIZE, stdin); 
		//fgets allows for invalid input recognition
		if(!sscanf(optionBuff, "%d", &option)){
		//if no integers inputted, set option to invalid number
		//NOTE: empty input defaults to previous command
			option = -1;
		}
		//if user entered multiple numbers "1 2 0" program only takes first number
		
		
		gettimeofday(&start, NULL);
		int success = fork();
		//create child process

		if(success != 0){
		//if not child process
			if(success == -1){
			//if unsuccessful fork
				printf("Error creating child process\n\n");
				break;
				//Alter user and continue program
			}
			wait(NULL);
			//wait until child finishes
		} else {
		//if child process
			if(option == 0){
			//if command whoami
				runWhoAmI();
			} else if(option == 1){
			//if command last
				runLast();
			} else if(option == 2){
			//if command ls
				runLs();
			} else {
				//if fork successful but invalid user input
				printf("Invalid argument. Options are 0, 1, 2");
				return 0;
				//return to parent process
			}
			printf("Execv Error\n\n");
			return 0; 
			//by nature of execv, only executes when execv fails
			//return to parent process
		}
		
		printf("\n-- Statistics --\n");
		getrusage(RUSAGE_CHILDREN, &resrcUse);//update resrcUse
		pgFault = (int) resrcUse.ru_majflt - prevPF;
		//pgFault set to total PF - total previous PF
		prevPF += pgFault;
		//previous PFs incremented
		recPgFault = (int) resrcUse.ru_minflt - prevRPF;
		//recPgFault set to total RPF - total previous RPF
		prevRPF += recPgFault;
		//previous RPFs incremented
		
		gettimeofday(&end, NULL);
		startMili = (start.tv_sec * 1000) + (start.tv_usec / 1000);
		//turn start time into miliseconds
		endMili = (end.tv_sec * 1000) + (end.tv_usec / 1000) - startMili;
		//turn end time into miliseconds - start miliseconds

		printf("Elapsed Time: %ldms\n", endMili);
		printf("Page Faults: %d\n", pgFault);
		printf("Page Faults (reclaimed): %d\n", recPgFault);
		printf("\n");
	}
	return 0;
}

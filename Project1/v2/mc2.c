//Nathan Walzer - nwalzer

#include <stdio.h>
#include <unistd.h>
#include <ctype.h> //fgets
#include <stdlib.h> //calloc
#include <string.h> //strcpy
#include <sys/types.h> //wait
#include <sys/wait.h> //wait
#include <sys/time.h> //getrusage and gettimeofday
#include <sys/resource.h> //getrusage

#define TRUE 1
#define BUFFSIZE 129
#define CUSTARGS 32

typedef struct{
	struct timeval startT;
	int pid;
	int done;
} backP;

void genericPrompt(){
	printf("G'Day, Commander! What command would you like to run?\n");
	printf("\t0. whoami  : Prints out the result of the whoami command\n");
	printf("\t1. last    : Prints out the result of the last command\n");
	printf("\t2. ls      : Prints out the result of a listing on a user-specified path\n");
	fflush(stdout);
} //prints first half of generic menu

void genericPrompt2(){
	printf("\ta. add command : Adds a new command to the menu\n");
	printf("\tc. change directory : Changes process working directory\n");
	printf("\te. exit : Leave Mid-Day Commander\n");
	printf("\tp. pwd : Prints working directory\n");
	printf("\tr. running processes : Prints list of running processes\n");
	printf("Option?: ");
	fflush(stdout);
} //prints second half of generic menu

void dynamicPrompt(char options[][BUFFSIZE], int total){
	for(int i = 0; i < total; i++){
		printf("\t%d. %s: user-inputted command\n", i+3, options[i]);
	}//user-inputted commands added sequentially from 0, loop 0->last command
	fflush(stdout);
} //prints 

void runLast(){
	char* args[] = {"last", NULL};
	printf("-- Last Logins --\n");
	execvp("last", args);
} //runs the "last" command

void runWhoAmI(){
	char* args[] = {"whoami", NULL};
	printf("-- Who Am I? --\n");
	execvp("whoami", args);
} //runs the "whoami" command

void runLs(){
	char* args[] = {"ls", NULL, NULL, NULL};
	char arg1[BUFFSIZE]; //ls does not take many arguments
	char path[BUFFSIZE]; //specified path can be very long
	printf("-- Directory Listing --\n");
	printf("Arguments?: ");
	fflush(stdout);
	fgets(arg1, sizeof(arg1), stdin);
	printf("\n");
	sscanf(arg1, "%s", arg1);
	//set arg1 to first string found ("\n" if no string found)

	printf("Path?: ");
	fflush(stdout);
	fgets(path, sizeof(path), stdin);
	printf("\n");
	sscanf(path, "%s", path);
	//set path to first string found ("\n" if no string found)

	if(!isspace(arg1[0])){
	//if first character of arg1 is not whitespace
	//NOTE: first character is whitespace ONLY if no input given
		args[1] = arg1;
		if(!isspace(path[0])){
		//if first character of path is not whitespace
		//NOTE: first character is whitespace ONLY if no input given
			args[2] = path;
		}
	} else {
		if(!isspace(path[0])){
		//if first character of path is not whitespace
		//NOTE: first character is whitespace ONLY if no input given
			args[1] = path;
		}
	}
	
	execvp("ls", args);
} //run "ls" command

void runCD(){
	int notSuccessful = 1;
	char path[BUFFSIZE];

	printf("-- Change Directory --\n");
	printf("New Directory?: ");
	fflush(stdout);
	fgets(path, sizeof(path), stdin);
	sscanf(path, "%s", path);//get user input
	
	if(!isspace(path[0])){ //if not empty input
		notSuccessful = chdir(path);//attempt to change directory
	} //path[0] will only be a space if empty input
	if(!notSuccessful){
	//if successful, return
		return;
	}
	//otherwise alert user
	printf("No change in directory\n");
} //changes directory

void runPWD(){
	char path[BUFFSIZE];
	printf("-- Current Directory --\n");
	getcwd(path, sizeof(path));
	printf("Directory: %s\n\n", path);
} //prints current working directory

void runCC(char* command){
	char* args[33];
	char* toAdd;
	char buff[BUFFSIZE];
	int i;
	
	strcpy(buff, command);
	toAdd = strtok(buff, " ");//args[0]
	printf("-- Command: %s --\n", command);
	for(i = 0; i < 32; i++){//set all arguments to NULL
		args[i] = NULL;
	}
	i = 0;
	while(toAdd != NULL){//while there are still more arguments to add
		sscanf(toAdd, "%s", toAdd);//gets rid of newline character
		
		args[i] = toAdd;//put an argument into position args[i]
		i++;
		toAdd = strtok(NULL, " ");//get next argument
	}
	args[32] = NULL;

	execvp(args[0], args);//run given command
}//runs a user-inputted command

void runBCC(char* command){
	char* args[33];
	char* toAdd;
	char buff[BUFFSIZE];
	int i;
	strcpy(buff, command);
	toAdd = strtok(buff, " ");//args[0]
	
	for(i = 0; i < 32; i++){//set all arguments to NULL
		args[i] = NULL;
	}
	i = 0;
	while(toAdd != NULL){//while there are still more arguments to add
		sscanf(toAdd, "%s", toAdd);//gets rid of newline character
		//printf("Q%sQ\n", toAdd);
		if(strcmp(toAdd, "&") != 0){
			args[i] = toAdd;//put an argument into position args[i]
			i++;
		} /*else {
			printf("FOUND ONE\n");
		}*/
		toAdd = strtok(NULL, " ");//get next argument
	}
	args[32] = NULL;

	execvp(args[0], args);//run given command
}//runs a user-inputted command

int isBP(char* command){
	for(int i = 0; i < BUFFSIZE && command[i] != '\0'; i++){
		if(command[i] == '&'){
			return 1;
		}
	}
	return 0;
}

int cleanBP(backP BP[CUSTARGS], int total){
	int i = 0;
	int oneDone = 0;
	if(total == 0){
		return 0;
	}
	while(i < 1){
		for(int j = 0; j < total && j < CUSTARGS-1; j++){//checking for any that are done
			if(BP[j].done || oneDone){ //if any are done, shift everything over
				oneDone = 1;
				BP[j] = BP[j+1];
			}
		}
		if(oneDone){//if one is done, rerun the loop to catch more
			oneDone = 0;
			i = 0;
			total--;
		} else {
			i = 1;
		}
	}
	return total;
}

void runR(backP BP[CUSTARGS], int total){
	printf("-- Background Processes --\n");
	for(int i = 0; i < total; i++){
		printf("[%d] %d\n", i+1, BP[i].pid);
	}
}

void addBP(backP BP[CUSTARGS], int total, int pid, struct timeval start){
	backP newP;
	newP.startT = start;
	newP.pid = pid;
	newP.done = 0;
	
	BP[total-1] = newP;
}

int getPos(backP BP[CUSTARGS], int total, int pid){
	for(int i = 0; i < total; i++){
		if(BP[i].pid == pid){
			return i;
		}
	}
	return -1;
}

int stillWaiting(backP BP[CUSTARGS], int total){
	for(int i = 0; i < total; i++){
		if(!BP[i].done){
			return 1;
		}//return 1 if any are NOT finished
	}
	//if it can get through the whole array, all processes have finished
	return 0;
}

void printBStats(backP BP[CUSTARGS], int total, int* prevPF, int* prevRPF){
	int PID;
	int pos;
	int pgFault;
	int recPgFault;
	long startMili;
	long endMili;
	struct timeval end;
	struct rusage resrcUse;
	struct rusage use;
	
	PID = wait3(NULL, WNOHANG, &use);
	while(PID > 0){
		pos = getPos(BP, total, PID);
		printf("-- Job Complete [%d] --\n", pos+1);
		printf("Process ID: %d\n", PID);
		printf("\n-- Statistics --\n");
		BP[pos].done = 1;
		getrusage(RUSAGE_CHILDREN, &resrcUse);//update resrcUse
		pgFault = (int) resrcUse.ru_majflt - *prevPF;
		//pgFault set to total PF - total previous PF
		*prevPF += pgFault;
		//previous PFs incremented
		recPgFault = (int) resrcUse.ru_minflt - *prevRPF;
		//recPgFault set to total RPF - total previous RPF
		*prevRPF += recPgFault;
		//previous RPFs incremented
		startMili = (BP[pos].startT.tv_sec * 1000) + (BP[pos].startT.tv_usec / 1000);
		gettimeofday(&end, NULL);
		endMili = (end.tv_sec * 1000) + (end.tv_usec / 1000) - startMili;
		//turn end time into miliseconds - start miliseconds

		printf("Elapsed Time: %ldms\n", endMili);
		printf("Page Faults: %d\n", pgFault);
		printf("Page Faults (reclaimed): %d\n", recPgFault);
		printf("\n\n");
		PID = wait3(NULL, WNOHANG, NULL);
	}
}

int main(int argc, char* argv[]){
	char option = 'Q'; //user option
	char optionBuff[BUFFSIZE+1]; //input buffer
	char custComs[CUSTARGS-12][BUFFSIZE]; //user-inputted commands
	char custBuff[BUFFSIZE]; //user-inputted commands buffer
	backP runningBP[CUSTARGS]; //keep tabs on running processes
	char* endOfFile;
	struct rusage resrcUse; //used for page faults
	struct timeval start, end; //used for raw time of day
	long int startMili, endMili; //used for generating elapsed time
	int pgFault, recPgFault, prevPF = 0, prevRPF = 0; //used for generating page faults and reclaimed page faults
	int i, addedCommands = 0, success = 0;
	int currBP = 0, tempID = 0, pos = 0;
	
	for(i = 0; i < CUSTARGS; i++){
		backP nullBP = {.pid = 0, .done = 0};
		runningBP[i] = nullBP;
	}
	
	printf("===== Mid-Day Commander, v0 =====\n");
	while(TRUE){ //run until user manually terminates
		for(int i = 0; i < BUFFSIZE+1; i++){
			optionBuff[i] = '\0';
		}//set all of buffer to null
		
		
		printBStats(runningBP, currBP, &prevPF, &prevRPF);
		
		if(!isspace(option)){		
			genericPrompt(); //menu prompts
			dynamicPrompt(custComs, addedCommands);
			genericPrompt2();
		}	
		
		printBStats(runningBP, currBP, &prevPF, &prevRPF);
		endOfFile = fgets(optionBuff, BUFFSIZE+1, stdin);//read input
		printBStats(runningBP, currBP, &prevPF, &prevRPF);
		currBP = cleanBP(runningBP, currBP);
		
		if(optionBuff[128] != '\0'){
		//if character #129 is not null terminating or null
			fgets(optionBuff, BUFFSIZE+1, stdin);//gets rid of extra input
			printf("Error, input too long\n");
			continue;
		}
		if(sscanf(optionBuff, " %c", &option) == EOF || option == 'e' || endOfFile == NULL){
		//if exit, exit program
			printf("Checking for in-progress background commands...\n");
			while(stillWaiting(runningBP, currBP)){
				printBStats(runningBP, currBP, &prevPF, &prevRPF);
				currBP = cleanBP(runningBP, currBP);
			}
			printf("No more background commands detected\nGoodbye, Commander\n");
			return 0;
		}//see if user wants to exit or input stream is done
		if(isspace(option)){
			continue;
		}//if blank input, ignore
		if(option == 'a'){
			if(addedCommands == CUSTARGS-14){
				printf("You've added too many commands\n");
				continue;
			}
			printf("\n-- Add Command --\n");
			printf("Command to Add?: ");
			fflush(stdout);
			fgets(custBuff, sizeof(custBuff), stdin);
			char* toAdd = strtok(custBuff, " ");
			for(i = 0; i < BUFFSIZE; i++){
				custComs[addedCommands][i] = '\0';
			}
			i = 0;
			while(toAdd != NULL && i < 32){
			//makes sure there are a max of 32 arguments
			//NOTE: Command counts are argument
				sscanf(toAdd, "%s", toAdd);
				strcat(custComs[addedCommands], toAdd);
				i++;
				strcat(custComs[addedCommands], " ");
				toAdd = strtok(NULL, " ");
			}
			option = ' ';
			sscanf(custComs[addedCommands], " %c", &option);
			//used to see if input is blank
			if(isspace(option) || i == 0){
				printf("Error, command cannot be blank\n\n");
				option = 'q';
				continue;
			} //if blank input
			//else
			printf("Okay, added with ID %d\n\n", addedCommands+3);
			addedCommands++;//increment number of commands
			option = 'q';
			continue;
		}//if user chose option a
		if(option == 'c'){
			runCD();
			printf("\n");
			continue;
		}//if user chose option c
		if(option == 'p'){
			runPWD();
			continue;
		}//if user chose option p
		if(option == 'r'){
			runR(runningBP, currBP);
			continue;
		}

		gettimeofday(&start, NULL);
		success = fork();
		//create child process

		if(success != 0){
		//if not child process
			if(success == -1){
			//if unsuccessful fork
				printf("Error creating child process\n\n");
				break;
				//Alter user and continue program
			}
			
			if(atoi(&option) > 2 && atoi(&option) < (addedCommands + 3)){ 
			//if custom command
				if(isBP(custComs[atoi(&option)-3]) && currBP < 30){
					currBP = cleanBP(runningBP, currBP) + 1;
					addBP(runningBP, currBP, success, start);
					printf("-- Command: %s --\n", custComs[atoi(&option)-3]);
					pos = getPos(runningBP, currBP, success);
					printf("[%d] %d\n\n", pos+1, success);
					continue;
				}
			}
			printBStats(runningBP, currBP, &prevPF, &prevRPF);
			currBP = cleanBP(runningBP, currBP);
			
			tempID = wait3(NULL, 0, NULL);
			if(tempID > 0){
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
				startMili = (start.tv_sec * 1000) + (start.tv_usec / 1000);
				gettimeofday(&end, NULL);
				endMili = (end.tv_sec * 1000) + (end.tv_usec / 1000) - startMili;
				//turn end time into miliseconds - start miliseconds

				printf("Elapsed Time: %ldms\n", endMili);
				printf("Page Faults: %d\n", pgFault);
				printf("Page Faults (reclaimed): %d\n", recPgFault);
				printf("\n");
				
				printBStats(runningBP, currBP, &prevPF, &prevRPF);
				currBP = cleanBP(runningBP, currBP);
			}
			//wait until child finishes
		} else{
		//if child process
			if(option == '0'){
			//if command whoami
				runWhoAmI();
			} else if(option == '1'){
			//if command last
				runLast();
			} else if(option == '2'){
			//if command ls
				runLs();
			} else if(atoi(&option) > 2 && atoi(&option) < (addedCommands + 3)){ 
			//if custom command
				if(isBP(custComs[atoi(&option)-3])){
					if(currBP < 30){
						currBP++;
						runBCC(custComs[atoi(&option)-3]);
					} else {
						printf("Sorry, you are running too many background commands\n\n");
					}
				} else{
					runCC(custComs[atoi(&option)-3]);
				}
			} else {
				//if fork successful but invalid user input
				printf("\n\nInvalid argument\n\n");
				return 0;
				//return to parent process
			}
			printf("Execv Error\n\n");
			return 0; 
			//by nature of execv, only executes when execv fails
			//return to parent process
		}
	}
	return 0;
}

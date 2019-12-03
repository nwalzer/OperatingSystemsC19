Nathan Walzer - nwalzer

NOTE FOR ALL VERSIONS: I'm going to be honest, the ls function does not work with file inputs. Its behavior is unpredictable for blank inputs. My test cases do not cover the use of ls because incorrect execution of ls can screw up the rest of the program. HOWEVER, ls works just fine when you are manually entering things in, even if you just enter blanks. This is also the case for 'a' if you try to give it a blank user-inputted command. If I had more time then I could've worked this out... sorry. :(
Also, the Makefile only works if all 3 of the programs are in the same directory, though in this zip file they are in their own folders


v0 - Basic shell that runs 3 commands: whoami, last, and ls. When prompted, user inputs their choice of command (symbolized by 0, 1, and 2). The program then forks and execs the child process such that it runs the system call specified by the user.

Both whoami and last, regardless of capability to take in arguments, are run without arguments. ls is run with user inputted arguments (including no arguments at all).

Improper input is handled. Invalid choices (i.e. '3', 'G', and '') alert the user that the input is invalid and the user is prompted to choose again. Empty input is handled depending on the previously inputted command. Empty input (or all whitespace input) will register as the same as the previously entered input, whether valid or invalid.

If multiple characters are inputted:
	If input starts with a number, uses whole number as input (ex. "22" = inputting 22, "22J" == "22")
	If input is series of alphabet characters, even if valid input lies further down the line (ex. "aaa 0") input is registered invalid

Regardless of proper or improper input, statistics are printed showcasing the time elapsed, page faults, and reclaimed page faults.

Handles lines up to 128 characters long (excluding '\0' byte)

TEST CASE:
MC0TEST/MC0RESULT = test 0, 1. Ensure infinite loop of last option inputted.




v1 - Similar shell to v0. Runs same basic 3 commands exactly the same, however also allows for the user to: a - adds a command (WIP), e - exit the program, c - change directories, and p - print working directory.

The 3 basic commands run virtually the same, however user choice for input is handled as a character, not an integer. 

'a' command - adds a command into an array of user-inputted commands. There is space for 
'e' command - triggered by both entering 'e', an EOF character, or empty user input. Simply terminates the program.
'c' command - prompts the user to change the working directory. Utilizes chdir(). If user does not specify new directory (i.e. blank argument) or specifies invalid directory (does not exist) user is alerted and directory remains the same.
'p' command - displays the current working directory. Utilizes getcwd(). The value displayed by this command WILL change if 'c' is successful.

TEST CASES:
GENERICTEST/GENERICRESULT = test all commands (excluding ls). Run 'a' with > 32 arguments
TOOMUCHINPUT+EOF/TMI+EOFRESULT = test if input > 128 char, test if picks up EOF and exits
CUSTOMCOMMANDSBUFFTEST/CCBUFFRESULT = test for # user-inputted commands > 18 (should stop adding them). Run 'a' with > 128 character command



NOTE ON RETURNING BACKGROUND PROCESSES: The array containing background processes gets updated every time a finished process is waited on. If you have process 1 and 2, and 1 finishes first, when process 2 finishes it will be displayed as "Job [1]" because it is the first one in the list

v2 - The same as v1 except designed to allow for programs to execute in the background (signaled by "[command] &")

One new command added:
'r' command - prints a list of the background processes that are currently running

Background Processes:
Background processes are handled via their own data struct. The struct has 3 fields:
	timeval startT - the time the process started
	int pid - the pid associated with the process
	int done - 0 if not done, 1 if done
The running background processes are stored in an array. When a user goes to run a background process the start time is recorded, the process is begun (NOTE: the '&' is not passed to execvp), the pid is recorded, done is set to 0, and the process is added into the array of background processes. 
Later, right before and after the user is prompted for a command, the background processes are quickly checked to see if any are done. If none are done, it does nothing. If one is done it prints the statistics. The elapsed time is calculated by subtracting the stored starting time (in ms) from the now-updated end time (in ms). Page faults and reclaimed page faults are determined in exactly the same manner. When calling getrusage(), the fields for page faults and reclaimed page faults are not updated by a child process until said process has finished, at which point the total amount of PF and RPF, across all child processes, is given. This program keeps a running tally of the total for each. When the background process finishes the number of PF and RPF is set to the total given by getrusage() minus the previous total. The difference is the number of PF and RPF associated with the process being printed. The previous totals are now incremented by the difference, thus equaling the current total and making them available for the next calculation.
The amount of running background processes is limited to 30, however successful completion of a background process will free up space to run more. 
For improper input:
Uses first non-whitespace character as user's choice of command.
Handles lines up to 128 characters long (excluding '\0'). Input up to 128 is used, the rest is discarded.
If input for an option is blank the program treats it as option 'e' and exits. 
Improper input - Program takes in ONLY the first character of the inputted line as the user's argument ("abc" == "a", "10 b2" == "1"). If invalid choice, alerts the user. Empty input for the choice of commands to run is considered a choice for 'e', exiting the program.

TEST CASES:
CUSTOMCOMMANDSBUFFTEST/CCBUFFRESULT = test for # user-inputted commands > 18 (should stop adding them). Run 'a' with > 128 character command
MC2Test/MC2RESULT = test all commands (except for ls). Test invalid input. Run 'a' where # arguments > 32. Run command that doesn't exist ("QQQ"). Run background commands such that they end in different order than they started. Run 'r'. Run 'e' while background programs still running.
TOOMANYBACK/TMBRESULT = Try to run more background commands than the background array will allow
EOFTEST/EOFRESULT = See if program registers 'EOF' as call for 'e'
















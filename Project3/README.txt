Phase 1:
Takes in 7 arguments, all integers:
The number of costuming teams - 2 <= x <= 4
The number of pirate threads - 10 <= x <= 50
The number of ninja threads - 10 <= x <= 50
Average pirate costuming time - 0 < x
Average ninja costuming time - 0 < x
Average pirate arrival time - 0 < x
Average ninja arrival time - 0 < x

	The program takes in these arguments and runs a store simulation with them. The two thread types, pirate and ninja, are not allowed to be in the store at the same time, however multiple pirates or multiple ninjas are allowed in the store at once. 

	At runtime the program allocates the specified number of pirate and ninja threads and randomizes their arrival time according to approximate random normal distribution using average arrival times. As each thread arrives it has a series of steps it must perform in order to be served. First, the threads use a spinlock to gain entry to the store. This spinning checks not only for the store to have physical space for them to enter but also if they are allowed in (given pirate or ninja status) AND checks for a special value. 
	
	Since a waittime of 30 minutes means a free costume we have implemented a MAX value, specified as 25/avg service time. If the average service time is, say 5 seconds, then we assume the worst case scenario upon thread arrival: 5 threads of the same time arriving just under 5 seconds after the previous one. So thread one arrives at T = 0, two arrives at T = 4.9, three at T = 9.8, and so on for the last two. With an average service time of 5 we limit this to 5 threads before forcing the program to reevaluate allowing this one type of thread to continue accessing the store. This means that we'd have roughly 25 seconds of execution for a single type of thread before assessing whether we need to switch, leaving 5 seconds of leeway to avoid 30 min waits.

	Inside the store the threads check the costuming teams sequentially (represented as an array). This method leads to high service times for the first team and high free times for the last team, though this does not matter all that much. Threads acquire a team, change the team's status to occupied, and then sleeps for some random normal distribution of the average costuming time. Certain time values are set and calculated and it is assessed whether this thread will receive their costume for free. The thread then exits and decrements the appropriate variables to indicate physical space is available.


During execution of this program we see some relatively high (~45%) rate of free costuming. This results from two main issues:
1) The random number generator is not amazing. Random normal distribution means that most of the threads arrive close to each other and thus end up backing each other up. This issue is out of our control, and high costuming times mean that it is impossible to avoid free costuming.
2) The pirates and ninjas will cut each other in line (or so says our excuse). Although pirate 1 might arrive before pirate 2, pirate 2 may choose to cut the line and steal 1's spot, thus driving 1's wait time up. Pirates and ninjas are naturally dishonest and selfish and will do this quite often. The store does not enforce a strict queue and leaves it to the pirates to be nice to each other.

There are two test output files. The first file details a large scale test with 100 ninja and pirate threads (50 N, 50 P). This one shows that a lot of threads back up the queue and lead to free costumes.
The second file details a small scale test which is completely successful in ensuring fairness and maximization of profits.

On some runs we saw a lot of threads being backed up and having long wait times. This is caused by a lot of threads arriving all at once and all backing each other up. 


Phase 2:
This program takes in no parameters. It simulates a busy traffic intersection attempting to optimize traffic flow. There are four quadrants: NW, SW, SE, and NE. Cars approach from a random direction (N, S, E, W) and either go straight, turn right, or turn left. Going right uses only one quadrant, straight 2, and left 3. 

The way this optimizes traffic flow is relatively simple. As each car approaches the intersection (from a random direction and with a random destination) it gets placed in a master queue. There are 3 traffic cop threads allowing traffic through. These threads loop infinitely. The traffic cop threads will dequeue the first car in line, wait for its first quadrant to be free (sem_wait), then move the car into that quadrant. It will then call sleep(0), which allows another traffic cop thread to execute and potentially move a car. Once execution comes back to the original it will wait on the next quadrant, move into it, then post the previous quadrant and sleep(0) again.

This process is the same for all cars and is executed until the car leaves the intersection. There are only a few small edge case scenarios in which this process does not fully optimize traffic flow, namely we do not allow, under ANY circumstances, 4 cars to be in the intersection. This is part of a tradeoff to avoid deadlocks. If we allowed 4 cars in the insection at one time, and all 4 were going straight and were from different directions, we would see a deadlock. To prevent this we allow only 3 cars in the intersection at any time, which is why we have 3 traffic cops. 

There is one scenario when interpreting results that must be made known. The way this program loops infinitely it is possible for two cars to be enqueued, and even in the intersection, that have the same number. This is OKAY. The number (1-20) associated with each car does not affect the performance. For example:
Car 1 enters from the North going left.
Car 1 is in quadrant NW

Car 1 enters from the East going straight.
Car 1 is in quadrant NE

Car 1 is in quadrant SW
Car 1 is in quadrant NW

Car 1 is through the intersection
Car 1 is in quadrant SE
Car 1 is through the intersection

THIS IS ACCEPTABLE BEHAVIOR and everything is functioning normally, we just happen to have some overlap in numbers.

There is no test-run for this phase as it runs infinitely, and was run for 5 hours straight with no issue.






















#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#define TRUE 1
#define FALSE 0

#define EMPTY 0
#define PIRATE 1
#define NINJA 2
int room_state;				// current state of the room, one of the three values above
int num_pirates_waiting;	// number of pirates in line
int num_ninjas_waiting;		// number of ninjas in line
int room_occupants;			// number of people in the room
int num_teams;				// number of costuming teams
int free_pirates;		// number of free costumes given to pirates
int free_ninjas;		// number of free costumes given to ninjas
int total_visits;
int done;
int numLoops;
int queueSize;

pthread_mutex_t state;		// state lock
pthread_mutex_t team_lock;	// costuming team lock

int pirate_visit_time;
int ninja_visit_time;

int max_pirate;
int max_pirate_threshold;

int max_ninja;
int max_ninja_threshold;

struct actor {
	int num_visits;
	time_t wait_time;
	time_t visit_time;
	int gold_owed;
};

struct team {
	int free;		 		// whether the team is free or not, TRUE | FALSE
	int free_time;  		// the duration of time the team has been free
	long pirate_busy_time;  // the duration of time the team has been busy for pirates
	long ninja_busy_time; 	// the duration of time the team has been busy for ninjas
	long last_taken;
	long pirate_gold;
	long ninja_gold;
};

struct team **teams;

int normal_random(int time) {
	double a = drand48();
	double b = drand48();
	int ret = abs(time * (sqrt(-2 * log(a)) * cos(2 * M_PI * b)));
	return ret; //(time + ret);
}

void pirate(int arrival_time, struct actor *pirate_struct) {			// pirate thread function
	sleep(normal_random(arrival_time));
	struct timeval *tv = (struct timeval *)malloc(sizeof(struct timeval));
	gettimeofday(tv, NULL);
	pirate_struct->wait_time = tv->tv_sec;
	pthread_mutex_lock(&state);
	total_visits++;
	num_pirates_waiting++;
	pthread_mutex_unlock(&state);

	while(TRUE) {
		pthread_mutex_lock(&state);
		if((room_state == EMPTY || room_state == PIRATE) && max_pirate_threshold < max_pirate && room_occupants < num_teams) {
			break;
		}
		pthread_mutex_unlock(&state);
		sleep(1);
	}

	room_state = PIRATE;
	room_occupants++;
	num_pirates_waiting--;	
	max_pirate_threshold++;

	gettimeofday(tv, NULL);
	pirate_struct->wait_time = tv->tv_sec - pirate_struct->wait_time;
	pthread_mutex_unlock(&state);

	pthread_mutex_lock(&team_lock);
	int i = 0;
	for(i = 0; i < num_teams; i++) {
		if(teams[i]->free == TRUE) {
			teams[i]->free = FALSE;
			gettimeofday(tv, NULL);
			teams[i]->free_time += tv->tv_sec - teams[i]->last_taken;
			break;
		}
	}
	int service_time = normal_random(pirate_visit_time);
	teams[i]->pirate_busy_time += service_time;
	pthread_mutex_unlock(&team_lock);
	
	sleep(service_time);
	pirate_struct->visit_time = service_time;
	
	pthread_mutex_lock(&team_lock);
	teams[i]->free = TRUE;
	gettimeofday(tv, NULL);
	teams[i]->last_taken = tv->tv_sec;
	if(pirate_struct->wait_time < 30){
		teams[i]->pirate_gold += service_time;
		pirate_struct->gold_owed += service_time;
    	} else {
		free_pirates++;
	}
	pthread_mutex_unlock(&team_lock);

	//gettimeofday(tv, NULL);
	//pirate_struct->visit_time = tv->tv_sec - pirate_struct->visit_time;
	
	pthread_mutex_lock(&state);
	room_occupants--;
	if(room_occupants == 0) {
		if(num_ninjas_waiting > 0 && max_pirate_threshold >= max_pirate) {
			room_state = NINJA;
		}
		else {
			room_state = EMPTY;
		}
		max_pirate_threshold = 0;
	}
	pthread_mutex_unlock(&state);
	printf("PIRATE: Visit #%d\n", pirate_struct->num_visits);
	printf("Total wait time: %ld\n", pirate_struct->wait_time);
	printf("Total visit time: %ld\n", pirate_struct->visit_time);
	printf("Total Gold Owed: %d\n\n\n", pirate_struct->gold_owed);
	fflush(stdout);
	if(rand() % 4 == 1) {
		pirate_struct->num_visits++;
		pirate(arrival_time, pirate_struct);
	}
}

void ninja(int arrival_time, struct actor *ninja_struct) {			// ninja thread function
	sleep(normal_random(arrival_time));
	struct timeval *tv = (struct timeval *)malloc(sizeof(struct timeval));
	gettimeofday(tv, NULL);
	ninja_struct->wait_time = tv->tv_sec;
	pthread_mutex_lock(&state);
	total_visits++;
	num_ninjas_waiting++;
	pthread_mutex_unlock(&state);

	while(TRUE) {
		pthread_mutex_lock(&state);
		if((room_state == EMPTY || room_state == NINJA) && max_ninja_threshold < max_ninja && room_occupants < num_teams) {
			break;
		}
		pthread_mutex_unlock(&state);
		sleep(1);
	}
	room_state = NINJA;
	room_occupants++;
	num_ninjas_waiting--;
	fflush(stdout);
	max_ninja_threshold++;
	gettimeofday(tv, NULL);
	ninja_struct->wait_time = tv->tv_sec - ninja_struct->wait_time;
	pthread_mutex_unlock(&state);

	pthread_mutex_lock(&team_lock);
    int i = 0;
    for(i = 0; i < num_teams; i++) {
        if(teams[i]->free == TRUE) {
            teams[i]->free = FALSE;
            gettimeofday(tv, NULL);
            teams[i]->free_time += tv->tv_sec - teams[i]->last_taken;
            break;
        }
    }
    int service_time = normal_random(ninja_visit_time);
    teams[i]->ninja_busy_time += service_time;
    pthread_mutex_unlock(&team_lock);
    
    sleep(service_time);
    ninja_struct->visit_time = service_time;
    
    pthread_mutex_lock(&team_lock);
    teams[i]->free = TRUE;
    gettimeofday(tv, NULL);
    teams[i]->last_taken = tv->tv_sec;
    if(ninja_struct->wait_time < 30){
	teams[i]->ninja_gold += service_time;
	ninja_struct->gold_owed += service_time;
    } else {
	free_ninjas++;
    }
    pthread_mutex_unlock(&team_lock);

	pthread_mutex_lock(&state);
	room_occupants--;
	if(room_occupants == 0) {
		if(num_pirates_waiting > 0 && max_ninja_threshold >= max_ninja) {
			room_state = PIRATE;
		}
		else {
			room_state = EMPTY;
		}
		max_ninja_threshold = 0;
	}
	pthread_mutex_unlock(&state);
	printf("NINJA: Visit #%d\n", ninja_struct->num_visits);
	printf("Total wait time: %ld\n", ninja_struct->wait_time);
	printf("Total visit time: %ld\n", ninja_struct->visit_time);
	printf("Total Gold Owed: %d\n\n\n", ninja_struct->gold_owed);
	fflush(stdout);
	if(rand() % 4 == 1) {
		ninja_struct->num_visits++;
		ninja(arrival_time, ninja_struct);
	}
}

void pirate_thread(int *arrival_time) {
    struct actor *pirate_struct = (struct actor *)calloc(1, sizeof(struct actor));
    pirate_struct->num_visits = 0;
    pirate_struct->wait_time = 0;
    pirate_struct->visit_time = 0;
    pirate_struct->gold_owed = 0;
    pirate(*arrival_time, pirate_struct);
}

void ninja_thread(int *arrival_time) {
    struct actor *ninja_struct = (struct actor *)calloc(1, sizeof(struct actor));
    ninja_struct->num_visits = 0;
    ninja_struct->wait_time = 0;
    ninja_struct->visit_time = 0;
    ninja_struct->gold_owed = 0;
    ninja(*arrival_time, ninja_struct);
}

void counter(){
	while(!done){
		sleep(10);
		numLoops++;
		pthread_mutex_lock(&state);
		queueSize += num_pirates_waiting + num_ninjas_waiting;
		pthread_mutex_unlock(&state);
	}
}

int main(int argc, const char *argv[]) {
	if(argc < 8){
		printf("Invalid arguments\n");
		return 1;
	}
	num_teams = atoi(argv[1]);					// number of teams, 2:4
	if(num_teams < 2 || num_teams > 4){
		printf("Invalid number of teams (2-4)\n");
		return 1;
	}
	int num_pirates = atoi(argv[2]);			// number of pirates, 10:50
	if(num_pirates < 10 || num_pirates > 50){
		printf("Invalid number of pirates (10-50)\n");
		return 1;
	}
	int num_ninjas = atoi(argv[3]);				// number of ninjas, 10:50
	if(num_ninjas < 10 || num_ninjas > 50){
		printf("Invalid number of ninjas (10-50)\n");
		return 1;
	}
	pirate_visit_time = atoi(argv[4]);			// average time spent in shop for pirates
	if(pirate_visit_time < 0){
		printf("Invalid costuming time (>0)\n");
		return 1;
	}
	ninja_visit_time = atoi(argv[5]);			// average time spent in shop for ninjas
	if(ninja_visit_time < 0){
		printf("Invalid costuming time (>0)\n");
		return 1;
	}
	int pirate_arrive_time = atoi(argv[6]);		// average arrival time of pirates
	if(pirate_arrive_time < 0){
		printf("Invalid arrival time (>0)\n");
		return 1;
	}
	int ninja_arrive_time = atoi(argv[7]);		// average arrival time of ninjas
	if(ninja_arrive_time < 0){
		printf("Invalid arrival time (>0)\n");
		return 1;
	}


	struct timeval *tv = (struct timeval *)malloc(sizeof(struct timeval));
	int ninja_gold = 0;
	int pirate_gold = 0;
	srand(time(0));
	
	room_state = EMPTY;
	num_pirates_waiting = 0;
	num_ninjas_waiting = 0;
	free_pirates = 0;
	free_ninjas = 0;
	total_visits = 0;
	done = FALSE;
	numLoops = 0;
	queueSize = 0;
	pthread_mutex_init(&state, NULL);
	pthread_mutex_init(&team_lock, NULL);

	max_pirate = 25 / pirate_visit_time;
	if(max_pirate <= 0){
		max_pirate = 1;
	}
	max_ninja = 25 / ninja_visit_time;
	if(max_ninja <= 0){
		max_ninja = 1;
	}
	max_pirate_threshold = 0;
	max_ninja_threshold = 0;

	teams = (struct team **)calloc(num_teams, sizeof(struct team *));
	for(int i = 0; i < num_teams; i++) {
		gettimeofday(tv, NULL);
		teams[i] = (struct team *)malloc(sizeof(struct team));
		teams[i]->free = TRUE;
		teams[i]->last_taken = tv->tv_sec;
		teams[i]->pirate_gold = 0;
		teams[i]->ninja_gold = 0;
	}
	pthread_t *pirates = (pthread_t *)calloc(num_pirates, sizeof(pthread_t));
	pthread_t *ninjas = (pthread_t *)calloc(num_ninjas, sizeof(pthread_t));
	pthread_t queueCounter;
	int i;
	int pirates_made = 0;
	int ninjas_made = 0;
	for(i = 0; i < 50; i++) {
		if(pirates_made < num_pirates) {
			pthread_create(&pirates[i], NULL, (void *)&pirate_thread, &pirate_arrive_time); // creates each pirate thread in the array
			pirates_made++;
		}
		if(ninjas_made < num_ninjas) {
			pthread_create(&ninjas[i], NULL, (void *)&ninja_thread, &ninja_arrive_time);      // creates each ninja thread in the array
			ninjas_made++;
		}
	}
	pthread_create(&queueCounter, NULL, (void *)&counter, NULL);

	for(i = 0; i < num_pirates; i++) {
		pthread_join(pirates[i], NULL);		// joins every pirate thread
	}
	for(i = 0; i < num_ninjas; i++) {
		pthread_join(ninjas[i], NULL);		// joins every ninja thread
	}
	done = TRUE;

	for(i = 0; i < num_teams; i++) {
		gettimeofday(tv, NULL);
		teams[i]->free_time += tv->tv_sec - teams[i]->last_taken;
		
		printf("Team %d was free for %d seconds\n", i, teams[i]->free_time);
		printf("Team %d was busy with pirates for %ld seconds and busy with ninjas for %ld seconds\n", i, teams[i]->pirate_busy_time, teams[i]->ninja_busy_time);
		ninja_gold += teams[i]->ninja_gold;
		pirate_gold += teams[i]->pirate_gold;
	}
	printf("Ninjas had %d free costumes and owe %d gold\n", free_ninjas, ninja_gold);
	printf("Pirates had %d free costumes and owe %d gold\n", free_pirates, pirate_gold);
	printf("Average queue length was %d\n", queueSize / numLoops);
	printf("Average gold per visit: %d\n", (pirate_gold + ninja_gold) / total_visits);
	printf("Gross Revenue was %d\n", pirate_gold + ninja_gold);
	printf("Total Profit was %d\n", pirate_gold + ninja_gold - (5 * num_teams));
}

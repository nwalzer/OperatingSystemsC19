#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define TRUE 1
#define FALSE 0

#define NORTH 0
#define WEST 1
#define SOUTH 2
#define EAST 3

#define STRAIGHT 0
#define RIGHT 1
#define LEFT 2

sem_t ne;
sem_t nw;
sem_t se;
sem_t sw;
sem_t q_lock;
sem_t waiting;
sem_t numLock;
sem_t validAdd;

int ID;

struct car{
	int from;
	int dir;
	int num;
	struct car* next;
};

struct car queue;

void enqueue(struct car* head, struct car* toAdd){
	sem_wait(&q_lock);
	
	struct car* last = head;
	
	while(last->next != NULL){
		last = last->next;
	}
	
	toAdd->next = NULL;
	last->next = toAdd;
	
	sem_post(&waiting);
	sem_post(&q_lock);
}

struct car* dequeue(struct car* head){
	if(head->next == NULL){
		printf("NO CARS WAITING\n");
		return NULL;
	}
	
	sem_wait(&waiting);
	sem_wait(&q_lock);
	
	struct car* toRet = head->next;
	head->next = toRet->next;
	
	sem_post(&q_lock);
	return toRet;
}

void manager(){
	sleep(1);
	while(TRUE){
		struct car* thisOne = dequeue(&queue);
		if(thisOne != NULL){
			sem_post(&validAdd);
			if(thisOne->from == NORTH){ //if coming from North
				printf("\nCar %d entering from the NORTH\n", thisOne->num);
				if(thisOne->dir == STRAIGHT){ //if going straight
					printf("Going STRAIGHT\n\n");
					sem_wait(&nw);
					printf("Car %d in NW\n", thisOne->num);
					sleep(0);
					sem_wait(&sw);
					sem_post(&nw);
					printf("Car %d in SW\n", thisOne->num);
					sleep(0);
					sem_post(&sw);
				} else if(thisOne->dir == RIGHT){ //if going right
					printf("Going RIGHT\n\n");
					sem_wait(&nw);
					printf("Car %d in NW\n", thisOne->num);
					sleep(0);
					sem_post(&nw);
				} else { //if going left
					printf("Going LEFT\n\n");
					sem_wait(&nw);
					printf("Car %d in NW\n", thisOne->num);
					sleep(0);
					sem_wait(&sw);
					sem_post(&nw);
					printf("Car %d in SW\n", thisOne->num);
					sleep(0);
					sem_wait(&se);
					sem_post(&sw);
					printf("Car %d in SE\n", thisOne->num);
					sleep(0);
					sem_post(&se);
				}
				printf("Car %d through intersection\n", thisOne->num);
				fflush(stdout);
			} else if(thisOne->from == WEST){ //if coming from west
				printf("\nCar %d entering from the WEAST\n", thisOne->num);
				if(thisOne->dir == STRAIGHT){ //if going straight
					printf("Going STRAIGHT\n\n");
					sem_wait(&sw);
					printf("Car %d in SW\n", thisOne->num);
					sleep(0);
					sem_wait(&se);
					sem_post(&sw);
					printf("Car %d in SE\n", thisOne->num);
					sleep(0);
					sem_post(&se);
				} else if(thisOne->dir == RIGHT){ //if going right
					printf("Going RIGHT\n\n");
					sem_wait(&se);
					printf("Car %d in SE\n", thisOne->num);
					sleep(0);
					sem_post(&se);
				} else { //if going left
					printf("Going LEFT\n\n");
					sem_wait(&sw);
					printf("Car %d in SW\n", thisOne->num);
					sleep(0);
					sem_wait(&se);
					sem_post(&sw);
					printf("Car %d in SE\n", thisOne->num);
					sleep(0);
					sem_wait(&ne);
					sem_post(&se);
					printf("Car %d in NE\n", thisOne->num);
					sleep(0);
					sem_post(&ne);
				}
				printf("Car %d through intersection\n", thisOne->num);
				fflush(stdout);
			} else if(thisOne->from == SOUTH){ //if coming from south
				printf("\nCar %d entering from the SOUTH\n", thisOne->num);
				if(thisOne->dir == STRAIGHT){ //if going straight
					printf("Going STRAIGHT\n\n");
					sem_wait(&se);
					printf("Car %d in SE\n", thisOne->num);
					sleep(0);
					sem_wait(&ne);
					sem_post(&se);
					printf("Car %d in NE\n", thisOne->num);
					sleep(0);
					sem_post(&ne);
				} else if(thisOne->dir == RIGHT){ //if going right
					printf("Going RIGHT\n\n");
					sem_wait(&se);
					printf("Car %d in SE\n", thisOne->num);
					sleep(0);
					sem_post(&se);
				} else { //if going left
					printf("Going LEFT\n\n");
					sem_wait(&se);
					printf("Car %d in SE\n", thisOne->num);
					sleep(0);
					sem_wait(&ne);
					sem_post(&se);
					printf("Car %d in NE\n", thisOne->num);
					sleep(0);
					sem_wait(&nw);
					sem_post(&ne);
					printf("Car %d in NW\n", thisOne->num);
					sleep(0);
					sem_post(&nw);
				}
				printf("Car %d through intersection\n", thisOne->num);
				fflush(stdout);
			} else { //if coming from east
				printf("\nCar %d entering from the EAST\n", thisOne->num);
				if(thisOne->dir == STRAIGHT){ //if going straight
					printf("Going STRAIGHT\n\n");
					sem_wait(&ne);
					printf("Car %d in NE\n", thisOne->num);
					sleep(0);
					sem_wait(&nw);
					sem_post(&ne);
					printf("Car %d in NW\n", thisOne->num);
					sleep(0);
					sem_post(&nw);
				} else if(thisOne->dir == RIGHT){ //if going right
					printf("Going RIGHT\n\n");
					sem_wait(&ne);
					printf("Car %d in NE\n", thisOne->num);
					sleep(0);
					sem_post(&ne);
				} else { //if going left
					printf("Going LEFT\n\n");
					sem_wait(&ne);
					printf("Car %d in NE\n", thisOne->num);
					sleep(0);
					sem_wait(&nw);
					sem_post(&ne);
					printf("Car %d in NW\n", thisOne->num);
					sleep(0);
					sem_wait(&sw);
					sem_post(&nw);
					printf("Car %d in SW\n", thisOne->num);
					sleep(0);
					sem_post(&sw);
				}
				printf("Car %d through intersection\n", thisOne->num);
				fflush(stdout);
			}
		} else {
			sleep(1);
		}
	}
}

void driver(){
	struct car* me = (struct car*) malloc(sizeof(struct car));
	int id;
	//int i = 0;
	sem_wait(&numLock);
	ID++;
	id = ID;
	sem_post(&numLock);
	//sleep(rand()%4);
	while(TRUE){
		me->from = rand() % 4;
		me->dir = rand() % 3;
		me->num = id;
		me->next = NULL;
		
		sem_wait(&validAdd);
		enqueue(&queue, me);
		sleep(rand()%3);
	}
}

int main(int argc, char* argv[]){
	srand(time(0));
	sem_init(&ne, 0, 1);
	sem_init(&nw, 0, 1);
	sem_init(&se, 0, 1);
	sem_init(&sw, 0, 1);
	sem_init(&q_lock, 0, 1);
	sem_init(&waiting, 0, 0);
	sem_init(&numLock, 0, 1);
	sem_init(&validAdd, 0, 20);
	
	queue.next = NULL;
	ID = 0;
	
	pthread_t *cars = (pthread_t *) calloc(20, sizeof(pthread_t));
	pthread_t *managers = (pthread_t *) calloc(3, sizeof(pthread_t));
	
	int i;
	//while(TRUE){
		for(i = 0; i < 20; i++){
			pthread_create(&cars[i], NULL, (void*) driver, NULL);
		}
	
		pthread_create(&managers[0], NULL, (void*) manager, NULL);
		pthread_create(&managers[1], NULL, (void*) manager, NULL);
		pthread_create(&managers[2], NULL, (void*) manager, NULL);
	
		for(i = 0; i < 20; i++){
			pthread_join(cars[i], NULL);
		}
		pthread_join(managers[0], NULL);
		pthread_join(managers[1], NULL);
		pthread_join(managers[2], NULL);
	//}
}





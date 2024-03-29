typedef int buffer_item;
#define BUFFER_SIZE 5

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define DINING 0


//----------------------------------------PRODUCER CONSUMER SEGMENT----------------------------------------------
#define RAND_DIVISOR 100000000
#define TRUE 1

pthread_mutex_t mutex;

sem_t full, empty;

/* the buffer */
buffer_item buffer[BUFFER_SIZE]={1,2,3,4,5};
int counter;
int ret;
int count=6;

pthread_t tid;       //Thread ID
pthread_attr_t attr; //Set of thread attributes

void *producer(void *param); /* the producer thread */
void *consumer(void *param); /* the consumer thread */
int remove_item(buffer_item *item);
int insert_item(buffer_item item);

void initializeData() {

    /* Create the mutex lock */
    pthread_mutex_init(&mutex, NULL);

    /* Create the full semaphore and initialize to 0 */
    ret = sem_init(&full, 0, 0);

    /* Create the empty semaphore and initialize to BUFFER_SIZE */
    ret = sem_init(&empty, 0, BUFFER_SIZE);

    /* Get the default attributes */
    pthread_attr_init(&attr);

    /* init buffer */
    counter = 0;
}

/* Producer Thread */
void *producer(void *param) {
    buffer_item item;

    printf("Producer created!\n");
    fflush(stdout);
    while(TRUE) {
        /* sleep for a random period of time */
        int rNum = rand() / RAND_DIVISOR;
        sleep(rNum);

        //Lock the mutex to block anybody else from updating the count variable while we are reading and writing to it
        pthread_mutex_lock(&mutex);
        item = count++;
        //Unlock the mutex as we are done with the global variable count have have stored it's old value in item
        pthread_mutex_unlock(&mutex);

        if(insert_item(item)) {
            fprintf(stderr, " Producer %ld report error condition\n", (long) param);
	    fflush(stdout);
        }
        else {
            printf("producer %ld produced %d\n", (long) param, item);
        }
	fflush(stdout);
    }
}

/* Consumer Thread */
void *consumer(void *param) {
    buffer_item item;

    while(TRUE) {
        /* sleep for a random period of time */
        int rNum = rand() / RAND_DIVISOR;
        sleep(rNum);

        if(remove_item(&item)) {
            fprintf(stderr, "Consumer %ld report error condition\n",(long) param);
	    fflush(stdout);
        }
        else {
            printf("consumer %ld consumed %d\n", (long) param, item);
        }
    }
}

/* Add an item to the buffer */
int insert_item(buffer_item item) {
	// wait until the buffer is not full
	sem_wait(&empty);
	// Lock the mutex so no two threads modify the counter and buffer at the same time
	pthread_mutex_lock(&mutex);

    /* When the buffer is not full add the item
     and increment the counter*/
    if(counter < BUFFER_SIZE) {
        buffer[counter] = item;
        counter++;

        // unlock as we are done modifying the buffer and counter
        pthread_mutex_unlock(&mutex);

        // Sends to the consumers that there is a new item to consume
		sem_post(&full);
        return 0;
    }
    else { /* Error the buffer is full */
        return -1;
    }
}

/* Remove an item from the buffer */
int remove_item(buffer_item *item) {
	// Wait until there are items to remove
	sem_wait(&full);
	// Lock the mutex so no two threads modify the counter and buffer at the same time
	pthread_mutex_lock(&mutex);

	/* When the buffer is not empty remove the item
	     and decrement the counter */
    if(counter > 0) {
        *item = buffer[(counter-1)];
        counter--;

        // unlock as we are done modifying the buffer and counter
        pthread_mutex_unlock(&mutex);

        // Sends to the producers that there is a new space to add items to
        sem_post(&empty);
        return 0;
    }
    else { /* Error buffer empty */
    	exit(1);
        return -1;
    }
}
//----------------------------------------PRODUCER CONSUMER SEGMENT----------------------------------------------

//----------------------------------------DINING PHILOSOPHERS SEGMENT--------------------------------------------

#define NUM_PHILOSOPHERS 5

// Forks
pthread_mutex_t forks[NUM_PHILOSOPHERS];

// sleep for 10 000 us or 10 ms to represent the thinking time
void think() {
	usleep(10000);
}


// The philosophers eating time
void eat() {
	usleep(10000);
}

void pickup(pthread_mutex_t* left_fork, pthread_mutex_t* right_fork) {
	// Loops until both forks have been successfully picked up
	while(1) {
		// Locking(picking them up) both forks so that no one else tries to use our forks
		pthread_mutex_lock(left_fork);

		/* Check if we can pickup the right fork and if we can do so,
		 * if we cannot we need to put down the left fork so someone else can pick it up*/
		if (pthread_mutex_trylock(right_fork) != 0) {
			pthread_mutex_unlock(left_fork);
			continue;
		}

		// We have been able to pickup both forks so we break out and then return
		break;
	}
}

void putdown(pthread_mutex_t* left_fork, pthread_mutex_t* right_fork) {
	// Unlocking(putting them back down) both forks so that our neighbors can use them
	pthread_mutex_unlock(right_fork);
	pthread_mutex_unlock(left_fork);
}

void *philosopher(void* param) {
	long i = (long)param;
	while (1) {
		// THINK;
		think();
		// PICKUP(FORK[i], FORK[i+1 mod 5]);
		pickup(&forks[i], &forks[(i+1) % NUM_PHILOSOPHERS]);
		printf("philosopher %ld has picked up forks\n", i);
		// EAT;
		eat();
		printf("philosopher %ld has eaten\n", i);
		// PUTDOWN(FORK[i], FORK[i+1 mod 5])
		printf("philosopher %ld puts down forks\n", i);
		putdown(&forks[i], &forks[(i+1) % NUM_PHILOSOPHERS]);
	}
}

void dining_philosophers()
{
	// List of philosopher threads
	pthread_t philosophers[NUM_PHILOSOPHERS];
	/* Get the default attributes */
	pthread_attr_init(&attr);

	for (long i = 0; i < NUM_PHILOSOPHERS; i++) {
		pthread_create(&philosophers[i], &attr, philosopher, (void*)i);
	}

	for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
		pthread_join(philosophers[i], NULL);
	}
}
//----------------------------------------DINING PHILOSOPHERS SEGMENT--------------------------------------------
int main(int argc, char *argv[]) {

    long i;
    void * status;
    /* Verify the correct number of arguments were passed in */
    //if(argc != 4) {
    //    fprintf(stderr, "USAGE:./main.out <INT> <INT> <INT>\n");
    //}

    int mainSleepTime = 30;//atoi(argv[1]); /* Time in seconds for main to sleep */
    int numProd = 10; //atoi(argv[2]); /* Number of producer threads */
    int numCons = 3; //atoi(argv[3]); /* Number of consumer threads */

    if (DINING == 1)
    {
    	//start dining philosophers
    	dining_philosophers();
    }
    else //Start producer consumer
    {
    	initializeData();

    	/* Create the producer threads */
    	for(i = 0; i < numProd; i++)
    	{
    	    pthread_create(&tid,&attr,producer,(void*) i);
    	}

    	/* Create the consumer threads */
    	for(i = 0; i < numCons; i++)
    	{
    	    pthread_create(&tid,&attr,consumer, (void*) i);
    	}
    	pthread_join(tid, &status);
    	/* Sleep for the specified amount of time in milliseconds */
    	sleep(mainSleepTime);


    	/* Exit the program */
    	printf("Exit the program\n");
    }


    exit(0);
}

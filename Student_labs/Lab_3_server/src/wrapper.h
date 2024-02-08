#ifndef WRAPPER_H
#define WRAPPER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>
#include <pthread.h>
#include <errno.h>

#define MAX_SIZE 1024

extern int MQcreate (mqd_t * mq, const char * name);
extern int MQconnect (mqd_t * mq, const char * name);
extern ssize_t MQread (mqd_t mq, void * buffer, size_t buffer_length);
extern ssize_t MQwrite (mqd_t mq, const void * data, size_t data_length);
int MQclose(mqd_t mq);




// Struct for planet data will be used in lab 2 and 3 !!!!!
// Just ignore in lab1 or you can try to send it on your mailslot,
// will be done in lab 2 and 3

typedef struct pt {
	char		name[20];	// Name of planet
	double		sx;			// X-axis position
	double		sy;			// Y-axis position
	double		vx;			// X-axis velocity
	double		vy;			// Y-axis velocity
	double		mass;		// Planet mass
	int 		color;		// Planet color
	int 		radius;		// Planet radius
	struct pt*	next;		// Pointer to next planet in linked list
	int			life;		// Planet life
	char		pid[30];	// String containing ID of creating process
} planet_type;

#endif /* WRAPPER_H */

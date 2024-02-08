/*
 ============================================================================
 Name        : Lab_3_client.c
 Author      : Jakob Danielsson
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "wrapper.h"


planet_type premade[4] = {
	{
		.name = "Sun",
		.mass = 100000000,
		.sx = 300,
		.sy = 300,
		.vx = 0,
		.vy = 0,
		.radius = 40,
		.color = 0xeebb33ff,
		.life = 1000000,
	},
	{
		.name = "Earth",
		.mass = 1000,
		.sx = 200,
		.sy = 300,
		.vx = 0,
		.vy = 0.008,
		.radius = 20,
		.color = 0x22cc55ff,
		.life = 1000000,
	},
	{
		.name = "Comet",
		.mass = 1000,
		.sx = 500,
		.sy = 300,
		.vx = 0.1,
		.vy = 0,
		.radius = 10,
		.color = 0x33833aff,
		.life = 1000000,
	},
	{
		.name = "Dying star",
		.mass = 1000000000,
		.sx = 500,
		.sy = 400,
		.vx = 0,
		.vy = 0,
		.radius = 60,
		.color = 0xbb7705ff,
		.life = 100,
	}
};

void clear() {
	int c;
	while ((c = getchar()) != '\n' && c != EOF) {}
}

void from_scratch(planet_type* planet) {
	printf("Enter planet details\nName: ");
	scanf("%s", planet->name);
	//clear();
	printf("X Position: ");
	scanf("%lf", &planet->sx);
	printf("Y Position: ");
	scanf("%lf", &planet->sy);
	printf("X Velocity: ");
	scanf("%lf", &planet->vx);
	printf("Y Velocity: ");
	scanf("%lf", &planet->vy);
	printf("Mass: ");
	scanf("%lf", &planet->mass);
	printf("Radius: ");
	scanf("%d", &planet->radius);
	printf("Color in hex: ");
	scanf("%x", &planet->color);
	printf("Life: ");
	scanf("%d", &planet->life);
}

void from_premade(planet_type* planet) {
	int num_premade = sizeof(premade) / sizeof(planet_type);
	for(int i = 0; i < num_premade; i++) {
		printf("%d\t%s\n", i, premade[i].name);
	}

	int choosen = -1;

	while (choosen < 0 || choosen >= num_premade) {
		printf("Choose a number between 0 and %d :", num_premade);
		scanf("%d", &choosen);
	}

	*planet = premade[choosen];
}

void take_input(planet_type* planet) {
	char input[20];
	while(1) {
		puts("Do you want to create your own planet(yes) or choose a premade(no) yes/no?");
		scanf("%s", input);
		if (strcmp(input, "yes") == 0) {
			from_scratch(planet);
			break;
		} else if (strcmp(input, "no") == 0) {
			from_premade(planet);
			break;
		}
		puts("You have to enter \"yes\" or \"no\"!");
	}
}

void pid_string(char* buffer, int buffer_length) {
	pid_t pid = getpid();
	snprintf(buffer, buffer_length, "/death_queue_%d", pid);
}

void* server_messages(void* args) {
	mqd_t* mq = args;

	while(1) {
		char buffer[MAX_SIZE];
		if(!MQread(*mq, buffer, MAX_SIZE)) {
			pthread_exit((void*)EXIT_FAILURE);
		}
		puts(buffer);
	}

	pthread_exit((void*)EXIT_SUCCESS);
}

int main(void)
{
	char pid[30];
	mqd_t planet_queue, death_queue;
	pthread_t thread;

	pid_string(pid, sizeof(pid));

	if(!MQconnect(&planet_queue, "/planet_queue")) {
		return EXIT_FAILURE;
	}

	if(!MQcreate(&death_queue, pid)) {
		return EXIT_FAILURE;
	}


	pthread_create(&thread, NULL, server_messages, &death_queue);

	while (1) {
		planet_type planet = { 0 };
		take_input(&planet);
		strncpy(planet.pid, pid, sizeof(planet.pid));
		MQwrite(planet_queue, &planet, sizeof(planet_type));
		puts("Sent new planet to the server");
	}

	if(!MQclose(planet_queue)) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

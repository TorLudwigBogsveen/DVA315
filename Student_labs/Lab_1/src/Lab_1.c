/*
 ============================================================================
 Name        : Lab_1.c
 Author      : Ludwig Bogsveen
 Version     :
 Copyright   : Ludwig Bogsveen
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "wrapper.h"
#include <pthread.h>
#include <unistd.h>

void* print_forever(void* arg) {
	const char* text = arg;
	while(1) {
		puts(text);
		usleep(200000);
	}
	pthread_exit(NULL);
}

void task1() {
	for(int i = 0; i < 10; i++) {
		puts("Hello World");
		sleep(1);
	}
}

void task2() {
	pthread_t t;

	if(pthread_create(&t, NULL, print_forever, "Hello Moon") != 0) {
		perror("pthread_create() error");
		exit(1);
	}

	for(int i = 0; i < 10; i++) {
		puts("Hello World");
		sleep(1);
	}

//	if (pthread_join(t, NULL) != 0) {
//		perror("pthread_join() error");
//		exit(3);
//	}
}

struct Task3 {
	pthread_mutex_t* mut;
	const char* text;
};

void* print_some(void* arg) {
	struct Task3* task = arg;
	while(1) {
		usleep(200000);
		pthread_mutex_lock(task->mut);

		for (int i = 0; i < 10; i++) {
			puts(task->text);
			usleep(200000);
		}

		pthread_mutex_unlock(task->mut);
	}

	pthread_exit(NULL);
}

void task3() {
	pthread_mutex_t mut;
	pthread_mutex_init(&mut, NULL);

	pthread_t t;

	struct Task3 task_data = {
		.mut = &mut,
		.text = "Hello Moon"
	};

	if(pthread_create(&t, NULL, print_some, &task_data) != 0) {
		perror("pthread_create() error");
		exit(1);
	}

	while(1) {
		usleep(200000);
		pthread_mutex_lock(&mut);

		for (int i = 0; i < 10; i++) {
			puts("Hello World");
			usleep(200000);
		}

		pthread_mutex_unlock(&mut);
	}

//	if (pthread_join(t, NULL) != 0) {
//		perror("pthread_join() error");
//		exit(3);
//	}
}

void* server_run_a() {
	mqd_t mq;
	if (MQcreate(&mq, "/lab_1_messages") == 0) {
		perror("Failed to create message queue");
		pthread_exit(NULL);
		return NULL;
	}
	while (1) {
		const size_t buffer_length = MAX_SIZE;
		char buffer[buffer_length];
		scanf("%s", buffer);
		MQwrite(mq, buffer, buffer_length);
	}
	pthread_exit(NULL);
}

void* client_run_a() {
	mqd_t mq;
	if (MQconnect(&mq, "/lab_1_messages") == 0) {
		perror("Failed to connect to message queue");
		pthread_exit(NULL);
		return NULL;
	}

	while (1) {
		const size_t buffer_length = MAX_SIZE;
		char buffer[buffer_length];
		int a = MQread(mq, buffer, buffer_length);
		if (a == -1) {
			perror("MQRead");
			break;
		}
		if (strcmp("END", buffer) == 0) {
			break;
		}
		puts(buffer);

	}

	pthread_exit(NULL);
}

void task_a() {
	pthread_t server, client;
	pthread_create(&server, NULL, server_run_a, NULL);
	pthread_create(&client, NULL, client_run_a, NULL);

//	if (pthread_join(server, NULL) != 0) {
//		perror("pthread_join() error");
//		exit(3);
//	}

	if (pthread_join(client, NULL) != 0) {
		perror("pthread_join() error");
		exit(1);
	}
}

struct TaskB {
	char* data;
	size_t data_len;
};

void* server_run_b() {
	mqd_t mq;
	if (MQcreate(&mq, "/lab_1_messages") == 0) {
		perror("Failed to create message queue");
		pthread_exit(NULL);
		return NULL;
	}
	while (1) {
		struct TaskB t = {
			.data = malloc(MAX_SIZE * sizeof(char)),
			.data_len = MAX_SIZE
		};
//		struct mq_attr attr;
//		mq_getattr(mq, &attr);
//		printf("%d", attr.mq_msgsize);
		scanf("%s", t.data);
		MQwrite(mq, &t, sizeof(struct TaskB));
	}
	pthread_exit(NULL);
}

void* client_run_b() {
	mqd_t mq;
	if (MQconnect(&mq, "/lab_1_messages") == 0) {
		perror("Failed to connect to message queue");
		pthread_exit(NULL);
		return NULL;
	}

	while (1) {
		struct TaskB t;
		char buffer[MAX_SIZE];
		int a = MQread(mq, buffer, MAX_SIZE);

//		struct mq_attr attr;
//		mq_getattr(mq, &attr);
//		printf("%d", attr.mq_msgsize);
//		return NULL;

		if (a == -1) {
			perror("MQRead");
			return NULL;
		}
		memcpy(&t, buffer, sizeof(struct TaskB));
		if (strcmp("END", t.data) == 0) {
			break;
		}
		puts(t.data);

	}

	pthread_exit(NULL);
}

void task_b() {
	pthread_t server, client;
	pthread_create(&server, NULL, server_run_b, NULL);
	pthread_create(&client, NULL, client_run_b, NULL);

//	if (pthread_join(server, NULL) != 0) {
//		perror("pthread_join() error");
//		exit(3);
//	}

	if (pthread_join(client, NULL) != 0) {
		perror("pthread_join() error");
		exit(3);
	}
}


int main(void)
{
	//task1();
	//task2();
	//task3();
	//task_a();
	task_b();
	return 0;
}

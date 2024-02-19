/*
 ============================================================================
 Name        : OSsched.c
 Author      : Jakob Danielsson
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : RR scheduler simulator, Ansi-style
 ============================================================================
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<string.h>
#include<time.h>
#include<sys/param.h>

#define sched_RR 1
#define sched_SJF 2
#define sched_MQ 3

#define QUEUE_SIZE 10
int sched_type = sched_MQ;
int finished = 0;
int context_switch_program_exit = 0;
int context_switch = 0;
int OS_cycles = 0;
int context_switches = 0;

typedef struct taskprop{
    int deadline;		//Deadline of a task, not necesarry to use
    int period;			//Periodicity of a task, not necesarry to use if you dont want to create a periodic scheduler such as rate monotonic. If you want to do this, talk with an assistant first
    int release_time;		//The time when a task is supposed to start, i.e., released from waiting queue
    int priority;		//Priority of task, can be used for the multiple queues
    int ID;			//ID, to distinguish different tasks from eachother
    int quantum;		//How long the task has left to execute
    int priority_quantum; //How long the task has left in a specific priority queue
    int queue_size;
    struct taskprop * next;
    struct taskprop * previous;
} task;

typedef struct prioritytaskprop {
	task* task;
	struct prioritytaskprop* next;
} prioritytask;

task * ready_queue = NULL;
task * waiting_queue = NULL;
task * idle_task;
int last_task = 0;

prioritytask* priority_queues[3] = {NULL, NULL, NULL};
int priority_quantum[3] = {1, 2, 4};

task tasks[QUEUE_SIZE]; //Queue
int activeTasks = 0;
int idleTasks = 0;

//Implementera insertfunktioner





//------------------Linked list functions------------------
void copy_task(task ** dest, task * src)		//Copies data of a task to another task
{
	task * temp = *dest;
	temp->ID = src->ID;
	temp->deadline = src->deadline;
	temp->release_time = src->release_time;
	temp->period = src->period;
	temp->priority = src->priority;
	temp->quantum = src->quantum;
	temp->priority_quantum = src->priority_quantum;
	*dest = temp;
}

void swap(task* a, task* b) {
	task tmp = *a;
	copy_task(&a, b);
	copy_task(&b, &tmp);
}

task* create(int ID, int deadline, int release_time, int period, int prio, int quantum, int priority_quantum, task* next)			//Creates a new task
{
    task* new_node = (task*)malloc(sizeof(task));
    if(new_node == NULL)
    {
        printf("Error creating a new node.\n");
        exit(0);
    }
    new_node->ID = ID;
    new_node->deadline = deadline;
    new_node->release_time = release_time;
    new_node->period = period;
    new_node->priority = prio;
    new_node->quantum = quantum;
    new_node->priority_quantum = priority_quantum;
    new_node->next = next;
    return new_node;
}
task * enqueue(task * head, task data)			//Appends a task to a list
{
    /* go to the last node */
    task *cursor = head;
    if (cursor == NULL)
    {
    	head = create(data.ID, data.deadline, data.release_time, data.period, data.priority, data.quantum, data.priority_quantum ,NULL);
    }
    else
    {
    	while(cursor->next != NULL)
    	{
    		cursor = cursor->next;
    	}
    /* create a new node */
    	task* new_node =  create(data.ID, data.deadline, data.release_time, data.period, data.priority, data.quantum, data.priority_quantum ,NULL);
    	cursor->next = new_node;
    }
    return head;
}

prioritytask * enqueue_priority(prioritytask * head, task* task)			//Appends a task to a list
{
    /* go to the last node */
	prioritytask *cursor = head;
    if (cursor == NULL)
    {
    	head = malloc(sizeof(prioritytask));
    	head->next = NULL;
    	head->task = task;
    }
    else
    {
    	while(cursor->next != NULL)
    	{
    		cursor = cursor->next;
    	}
    /* create a new node */
    	prioritytask* new_node = malloc(sizeof(prioritytask));
    	new_node->next = NULL;
    	new_node->task = task;
    	cursor->next = new_node;
    }
    return head;
}


task * pop(task * head)			//Removes the first element of a list
{
    if(head == NULL)
        return NULL;
    task *front = head;
    head = head->next;
    front->next = NULL;
    /* is this the last node in the list */
    if(front == head)
        head = NULL;
    free(front);
    return head;
}

prioritytask* pop_priority(prioritytask* head)			//Removes the first element of a list
{
    if(head == NULL)
        return NULL;
    prioritytask *front = head;
    head = head->next;
    front->next = NULL;
    /* is this the last node in the list */
    if(front == head)
        head = NULL;
    free(front);
    return head;
}

task * remove_back(task* head)			//Removes the back of a list
{
    if(head == NULL)
        return NULL;

    task *cursor = head;
    task *back = NULL;
    while(cursor->next != NULL)
    {
        back = cursor;
        cursor = cursor->next;
    }
    if(back != NULL)
        back->next = NULL;

    /* if this is the last node in the list*/
    if(cursor == head)
        head = NULL;

    free(cursor);

    return head;
}


task * remove_node (task* head,task* nd)
{
    /* if the node is the first node */
    if(nd == head)
    {
        head = pop(head);
        return head;
    }

    /* if the node is the last node */
    if(nd->next == NULL)
    {
        head = remove_back(head);
        return head;
    }

    /* if the node is in the middle */
    task* cursor = head;
    while(cursor != NULL)
    {
        if(cursor->next == nd)
            break;
        cursor = cursor->next;
    }

    if(cursor != NULL)
    {
    	task* tmp = cursor->next;
        cursor->next = tmp->next;
        tmp->next = NULL;
        free(tmp);
    }
    return head;
}
task * first_to_last (task * head)
{
	if (head==NULL)
		return NULL;
	if (head->next == NULL)
		return head;

	task * new_front = head->next;
	task* cursor = new_front;
	while(cursor->next != NULL)
	{
		cursor = cursor->next;
	}
	cursor->next = head;
	head->next=NULL;
	return new_front;

}

void clear(task* head) {
	while (head) {
		head = pop(head);
	}
}

void clear_priority(prioritytask* head) {
	while (head) {
		head = pop_priority(head);
	}
}

//------------------Reads a taskset from file and inserts them to ready queue------------------
void readTaskset_n(char * filepath)
{
	FILE *reads;											//File handle
	//char * sp = "/home/student/Desktop/Labs_material/taskset.txt";								//File path
    reads=fopen(filepath, "r");									//Open file
    task * data_struct = malloc(sizeof(task));				//Allocate data
    if (reads==NULL) {										//If reading fails, return
      	perror("Error");
       	return;
   	}
    else
	{
	    while(!feof(reads))									//Iterate through file
		{
			fscanf(reads,"%d %d %d %d %d %d\n", &data_struct->deadline, &data_struct->period, &data_struct->release_time, &data_struct->priority, &data_struct->ID, &data_struct->quantum);
			data_struct->priority_quantum = 1;
			waiting_queue=enqueue(waiting_queue, *data_struct);	//Read file and add it to the waiting queue
        }
    }
    free(data_struct);										//Free the struct
}

//------------------Wake up task - moves tasks from waiting queue to ready queue------------------
void OS_wakeup_n()
{
	if (waiting_queue == NULL)			//If waiting queue is empty, return
	{
		return;
	}
	task * temp = waiting_queue;		//Handle to head of waiting queue
	while (temp != NULL)				//Iterate through waiting queue
	{
		if (OS_cycles >= temp->release_time)		//If the OS clock is greater or equal to the release time of a task, append it to the ready queue
		{
			ready_queue=enqueue(ready_queue, *temp);				//Enqueue the task
			waiting_queue=remove_node(waiting_queue, temp);		//Remove the task from the waiting queue
			temp = waiting_queue;								//Set our iterator to the new waiting queue
		}
		else
		{
			temp = temp->next;									//Iterate the list...
		}
	}
}

task * scheduler_rr() {
	return ready_queue;
}

task * scheduler_sjf() {
	task* shortest_task = ready_queue;
	task* current = ready_queue;

	while (current) {
		if (current->quantum < shortest_task->quantum) {
			shortest_task = current;
		}
		current = current->next;
	}

	swap(ready_queue, shortest_task);

	return ready_queue;
}

task * scheduler_mq() {
	for (int i = 0; i < 3; i++) {
		clear_priority(priority_queues[i]);
		priority_queues[i] = NULL;
	}

	task* current = ready_queue;
	while (current) {
		if(current->priority_quantum <= 0) {
			current->priority = MIN(current->priority + 1, 3);
			current->priority_quantum = priority_quantum[current->priority - 1];
		}

		priority_queues[current->priority - 1] = enqueue_priority(priority_queues[current->priority - 1], current);
		current = current->next;
	}

	for (int i = 0; i < 3; i++) {
		if (priority_queues[i]) {
			swap(priority_queues[i]->task, ready_queue);
			break;
		}
	}

	return ready_queue;
}

//------------------Scheduler, returns the task to be executed ------------------
task * scheduler_n()
{
	if (ready_queue != NULL)			//If the ready queue isn't empty, we have tasks ready to be returned from the scheduler
	{
		if (sched_type == sched_RR) 		//Here is the round robin (RR) scheduler, in the RR case, we just return the first element of the ready queue
		{
			return scheduler_rr();
		}
		if (sched_type == sched_SJF) 		//Here is where you implement your SJF scheduling algorithm
		{
			return scheduler_sjf();
		}
		if (sched_type == sched_MQ) 		//Here is where you implement your MQ scheduling algorithm,
		{
			return scheduler_mq();
		}
	}
	else						//If the ready queue is empty, the operating system must have something to do, therefore we return an idle task
	{
		idle_task->quantum++;			//Make sure that the idle task dosnt run out of quantum
		printf("RETURNED IDLE TASK, ");
		return idle_task;			//Return the idle task
	}
	return NULL;
}
//------------------ Dispatcher executes the task ------------------
void dispatch_n(task* exec)
{
	if (exec->ID != last_task) //If the pointer of the last executing task is not equal to the newly executing task a context switch has occurred
	{
		context_switches++; //Increase the context switches
		last_task = exec->ID; //Change the pointer of the currently executing task
	}

	exec->quantum--; //Decrease the time quantum of a task, i.e. run the task

	//TODO FIX
	exec->priority_quantum--; //Decrease the priority quantum so we know when to switch priority

	if (exec->quantum > 0)
	{
		if (exec == idle_task)
		{
			// Print
			printf("Idle task is executing - Total context switches: %d \n", context_switches);
		}
		else
		{
			// Print task info
			printf("Task %d with priority %d is executing with %d quanta left - Total context switches: %d \n", exec->ID, exec->priority, exec->quantum, context_switches);
		}

		ready_queue = first_to_last(ready_queue); //Re-sort ready queue
	}
	else
	{
		printf("Task %d with priority %d has executed and finished its quanta - Total context switches: %d \n", exec->ID, exec->priority, context_switches);	//Printout task info
		ready_queue->release_time = ready_queue->release_time+ready_queue->period;
		waiting_queue = enqueue(waiting_queue, *ready_queue); 			//Add the finished task to the waiting queue
		ready_queue = pop(ready_queue); 						//Pop the finished task from ready queue
	}
}

int main(int argc, char **argv)
{
	char * fp = "src/taskset.txt";			//File path to taskset
	readTaskset_n(fp); 						//Read taskset
	task * task_to_be_run; 					//Return taskset from scheduler
	idle_task = create(1337, 0, 0, 0, 0, 200000000, 0, NULL);  //Create a dummy idle task
	while (1)
	{
		OS_wakeup_n();						//Wake up sleeping tasks
		task_to_be_run = scheduler_n();		//Fetch the task to be run
		dispatch_n(task_to_be_run);			//Dispatch the task to be run
		OS_cycles++;						//Increment OS clock
		usleep(1000000);					//Sleep so we dont get overflown with output
	}
}

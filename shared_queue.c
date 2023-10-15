#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#define MAX_QUEUE_SIZE 30
#define MAX_ITEM_SIZE 128

typedef struct Process {
    char executable[MAX_ITEM_SIZE];
    pid_t pid;
    int priority;
} Process;

typedef struct PriorityQueue {
    Process processes[MAX_QUEUE_SIZE];
    int rear;
    int num_process;
    sem_t mutex;
    sem_t empty;
    sem_t full;
} PriorityQueue;

int create_priority_queue(key_t key, PriorityQueue **queue);
void enqueue(PriorityQueue* queue, const Process* process);
int dequeue(PriorityQueue* queue, Process* process);
void destroy_priority_queue(PriorityQueue *queue);
int get_rear(PriorityQueue* queue);
int get_num_process(PriorityQueue* queue);

int is_empty(PriorityQueue *queue) {
    sem_wait(&queue->mutex);
    int empty = (queue->rear == -1);
    sem_post(&queue->mutex);

    return empty;
}

int is_full(PriorityQueue *queue) {
    sem_wait(&queue->mutex);
    int full = (queue->rear == MAX_QUEUE_SIZE - 1);
    sem_post(&queue->mutex);

    return full;
}

int create_priority_queue(key_t key, PriorityQueue **queue) {
    int shmid = shmget(key, sizeof(PriorityQueue), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return -1;
    }

    *queue = (PriorityQueue *)shmat(shmid, NULL, 0);
    if (*queue == (PriorityQueue *)(-1)) {
        perror("shmat");
        return -1;
    }

    sem_init(&(*queue)->mutex, 1, 1);
    sem_init(&(*queue)->empty, 1, MAX_QUEUE_SIZE);
    sem_init(&(*queue)->full, 1, 0);

    (*queue)->num_process = 0;
    (*queue)->rear = -1;
    return 0;
}

void enqueue(PriorityQueue* queue, const Process* process) {
    sem_wait(&queue->empty);
    sem_wait(&queue->mutex);

    queue->rear++;
    queue->processes[queue->rear] = *process;
    queue->num_process++;
    
    sem_post(&queue->mutex);
    sem_post(&queue->full);
}

int dequeue(PriorityQueue* queue, Process* process) {
    if (queue->rear == -1) {
        return 0;  // Queue is empty
    }
    
    sem_wait(&queue->full);
    sem_wait(&queue->mutex);

    *process = queue->processes[0];
    for (int i = 0; i < queue->rear; i++) {
        queue->processes[i] = queue->processes[i + 1];
    }
    queue->num_process--;
    queue->rear--;

    sem_post(&queue->mutex);
    sem_post(&queue->empty);

    return 1;  // Successfully dequeued
}

void destroy_priority_queue(PriorityQueue *queue) {
    shmdt(queue);
}

int get_rear(PriorityQueue *queue) {
    sem_wait(&queue->mutex);
    int ret = queue->rear;
    sem_post(&queue->mutex);

    return ret;
}

int get_num_process(PriorityQueue* queue){
    sem_wait(&queue->mutex);
    int ret = queue->num_process;
    sem_post(&queue->mutex);

    return ret;
}
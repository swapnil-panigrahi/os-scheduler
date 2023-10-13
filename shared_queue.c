#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <errno.h>

#define MAX_QUEUE_SIZE 30
#define MAX_ITEM_SIZE 256

struct SharedQueue {
    char items[MAX_QUEUE_SIZE][MAX_ITEM_SIZE];
    pid_t pid[MAX_QUEUE_SIZE];
    int front, rear;
    sem_t mutex;
    sem_t full;
    sem_t empty;
};

int create_shared_queue(key_t key, struct SharedQueue **queue);
void enqueue(struct SharedQueue *queue, const char *item);
void dequeue(struct SharedQueue *queue, char *item);
void destroy_shared_queue(struct SharedQueue *queue);

int create_shared_queue(key_t key, struct SharedQueue **queue) {
    int shmid = shmget(key, sizeof(struct SharedQueue), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return -1;
    }

    *queue = (struct SharedQueue *)shmat(shmid, NULL, 0);
    if (*queue == (struct SharedQueue *)(-1)) {
        perror("shmat");
        return -1;
    }

    sem_init(&(*queue)->mutex, 1, 1);
    sem_init(&(*queue)->full, 1, 0);
    sem_init(&(*queue)->empty, 1, MAX_QUEUE_SIZE);

    (*queue)->front = (*queue)->rear = -1;
    return 0;
}

void enqueue(struct SharedQueue *queue, const char *item, pid_t p) {
    sem_wait(&queue->empty);
    sem_wait(&queue->mutex);

    queue->rear = (queue->rear + 1) % MAX_QUEUE_SIZE;
    strcpy(queue->items[queue->rear], item);
    queue->pid[queue->rear] = p;

    sem_post(&queue->mutex);
    sem_post(&queue->full);
}

void dequeue(struct SharedQueue *queue, char *item, pid_t* p) {
    sem_wait(&queue->full);
    sem_wait(&queue->mutex);

    queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
    strcpy(item, queue->items[queue->front]);
    *p = queue->pid[queue->front];

    sem_post(&queue->mutex);
    sem_post(&queue->empty);
}

void destroy_shared_queue(struct SharedQueue *queue) {
    shmdt(queue);
}
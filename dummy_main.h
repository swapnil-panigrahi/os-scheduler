#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define execv sleep

typedef struct Process{
    pid_t pid;
    int burst_time;
} Process;

int NCPU;
int TSLICE;

typedef struct {
    Process process_queue[10];
    int front;
    int back;
} ProcessQ;

void initQueue(ProcessQ *queue) {
    queue->front = -1;
    queue->back = -1;
}

bool isEmpty(ProcessQ *queue) {
    return queue->front == -1;
}

void enqueue(ProcessQ *queue, pid_t pid, int burst_time) {
    if (isEmpty(queue)) {
        queue->front = 0;
    }

    queue->back = (queue->back + 1) % 10;
    queue->items[queue->back].pid = pid;
    queue->items[queue->back].burst_time = burst_time;
}

Process dequeue(ProcessQ *queue) {
    Process empty_process = { -1, -1 };

    if (isEmpty(queue)) {
        fprintf(stderr, "Queue is empty\n");
        return empty_process;
    }

    Process topProcess = queue->items[queue->front];

    if (queue->front == queue->back) {
        queue->front = -1;
        queue->back = -1;
    } else {
        queue->front = (queue->front + 1) % 10;
    }

    return topProcess;
}

void scheduler(int num_processes, int burst_time) {
    ProcessQ queue;
    initQueue(&queue);

    for (int i = 1; i <= num_processes; i++) {
        enqueue(&queue, i, burst_time);
    }

    while (!isQueueEmpty(&queue)) {
        int cpu_count = 0;

        while (cpu_count < NCPU && !isQueueEmpty(&queue)) {
            Process process = dequeue(&queue);
            execv(burst_time);

            process.remaining_time -= burst_time;
            if (process.remaining_time > 0) {
                enqueue(&queue, process.pid, process.remaining_time);
            }

            cpu_count++;
        }
    }
}

int dummy_main(int argc, char** argv);
int main(int argc, char** argv){

    printf("Enter the number of CPUs: ");
    scanf("%d",&NCPU);
    printf("\n");

    printf("Enter the time slice in milliseconds: ");
    scanf("%d",&TSLICE);
    printf("\n");

    //Fork and add exec to shell here
    
    int ret = dummy_main(argc,argv);
    if (ret==0){
        scheduler(NCPU, TSLICE);
    }
}
#define main dummy_main
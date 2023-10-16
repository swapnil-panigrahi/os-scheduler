#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>
#include "shared_queue.c"

int NCPU;
int TSLICE;
PriorityQueue* queue;
volatile sig_atomic_t terminate = 0;

void termination_request(int);
void scheduler(PriorityQueue*);

pid_t pids[256];
char* prcoess_name[256];
int globalProcess = 0;

void termination_request(int signum) {
    (void) signum;
    terminate = 1;

    // Loop through all processes and wait for them to finish
    for (int i = 0; i < globalProcess; i++) {
        if (kill(pids[i], 0) == 0){
            kill(pids[i],SIGCONT);
        }
        int status;
        waitpid(pids[i], &status, 0);
    }
    printf("Scheduler history: \n");
    for (int i = 0; i < globalProcess; i++){
        printf("%d: %s\n", pids[i], prcoess_name[i]);
    }

    destroy_priority_queue(queue);
    printf("Scheduler exiting...\n");
    exit(0);
}


int min(int x, int y){
    if (x<y) return x;
    return y;
}

void scheduler(PriorityQueue* queue) {
    Process P[NCPU];
    int num = get_num_process(queue);
    int cycle = min(NCPU, num);

    while (cycle > 0){
        cycle = min(NCPU, num);

        for (int i=0; i<cycle; i++){
            dequeue(queue, &P[i]);
            
            if (P[i].pid == -2){
                P[i].pid = fork();
                if (P[i].pid == -1){
                    perror("fork");
                    return;
                }
                char* args[2];
                args[0] = P[i].executable;
                args[1] = NULL;
        
                if (P[i].pid == 0){
                    execv(args[0], args);
                    perror("execv");
                    exit(EXIT_FAILURE);
                }
                else{
                    pids[globalProcess] = P[i].pid;
                    prcoess_name[globalProcess] = P[i].executable;
                    globalProcess++;
                }
            }
            if (kill(P[i].pid, SIGCONT) == -1) {
                perror("SIGCONT");
                exit(EXIT_FAILURE);
            }
        }
        usleep(TSLICE*1000);
        for (int i=0; i<cycle; i++){
            if (kill(P[i].pid,0) == 0){
                enqueue(queue, &P[i]);
                
                if (kill(P[i].pid, SIGTSTP) == -1) {
                    perror("SIGTSTP");
                    exit(EXIT_FAILURE);
                }
            }
        }
        num -= cycle;
    }
}

int main(int argc, char** argv){
    int shmid = atoi(argv[1]);
    pid_t shell_pid = atoi(argv[2]);
    NCPU = atoi(argv[3]);
    TSLICE = atoi(argv[4]);

    pid_t pid = getpid();
    printf("PID: %ld\n", pid);
    printf("NCPU: %d\nTSLICE: %d\nSHMID: %d\n", NCPU, TSLICE, shmid);
    
    queue = shmat(shmid, NULL, 0);
    if (queue == (PriorityQueue*)(-1)) {
        perror("shmat");
        return 1;
    }
    if (signal(SIGINT, termination_request) == SIG_ERR) {
        perror("Unable to set termination signal handler");
        exit(EXIT_FAILURE);
    }
    
    while (!terminate){
        scheduler(queue);
    }
    return 0;
}
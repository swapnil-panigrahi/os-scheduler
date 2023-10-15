# os-scheduler
## Contributions
Rohak Kansal - creation and implementation of the shared memory datastructure for the scheduler and simple-shell.
Swapnil Panighari - implementation of the round robin method of scheduling in the simple scheduler and the submit cmd in the simple-shell.
## Implementation
### Queue
- using three arrays of 30 elements (max number of processes that can be queued at a time).
- one for the name of the processes.
- second to store the pid of the processes.
- third to store their priority.
- using three semaphores to ensure there isno race condition.
- one for checking weather the queue is being currently acssesed by a process or not.
- the others to see if the queue is full or empty.
### Simple-scheduler
- first it is started using the exec function in the simple-shell.
- it acsseses the shared memory every time slice to enqueue unfinished processes from the last time slice and dequeue processes to be done.
- after dequeueing it runs the processes and calles sleep for the rest of the time slice.
- ater the time slice is over it begins the process again.
- when the shell is teminated using ctrl+c the shell passes the cmd to the scheduler that no new processes will be added at which point the scheduler continues the process untill the queue becomes empty after which it terminates it self.
## Functionality
- `submit` cmd has been added to simple-shell which allows the user to specify a program to be rrun using the custom scheduler with a priority value.
- simple-shell now has a custom scheduer which runs in the background.
- it prints the pid of the child processes in which the programs using the submit cmd were given.
- the custom scheduler uses a round-robin approach to schedule the processes with a specific priority value first.
## [GitHub Repository](https://github.com/swapnil-panigrahi/os-scheduler)

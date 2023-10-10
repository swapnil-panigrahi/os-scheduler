#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

typedef struct History{
    char command[1000000];
    pid_t child_pid;
    time_t time;
    double exec_time;
} History;

History history[100];
volatile sig_atomic_t ctrl_c = 0;
int hist_indx = 0;
int background_process = 0;
int bg_process_indx = 0;

enum {
    INVALID_CMD = 0,
    DOT_SLASH_CMD,
    CAT_CMD,
    MKDIR_CMD,
    RMDIR_CMD,
    RM_CMD,
    MV_CMD,
    CP_CMD,
    PWD_CMD,
    UNIQ_CMD,
    SORT_CMD,
    GREP_CMD,
    WC_CMD,
    ECHO_CMD,
    LS_CMD,
    HISTORY_CMD,
    PIPE_CMD,
    SCRIPT_CMD
};  //Commands enum

void sigint_handler(int signum);
int is_valid_cmd(char* input);
char** return_args(char* input);
void execvp_cmd(char* input);
void echo(char* input);
void hist();
void dot_slash(char* input);
void pipe_cmd(char* input);
void script(char* input);

int main() {
    char input[1000000];
    time_t input_time;
    clock_t start,end;

    signal(SIGINT, sigint_handler); //Registering the ctrl+c signal
    while(!ctrl_c){
        while (1) {
            pid_t bg_pid = waitpid(-1, NULL, WNOHANG);
            if (bg_pid <= 0) {
                break;
            }
            printf("[%d] done\n", bg_pid);
            history[bg_process_indx].child_pid = bg_pid;
            
            background_process = 0;
        }   //Background process check

        int cmd = 0;
        while (!cmd){
            printf("\033[1;35msimple-shell> \033[0m");
            if (fgets(input, sizeof(input), stdin) == NULL) {
                printf("\n");
                break;
            }   
            //Taking command as input
            
            time(&input_time);
            //Noting time of the input 

            input[strcspn(input, "\n")] = '\0';
            // Check if the input is empty
            
            if (input[strlen(input) - 1] == '&') {
                strcpy(history[hist_indx].command, input);
                bg_process_indx = hist_indx;
                background_process = 1;

                input[strlen(input) - 1] = '\0';
            }
            else{
                background_process = 0;
            }
            
            if (strlen(input) == 0) {
                continue;
            }
            // Prompt for input again
            if(strchr(input, '|') == NULL) cmd = is_valid_cmd(input); //Pipeline check
            else cmd = 16;
            
            if(cmd == 0) printf("Invalid command. Please try again.\n"); //Invalid Command
        }

        start = clock();
        switch(cmd){
            case SCRIPT_CMD:
                script(input);
                break;
            case PIPE_CMD:
                strcpy(history[hist_indx].command,input);
                pipe_cmd(input);
                break;
            case HISTORY_CMD:
                hist();
                break;
            case LS_CMD:
                execvp_cmd(input);
                break;
            case ECHO_CMD:
                echo(input);
                break;
            case WC_CMD:
                execvp_cmd(input);
                break;
            case MKDIR_CMD:
                execvp_cmd(input);
                break;
            case RMDIR_CMD:
                execvp_cmd(input);
                break;
            case RM_CMD:
                execvp_cmd(input);
                break;
            case MV_CMD:
                execvp_cmd(input);
                break;
            case CP_CMD:
                execvp_cmd(input);
                break;
            case PWD_CMD:
                execvp_cmd(input);
                break;
            case UNIQ_CMD:
                execvp_cmd(input);
                break;
            case SORT_CMD:
                execvp_cmd(input);
                break;
            case GREP_CMD:
                execvp_cmd(input);
                break;
            case CAT_CMD:
                execvp_cmd(input);
                break;
            case DOT_SLASH_CMD:
                dot_slash(input);
                break;
        }
        end = clock();
        
        history[hist_indx].exec_time = ((double) (end - start))/CLOCKS_PER_SEC;
        history[hist_indx].time = input_time;
        if (cmd != PIPE_CMD && !background_process) strcpy(history[hist_indx].command,input);

        //Appending to history array
        hist_indx++;
    }
    return 0;
}

void sigint_handler(int signum) {
    (void)signum;  //Suppress unused variable warning
    ctrl_c = 1;
    printf("\n\n");
    for (int i=0; i<hist_indx; i++){    //Printing the command, PID, input time, exec time
        printf("\033[1;32mCommand:\033[0m %s\n", history[i].command);
        printf("\033[1;32mChild PID:\033[0m %d\n", history[i].child_pid);
        printf("\033[1;32mTime:\033[0m %s", ctime(&history[i].time));
        printf("\033[1;32mExecution Time:\033[0m %lf\n\n", history[i].exec_time);
    }
    exit(0);  // Terminate the program immediately
}

int is_valid_cmd(char* input){
    const char* valid_cmds[] = {
        "",
        "./",
        "cat",
        "mkdir",
        "rmdir",
        "rm",
        "mv",
        "cp",
        "pwd",
        "uniq",
        "sort",
        "grep",
        "wc",
        "echo",
        "ls",
        "history",
    };  //Valid Commands Array

    //Tokenize the input to get the command header
    char* input_copy = strdup(input);
    char* cmd_header = strtok(input_copy, " ");
    //Iterate through the valid command strings and check for a match
    for (int i = 1; i <= HISTORY_CMD; i++) {
        if (strcmp(cmd_header, valid_cmds[i]) == 0) {
            free(input_copy); //Freeing
            return i;
        }
    }
    if ((strncmp(input, "./", 2) == 0)) return 1;
    if ((strncmp(input, "sh ", 3) == 0)) return 17;
    
    //Returning 0 for invalid command
    free(input_copy);
    return INVALID_CMD;
}

char** return_args(char* input) {
    char** args = NULL;
    int count = 0;
    char* input_copy = strdup(input);

    if (input_copy == NULL) {
        perror("strdup");
        return NULL;
    }

    char* token = strtok(input_copy, " ");
    
    while (token != NULL) {
        args = realloc(args, (count + 2) * sizeof(char*));
        if (args == NULL) {
            perror("reallocate");
            free(input_copy);
            return NULL;
        }

        args[count] = strdup(token);

        if (args[count] == NULL) {
            perror("strdup");
            free(input_copy);
            for (int i = 0; i < count; i++) {
                free(args[i]);
            }
            free(args);
            return NULL;
        }

        count++;
        args[count] = NULL;
        token = strtok(NULL, " ");
    }

    free(input_copy);
    return args;
}

void execvp_cmd(char* input) {
    pid_t child_pid;
    child_pid = fork();
    if(child_pid == -1){
        perror("fork");
        return;
    }
    if(child_pid == 0){
        char** args = return_args(input);
        
        execvp(args[0],args);
        perror("execvp");
        exit(EXIT_FAILURE);

    }
    else{
        if (!background_process){
            int status;
            if(waitpid(child_pid,&status, 0) == -1){
                perror("Waitpid");
                return;
            }
            if (WIFEXITED(status)) {
                history[hist_indx].child_pid = child_pid;
                //printf("Child process exited with status %d\n", WEXITSTATUS(status));
            }
        }
    }
}

void echo(char* input) {
    char* input_copy = strdup(input);
    if (input_copy == NULL) {
        perror("strdup");
    }

    char* command = strtok(input_copy, " \t\n");
    char* argument = strtok(NULL, "\n");

    char* args[] = {command, argument, NULL};
    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        return;
    }

    if (child_pid == 0) {
        execvp(command, args);
        //Execvp error
        perror("execvp");
        exit(EXIT_FAILURE);
    } 
    else {
        if (!background_process){
            int status;
            if (waitpid(child_pid, &status, 0) == -1) {
                perror("waitpid");
                return;
            }
            if (WIFEXITED(status)) {
                history[hist_indx].child_pid = child_pid;
                // printf("Child process exited with status %d\n", WEXITSTATUS(status));
            }
        }
    }
}

void hist(){
    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        return;
    }

    if (child_pid == 0) {
        for (int i=0; i<hist_indx; i++){
            printf("%s\n", history[i].command);
        }
        exit(0);
    } 
    else {
        if (!background_process){
            int status;

            waitpid(child_pid, &status, 0);
            history[hist_indx].child_pid = child_pid;
            // printf("Child PID: %d\n", child_pid);
        }
    }
}

void dot_slash(char* input){
    pid_t child_pid;
    child_pid = fork();
    if(child_pid == -1){
        perror("fork");
        return;
    }
    if (child_pid == 0){
        execl(input, input, NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    }
    else{
        if (!background_process){
            int status;
            if(waitpid(child_pid, &status, 0) == -1){
                perror("waitpid");
                return;
            }
            if (WIFEXITED(status)) {
                history[hist_indx].child_pid = child_pid;
                //printf("Child process exited with status %d\n", WEXITSTATUS(status));
            }
        }
    }
    printf("\n");
}

void pipe_cmd(char* input) {
    char* commands[20];
    int num_cmds = 0;

    char* token = strtok(input, "|");
    while (token != NULL && num_cmds < 20) {
        commands[num_cmds++] = token;
        token = strtok(NULL, "|");
    }

    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0) {
        int pipe_fd_previous[2] = {-1, -1};

        for (int i = 0; i < num_cmds; i++) {
            int pipe_fd_next[2];

            if (pipe(pipe_fd_next) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            pid_t cmd_child_pid = fork();
            if (cmd_child_pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (cmd_child_pid == 0) {
                if (pipe_fd_previous[0] != -1) {
                    dup2(pipe_fd_previous[0], STDIN_FILENO);
                    close(pipe_fd_previous[0]);
                    close(pipe_fd_previous[1]);
                }

                if (i < num_cmds - 1) {
                    dup2(pipe_fd_next[1], STDOUT_FILENO);
                    close(pipe_fd_next[0]);
                    close(pipe_fd_next[1]);
                }

                execlp("sh", "sh", "-c", commands[i], NULL);
                perror("execlp");
                exit(EXIT_FAILURE);
            } else {
                if (pipe_fd_previous[0] != -1) {
                    close(pipe_fd_previous[0]);
                    close(pipe_fd_previous[1]);
                }

                pipe_fd_previous[0] = pipe_fd_next[0];
                pipe_fd_previous[1] = pipe_fd_next[1];
            }
        }

        if (pipe_fd_previous[0] != -1) {
            close(pipe_fd_previous[0]);
            close(pipe_fd_previous[1]);
        }

        for (int i = 0; i < num_cmds; i++) {
            wait(NULL);
        }

        exit(EXIT_SUCCESS);
    } 
    else {
        if (!background_process){
            int status;
            if (waitpid(child_pid, &status, 0) == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }

            if (WIFEXITED(status)) {
                history[hist_indx].child_pid = child_pid;
                //printf("Child process exited with status %d\n", WEXITSTATUS(status));
            }
        }
    }
}

void script(char* input) {
    pid_t child_pid = fork();

    if (child_pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0){
        execl("/bin/sh", "sh", input+3, NULL);

        perror("execl");
        exit(EXIT_FAILURE);
    } 
    else{
        if (!background_process){
            int status;
            if (waitpid(child_pid, &status, 0) == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }

            if (WIFEXITED(status)) {
                history[hist_indx].child_pid = child_pid;
                //printf("Shell script exited with status %d\n", WEXITSTATUS(status));
            }
        }
    }
}
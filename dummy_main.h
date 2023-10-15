#include <signal.h>
#include <unistd.h>

int dummy_main(int argc, char** argv);
void sigint_handler(int signum);
void sigterm_handler(int signum);

void sigint_handler(int signum){}
void sigterm_handler(int signum){}

int main(int argc, char** argv) {
    kill(getpid(), SIGTSTP);

    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigterm_handler);

    int ret = dummy_main(argc, argv);
    return ret;
}
#define main dummy_main
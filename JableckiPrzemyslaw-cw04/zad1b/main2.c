#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


#define TRUE 1
#define FALSE 0
int switch_ = 0;
int hasAliveChild = FALSE;
pid_t pid = 0;
void sigtstp_handle(int signum) {
    if (switch_ == 1)
        switch_ = 0;
    else {
        printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu\n");
        switch_ = 1;
    }
}

void sigint(int signum) {
    printf("Odebrano sygnał SIGINT\n");
    exit(0);
}

int main() {


    struct sigaction act;
    act.sa_handler = sigint;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
    signal(SIGTSTP, &sigtstp_handle);
    printf("child pid: %d\n", pid);
    printf("proces pid: %d\n", getpid());

    while (1) {
        if (switch_ == 0) {
            if (!hasAliveChild) {
                hasAliveChild = TRUE;
                pid = fork();
                printf("child pid: %d\n", pid);
                printf("proces pid: %d\n", getpid());
                if (pid == 0) {
                    execl("./date.sh", "./date.sh", NULL);
                    exit(EXIT_SUCCESS);
                }
            }
        } else {
            if(hasAliveChild){
                printf("child pid: %d\n", pid);
                printf("proces pid: %d\n", getpid());
                kill(pid, SIGKILL);
                hasAliveChild = FALSE;
            }
        }
    }
}
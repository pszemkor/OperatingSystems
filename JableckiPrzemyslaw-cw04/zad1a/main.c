#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int switch_ = 0;

void sigtstp_handle(int signum) {
    if (switch_ == 0){
        switch_ = 1;
        printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu\n");
        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask, SIGTSTP);
        sigdelset(&mask, SIGINT);;
        sigsuspend(&mask);
    }else{
        switch_ = 0;
    }


}
void sigint(int signum){
    printf("\rOdebrano sygnał SIGINT\n");
    exit(0);
}

int main() {
    signal(SIGTSTP, &sigtstp_handle);

    struct sigaction act;
    act.sa_handler = sigint;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    while(1) {
        system("date");
        sleep(1);
    }

    return 0;
}
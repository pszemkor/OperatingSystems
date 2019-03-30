//
// Created by przjab98 on 30.03.19.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

static int received_signals = 0;
char* global_mode;

void handler(int sig, siginfo_t *info, void *ucontext) {
    if(sig == SIGUSR1){
        received_signals++;
    }else if (sig == SIGUSR2){
        if(!strcmp(global_mode, "KILL")){
            for(int i = 0; i < received_signals; i++)
                kill(info->si_pid, SIGUSR1);
            kill(info->si_pid, SIGUSR2);
        }else if(!strcmp(global_mode,"SIGQUEUE")){
                        union sigval tmp;
            for(int i = 0; i < received_signals; i++)
                sigqueue(info->si_pid, SIGUSR1, tmp);
            sigqueue(info->si_pid, SIGUSR2, tmp);
        }else{
            //TO DO
        }

        printf("catcher received : %d signals \n", received_signals);
        exit(0);
    }
}
int main(int argc, char *argv[]){

    printf("my pid: %d", getpid() );
    if(argc != 2){
        fprintf(stderr,"too few args \n");
        exit(1);
    }
    char* mode = argv[1];
    if(strcmp(mode, "KILL")!=0 && strcmp(mode,"SIGQUEUE")!=0 && strcmp(mode,"SIGRT")!=0){
        fprintf(stderr,"unknown mode \n");
        exit(1);
    }
    global_mode = mode;

    //program is supposed to block every other signals than sigusr1 and sigusr2
    sigset_t oldmask, blockmask;
    sigemptyset(&blockmask);
    sigfillset(&blockmask);
    sigemptyset(&oldmask);
    if(!strcmp("KILL",mode) || !strcmp("SIGQUEUE", mode)){
        sigdelset(&blockmask, SIGUSR1);
        sigdelset(&blockmask, SIGUSR2);
    }else{
        //TO DO
    }
    if (sigprocmask(SIG_BLOCK, &blockmask, &oldmask) == -1) {
        fprintf(stderr, "cannot set mask \n");
        exit(1);
    }
    struct sigaction act;

    /* When the SA_SIGINFO flag is specified in act.sa_flags, the signal
     handler address is passed via the act.sa_sigaction field */
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = handler;
    sigemptyset(&act.sa_mask);
    if (!strcmp(mode, "KILL") || !strcmp(mode, "SIGQUEUE")) {
        sigaddset(&act.sa_mask, SIGUSR1);
        sigaddset(&act.sa_mask, SIGUSR2);
        sigaction(SIGUSR1, &act, NULL);
        sigaction(SIGUSR2, &act, NULL);
    } else {
        //TO DO -> SIGRT MODE
    }
    printf("my pid: %d \n", getpid() );
    while(1);
    return 0;
}
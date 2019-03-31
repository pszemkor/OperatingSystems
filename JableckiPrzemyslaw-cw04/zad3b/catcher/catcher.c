//
// Created by przjab98 on 30.03.19.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

static int received_signals = 0;
char *global_mode;

void handler(int sig, siginfo_t *info, void *ucontext)
{
    if (sig == SIGUSR1 || sig == SIGRTMIN)
    {
        received_signals++;
        if (!strcmp(global_mode, "KILL"))
        {
            kill(info->si_pid, SIGUSR1);
        }
        else if (!strcmp(global_mode, "SIGQUEUE"))
        {
            union sigval tmp;
            sigqueue(info->si_pid, SIGUSR1, tmp);
        }
        else
        {
            kill(info->si_pid, SIGRTMIN);
        }
    }
    else if (sig == SIGUSR2 || sig == SIGRTMIN + 1)
    {
        if (!strcmp(global_mode, "KILL"))
        {
            ;
            kill(info->si_pid, SIGUSR2);
        }
        else if (!strcmp(global_mode, "SIGQUEUE"))
        {
            union sigval tmp;
            sigqueue(info->si_pid, SIGUSR2, tmp);
        }
        else
        {
            kill(info->si_pid, SIGRTMIN + 1);
        }

        printf("catcher received : %d signals \n", received_signals);
        exit(0);
    }
}
int main(int argc, char *argv[])
{

    printf("my pid: %d", getpid());
    if (argc != 2)
    {
        fprintf(stderr, "too few args \n");
        exit(1);
    }
    char *mode = argv[1];
    if (strcmp(mode, "KILL") != 0 && strcmp(mode, "SIGQUEUE") != 0 && strcmp(mode, "SIGRT") != 0)
    {
        fprintf(stderr, "unknown mode \n");
        exit(1);
    }
    global_mode = mode;

    //program is supposed to block every other signals than sigusr1 and sigusr2
    sigset_t oldmask, blockmask;
    sigemptyset(&blockmask);
    sigfillset(&blockmask);
    sigemptyset(&oldmask);
    if (!strcmp("KILL", mode) || !strcmp("SIGQUEUE", mode))
    {
        sigdelset(&blockmask, SIGUSR1);
        sigdelset(&blockmask, SIGUSR2);
    }
    else
    {
        sigdelset(&blockmask, SIGRTMIN);
        sigdelset(&blockmask, SIGRTMIN + 1);
    }
    if (sigprocmask(SIG_BLOCK, &blockmask, &oldmask) == -1)
    {
        fprintf(stderr, "cannot set mask \n");
        exit(1);
    }
    struct sigaction act;

    /* When the SA_SIGINFO flag is specified in act.sa_flags, the signal
     handler address is passed via the act.sa_sigaction field */
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = handler;
    sigemptyset(&act.sa_mask);
    if (!strcmp(mode, "KILL") || !strcmp(mode, "SIGQUEUE"))
    {
        sigaddset(&act.sa_mask, SIGUSR1);
        sigaddset(&act.sa_mask, SIGUSR2);
        sigaction(SIGUSR1, &act, NULL);
        sigaction(SIGUSR2, &act, NULL);
    }
    else
    {
        sigaddset(&act.sa_mask, SIGRTMIN);
        sigaddset(&act.sa_mask, SIGRTMIN + 1);
        sigaction(SIGRTMIN, &act, NULL);
        sigaction(SIGRTMIN + 1, &act, NULL);
    }
    printf("my pid: %d \n", getpid());
    while (1)
        ;
    return 0;
}
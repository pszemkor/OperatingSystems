#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

int received_signals = 0;
int sent_signals;

int convert_to_num(char *given_string)
{
    if (!given_string)
    {
        return -1;
    }
    char *tmp;
    int result = (int)strtol(given_string, &tmp, 10);
    if (strcmp(tmp, given_string) != 0)
    {
        return result;
    }
    else
    {
        return -1;
    }
}

void handler(int sig, siginfo_t *info, void *ucontext)
{
    //TO DO ->  REALTIME SIGNALS
    if (sig == SIGUSR1 || sig == SIGRTMIN)
    {
        received_signals++;
    }
    else if (sig == SIGUSR2 || sig == SIGRTMIN + 1)
    {
        printf("%d signals were sent and: %d were received \n", sent_signals, received_signals);
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "too few args \n");
        exit(1);
    }

    int catcher_pid = convert_to_num(argv[1]);
    int signals_to_send = convert_to_num(argv[2]);
    sent_signals = signals_to_send;
    char *mode = argv[3];
    if (catcher_pid < 0 || signals_to_send < 0)
    {
        fprintf(stderr, "wrong type of argument \n");
        exit(1);
    }
    if (strcmp(mode, "KILL") != 0 && strcmp(mode, "SIGQUEUE") != 0 && strcmp(mode, "SIGRT") != 0)
    {
        fprintf(stderr, "unknown mode \n");
        exit(1);
    }

    //program is supposed to block every signals except sigusr1 and sigusr2
    sigset_t oldmask, blockmask;
    sigemptyset(&oldmask);
    sigemptyset(&blockmask);
    sigfillset(&blockmask);

    if (!strcmp(mode, "KILL") || !strcmp(mode, "SIGQUEUE"))
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

    //sending signals to catcher:
    if (!strcmp(mode, "KILL"))
    {
        int i;
        for (i = 0; i < signals_to_send; i++)
        {
            kill(catcher_pid, SIGUSR1);
        }
        kill(catcher_pid, SIGUSR2);
    }
    else if (!strcmp(mode, "SIGQUEUE"))
    {
        union sigval tmp;
        for (int i = 0; i < signals_to_send; i++)
        {
            sigqueue(catcher_pid, SIGUSR1, tmp);
        }
        sigqueue(catcher_pid, SIGUSR2, tmp);
    }
    else
    {
        int i;
        for (i = 0; i < signals_to_send; i++)
        {
            kill(catcher_pid, SIGRTMIN);
        }
        kill(catcher_pid, SIGRTMIN + 1);
    }

    struct sigaction act;

    /* When the SA_SIGINFO flag is specified in act.sa_flags, the signal
     handler address is passed via the act.sa_sigaction field */
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = handler;
    sigemptyset(&act.sa_mask);

    //receiving signals from catcher:
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

    //this loop won't let program end
    while (1)
    {
        ;
    }

    return 0;
}
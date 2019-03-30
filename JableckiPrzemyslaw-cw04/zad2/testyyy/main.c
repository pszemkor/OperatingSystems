// #define _XOPEN_SOURCE 700
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <libgen.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <signal.h>
static int enable = 1;

void handle_stop(int signum)
{
    enable = 0;
}
void handle_start(int signum)
{
    enable = 1;
}

int main()
{

    pid_t *pids = malloc(1);

    int cpid = fork();
    if (!cpid)
    {
        signal(SIGUSR1, &handle_stop);
        signal(SIGUSR2, &handle_start);
        while (1)
        {
            while (enable)
            {
                printf("enabled: %d \n", getppid());
                sleep(2);
            }
            printf("nope\n");
            sleep(2);
        }
                    printf("nosdadasdasdsadpe\n");
    }
    else
    {
        pids[0] = cpid;
    }

    char buffer[255];
    while (1)
    {
        fgets(buffer,255,stdin);
        if (!strncmp(buffer, "STOP", 4))
        {
            int pid = pids[0];
            kill(pid, SIGUSR1);
        }
        else if (!strncmp(buffer, "START", 4))
        {
            int pid = pids[0];
            kill(pid, SIGUSR2);
        }
        else if (!strncmp(buffer, "END", 3))
        {
            kill(pids[0].pid, SIGTERM);
            break;
        }
        else
        {
            printf("Unknown command\n");
        }
    }
}
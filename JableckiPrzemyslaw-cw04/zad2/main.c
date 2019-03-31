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
#include "utills.h"

static int switch_ = 1;
pid_state_t *pids_global;
int global_size;

void handler_stop(int signum)
{
    switch_ = 0;
    printf("otrzymalem sygnal: %d  \n", signum);
}
void handler_start(int signum)
{
    switch_ = 1;
    printf("otrzymalem sygnal: %d \n", signum);
}

void sigint_handler(int signum)
{
    int i;
    for (i = 0; i < global_size; i++)
    {
        kill(pids_global[i].pid, SIGINT);
    }
    waitpid(pids_global[i].pid, NULL, 0);
    struct rusage usage;
    getrusage(RUSAGE_CHILDREN, &usage);
    printf("system time: %2ld[s]\n", usage.ru_stime.tv_sec);
    printf("system time: %2ld[us]\n", usage.ru_stime.tv_usec);
    printf("user time: %2ld[s]\n", usage.ru_utime.tv_sec);
    printf("user time: %2ld [s]\n", usage.ru_utime.tv_usec);

    exit(0);
}

int make_backups(size_t freq, struct stat st, time_t last_modification,
                 char *file_from_list_path)
{
    signal(SIGUSR1, &handler_stop);
    signal(SIGUSR2, &handler_start);

    last_modification = st.st_mtime;
    int changed_files = 0;
    char *file_content_array = getFile(file_from_list_path);
    if (!file_content_array)
    {
        exit(changed_files);
    }
    size_t size = (size_t)get_file_size(file_from_list_path);

    while (1)
    {
        sleep(1);
        while (switch_)
        {
            printf("dzialam, a to moje pid: %d i mojego rodzica %d \n", getpid(), getppid());

            if (stat(file_from_list_path, &st) == -1)
            {
                fprintf(stderr, "stat problem :(\n");
                exit(changed_files);
            }

            sleep((unsigned int)freq);
            if (last_modification != st.st_mtime)
            {
                last_modification = st.st_mtime;

                int check = write1(file_from_list_path, file_content_array, (int)size,
                                   last_modification);
                if (check < 0)
                {
                    fprintf(stderr, "writing backup problem\n");
                    exit(changed_files);
                }
                free(file_content_array);

                size = (size_t)get_file_size(file_from_list_path);
                file_content_array = getFile(file_from_list_path);
                if (!file_content_array)
                {
                    exit(changed_files);
                }
                changed_files++;
            }
        }
    }
    free(file_content_array);
    return changed_files;
}

void start_all(pid_state_t *pids, int size)
{
    int i;
    for (i = 0; i < size; i++)
    {
        pids[i].stoppped = 0;
        kill(pids[i].pid, SIGUSR2);
    }
}
void stop_all(pid_state_t *pids, int size)
{
    int i;
    for (i = 0; i < size; i++)
    {
        pids[i].stoppped = 1;
        kill(pids[i].pid, SIGUSR1);
    }
}

void stop(pid_state_t *pids, int size, int pid)
{
    int found = 0;
    int i;
    for (i = 0; i < size; i++)
    {
        if (pids[i].pid == pid)
        {
            pids[i].stoppped = 1;
            kill(pid, SIGUSR1);
            found = 1;
        }
    }
    if (!found)
    {
        printf("given pid does not exist or is not monitoring any file right now \n");
    }
}

void start(pid_state_t *pids, int size, int pid)
{
    int found = 0;
    int i;
    for (i = 0; i < size; i++)
    {
        if (pids[i].pid == pid)
        {
            pids[i].stoppped = 0;
            kill(pid, SIGUSR2);
            found = 1;
        }
    }
    if (!found)
    {
        printf("given pid does not exist or is not monitoring any file right now \n");
    }
}

void list(pid_state_t *pids, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (pids[i].stoppped == 0)
            printf("working: %d\n", pids[i].pid);
    }
}

int handle_commands(pid_state_t *pids, int size)
{
    pids_global = pids;
    global_size = size;
    char buffer[255];
    signal(SIGINT, sigint_handler);
    while (1)
    {
        if (!fgets(buffer, 255, stdin))
        {
            fprintf(stderr, "fgets problem \n");
            exit(1);
        };
        if (!strncmp(buffer, "START ALL", 8))
        {
            start_all(pids, size);
        }
        else if (!strncmp(buffer, "STOP ALL", 7))
        {
            stop_all(pids, size);
        }
        else if (!strncmp(buffer, "STOP", 4))
        {
            int pid = convert_to_num(buffer + 5);
            printf("pid: %d\n", pid);
            stop(pids, size, pid);
        }
        else if (!strncmp(buffer, "START", 4))
        {
            int pid = convert_to_num(buffer + 5);
            printf("pid: %d\n", pid);
            start(pids, size, pid);
        }
        else if (!strncmp(buffer, "END", 3))
        {
            int i;
            for (i = 0; i < size; i++)
            {
                kill(pids[i].pid, SIGINT);
            }
            waitpid(pids[i].pid, NULL, 0);
            struct rusage usage;
            getrusage(RUSAGE_CHILDREN, &usage);
            printf("system time: %2ld[s]\n", usage.ru_stime.tv_sec);
            printf("system time: %2ld[us]\n", usage.ru_stime.tv_usec);
            printf("user time: %2ld[s]\n", usage.ru_utime.tv_sec);
            printf("user time: %2ld [s]\n", usage.ru_utime.tv_usec);

            return 1;
        }
        else if (!strncmp(buffer, "LIST", 4))
        {
            list(pids, size);
        }
        else
        {
            printf("Unknown command\n");
        }
    }
    return 1;
}
int monitor(char *list_path)
{
    FILE *fp = NULL;
    fp = fopen(list_path, "r");
    if (!fp)
    {
        fprintf(stderr, "something went wrong, cannot open list file \n");
        return -1;
    }

    long lines_count = get_line_count(list_path);
    size_t line_s = 0;
    char *line = NULL;
    int read = 0;

    pid_state_t *pids = malloc(lines_count * sizeof(pid_state_t));
    int pid_index = 0;

    //iterate through records given in list file
    while ((read = (int)getline(&line, &line_s, fp)) != -1)
    {
        struct stat st;
        time_t last_modification;
        unsigned int freq = 1;
        char *file_from_list_path = malloc(255);
        if (sscanf(line, "%255s %d", file_from_list_path, &freq) != 2)
        {
            fprintf(stderr, "cannot get file name and frequence of monitoring from file \n");
            exit(1);
        }
        if (!file_from_list_path)
        {
            fprintf(stderr, "cannot allocate memory for buffer\n");
            exit(1);
        }
        if (freq < 0)
        {
            fprintf(stderr, "wrong number in file\n");
            exit(EXIT_FAILURE);
        }
        printf("path: %s\n", file_from_list_path);
        if (stat(file_from_list_path, &st) == -1)
        {
            fprintf(stderr, "stat problem :(\n");
            return -1;
        }
        last_modification = st.st_mtime;

        int cpid = fork();
        if (cpid == 0)
        {
            int changed_files = make_backups(freq, st, last_modification,
                                             file_from_list_path);
            exit(changed_files);
        }
        else
        {
            pids[pid_index++].pid = cpid;
            pids[pid_index - 1].stoppped = 0;
        }
        free(file_from_list_path);
    }

    // AFTER MAIN WHILE -> command from users
    int commands_status = handle_commands(pids, lines_count);
    if (fclose(fp) == EOF)
    {
        fprintf(stderr, "cannot close list file, probably file was deleted \n");
        exit(1);
    }
    free(pids);
    if (commands_status < 0)
        return -1;
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "too few arguments \n");
        exit(1);
    }

    char *list_path = realpath(argv[1], NULL);
    if (!list_path)
    {
        fprintf(stderr, "wrong path of list file; cannot resolve path \n");
        exit(1);
    }

    monitor(list_path);

    return 0;
}
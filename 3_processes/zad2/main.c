#define _XOPEN_SOURCE 700

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
#include "utils.h"

int exec_mode(char *file_from_list_path, time_t last_modification, struct stat st, int freq, int monitoring_time) {
    int changed_files = 0;
    char *backup = malloc(strlen(file_from_list_path) + 20);
    sprintf(backup, "%s%s", file_from_list_path, get_time(last_modification));
    int proc = fork();
    if (proc == 0) {
        execl("cp", "cp", file_from_list_path, backup, NULL);
        exit(EXIT_SUCCESS);
    } else if (proc < 0) {
        fprintf(stderr, " something went wrong, cannot create new process\n");
        exit(changed_files);
    } else {
        int status;
        wait(&status);
        if (status == -1)
            fprintf(stderr, "canot create backup \n");
    }
    int j;
    for (j = 0; j < monitoring_time / freq; j++) {
        printf("cos tam sprawdzam, a to moje pid i plik: %d, %s\n", getpid(),file_from_list_path);
        sleep((unsigned int) freq);

        if (stat(file_from_list_path, &st) == -1) {
            fprintf(stderr, "stat problem :(\n");
            return -1;
        }

        if (last_modification != st.st_mtime) {
            ++changed_files;
            int new_proc_pid = fork();
            if (new_proc_pid == 0) {
                char *newest = malloc(strlen(file_from_list_path) + 20);
                sprintf(newest, "%s%s", file_from_list_path, get_time(st.st_mtime));
                execl("/bin/cp", "cp", file_from_list_path, newest, (char *) 0);
                free(newest);
                exit(0);
            } else if (new_proc_pid == -1) {
                fprintf(stderr, "somthing went wrong, cannot make backup for file :( \n");
                exit(changed_files);
            } else {
                int status;
                wait(&status);
                if (status != 0) {
                    fprintf(stderr, "Could not backup '%s'\n", file_from_list_path);
                }

            }
            last_modification = st.st_mtime;
        }

    }
    free(backup);
    return changed_files;
}


int prog_memory(size_t monitoring_time, size_t freq, struct stat st, time_t last_modification,
                char *file_from_list_path) {
    int changed_files = 0;
    char *file_content_array = getFile(file_from_list_path);
    if(!file_content_array){
        exit(changed_files);
    }
    size_t size = (size_t) get_file_size(file_from_list_path);
    int j;
    for (j = 0; j < monitoring_time / freq; j++) {

        if (stat(file_from_list_path, &st) == -1) {
            fprintf(stderr, "stat problem :(\n");
            return -1;
        }

        //printf("cos tam sprawdzam, a to moje pid i plik: %d, %s\n", getpid(), file_from_list_path);

        sleep((unsigned int) freq);
        if (last_modification != st.st_mtime) {
            last_modification = st.st_mtime;

            int check = write1(file_from_list_path, file_content_array, (int) size,
                               last_modification);
            if (check < 0) {
                fprintf(stderr, "writing backup problem\n");
                exit(changed_files);
            }
            free(file_content_array);

            size = (size_t) get_file_size(file_from_list_path);
            file_content_array = getFile(file_from_list_path);
            if(!file_content_array){
                exit(changed_files);
            }
            changed_files++;
        }

    }
    free(file_content_array);
    return changed_files;

}


int monitor(char *list_path, int monitoring_time, char *mode) {
    FILE *fp = NULL;
    fp = fopen(list_path, "r");
    if (!fp) {
        fprintf(stderr, "something went wrong, cannot open list file \n");
        return -1;
    }

    long lines_count = get_line_count(list_path);
    size_t line_s = 0;
    char *line = NULL;
    int read = 0;

    pid_t *pids = malloc(lines_count * sizeof(char));
    int pid_index = 0;


    //iterate through records given in list file
    while ((read = (int) getline(&line, &line_s, fp)) != -1) {
        struct stat st;
        time_t last_modification;
        int i = 0;

        //getting files and frequency of monitoring for each file
        while (line[i] != '\0')
            if ((int) line[i++] == 32)//space in ASCII
                break;
        char *i_buffer = malloc((read - i) * sizeof(char));
        if (!i_buffer) fprintf(stderr, "cannot allocate memory for buffer\n");

        char *file_from_list_path = malloc((size_t) (i - 1));
        if (!file_from_list_path) fprintf(stderr, "cannot allocate memory for buffer\n");

        memcpy(i_buffer, line + i, (size_t) (read - i));
        memcpy(file_from_list_path, line, (size_t) (i - 1));
        unsigned int freq = (unsigned int) convert_to_num(i_buffer);

        if (freq < 0) {
            fprintf(stderr, "wrong number in file\n");
            exit(EXIT_FAILURE);
        }

        if (stat(file_from_list_path, &st) == -1) {
            fprintf(stderr, "stat problem :(\n");
            return -1;
        }

        int cpid = fork();
        int changed_files = 0;
        if (cpid == 0) {
            last_modification = st.st_mtime;
            if (monitoring_time / freq != 0) {
                if (strcmp(mode, "prog_memory") == 0) {
                    changed_files = prog_memory((size_t) monitoring_time, freq, st, last_modification,
                                                file_from_list_path);
                } else { //EXEC MODE
                    changed_files = exec_mode(file_from_list_path, last_modification,
                                              st, freq, monitoring_time);
                }
            }
            exit(changed_files);

        } else {
            pids[pid_index++] = cpid;
        }
        free(i_buffer);
        free(file_from_list_path);
    }


// AFTER MAIN WHILE -> getting statuses
    int i;
    for (i = 0; i < lines_count; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        printf("pid: %d\n", pids[i]);
        printf("changed files: %d \n", WEXITSTATUS(status));
    }

    if (fclose(fp) == EOF) {
        fprintf(stderr, "cannot close list file :(");
        exit(1);
    }
    free(pids);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "too few arguments \n");
        exit(1);
    }

    char *list_path = realpath(argv[1], NULL);
    if (!list_path) {
        fprintf(stderr, "wrong path of list file; cannot resolve path \n");
        exit(1);
    }
    int monitoring_time = convert_to_num(argv[2]);
    if (monitoring_time < 0 ) {
        fprintf(stderr, "wrong type of second argument, integer expected \n");
        exit(1);
    }
    if (strcmp(argv[3], "prog_memory") != 0 && strcmp(argv[3], "exec") != 0) {
        printf("%s", argv[3]);
        fprintf(stderr, "unknown mode; did you mean prog_memory or exec?  \n");
        exit(1);
    }

    monitor(list_path, monitoring_time, argv[3]);


    return 0;
}



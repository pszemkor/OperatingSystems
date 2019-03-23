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



int prog_memory(size_t monitoring_time, size_t freq, struct stat st, time_t last_modification,
                char *file_from_list_path) {
    int changed_files = 0;

    char *file_content_array = NULL;
    size_t size = (size_t) get_file_size(file_from_list_path);
    file_content_array = malloc(size);
    printf("curr path: %s\n", file_from_list_path);
    getFile(&file_content_array, file_from_list_path);

    int j;
    for (j = 0; j < monitoring_time / freq; j++) {

        if (stat(file_from_list_path, &st) == -1) {
            fprintf(stderr, "stat problem :(\n");
            return -1;
        }
        printf("cos tam sprawdzam, a to moje pid i plik: %d, %s\n", getpid(), file_from_list_path);
        printf("in file: %s\n", file_content_array);
        sleep((unsigned int) freq);
        if (last_modification != st.st_mtime) { // checks whether file was modificated
            last_modification = st.st_mtime;
            //WRITE TO NEW FILE
            int check = write1(file_from_list_path, file_content_array, (int) size,
                               last_modification);
            if (check < 0) {
                fprintf(stderr, "writing backup problem\n");
                exit(changed_files);
            }
            //READ FILE
            size = (size_t) get_file_size(file_from_list_path);
            file_content_array = realloc(file_content_array, size);
            getFile(&file_content_array, file_from_list_path);
            changed_files++;
        }

    }
    free(file_content_array);
    return changed_files;

}


int monitor(char *list_path, int monitoring_time, char *mode, int sec_limit, int mb_limit) {
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
        for (int m = 0; m <= i - 1; m++) {
            printf("%c", line[m]);
        }
        printf("\n");
        unsigned int freq = (unsigned int) convert_to_num(i_buffer);
//        printf("i: %d\n",i);
//        printf("freq: %d\n", freq);
//        printf("line: %s\n", line);
//        printf("path: %s\n", file_from_list_path);
//        printf("strlen: %d\n", (int) strlen(file_from_list_path));
        printf("allocated %d\n", i - 1);
        if (freq < 0) {
            fprintf(stderr, "wrong number in file\n");
            exit(EXIT_FAILURE);
        }

        if (stat(file_from_list_path, &st) == -1) {
            fprintf(stderr, "stat problem :(\n");
            printf("KAJHFKDSJHFKDSJHFKSDJHFKJDSHF: %s\n", file_from_list_path);
            return -1;
        }

        int cpid = fork();
        int changed_files = 0;
        //child process is monitoring given file
        if (cpid == 0) {

            struct rlimit cpu_limit;
            cpu_limit.rlim_max = (rlim_t) sec_limit;
            setrlimit(RLIMIT_CPU,&cpu_limit);

            struct rlimit vm_limit;
            vm_limit.rlim_max = (rlim_t) mb_limit;
            setrlimit(RLIMIT_CPU,&vm_limit);

//            if(cpu_limit.rlim_cur > cpu_limit.rlim_max || vm_limit.rlim_cur > vm_limit.rlim_max){
//                fprintf(stderr,"hard limit greater than soft \n");
//                exit(changed_files);
//            }


            last_modification = st.st_mtime;
            if (monitoring_time / freq != 0) { //checks whether monitoring time is not shorter than refreshing time

                //storing file in program_memory mode:
                if (strcmp(mode, "prog_memory") == 0) {

                    changed_files = prog_memory((size_t) monitoring_time, freq, st, last_modification,
                                                file_from_list_path);

                } else { //EXEC MODE
                    char *backup = malloc(strlen(file_from_list_path) + 20);
                    sprintf(backup, "%s%s", file_from_list_path, get_time(last_modification));
                    int proc = fork();
                    if (proc == 0) {
                        execl("/bin/cp", "cp", file_from_list_path, backup, (char *) 0);
                        exit(EXIT_SUCCESS);
                    }
                    int j;
                    for (j = 0; j < monitoring_time / freq; j++) {
                        printf("cos tam sprawdzam, a to moje pid i plik: %d, %s\n", getpid(),
                               file_from_list_path);
                        sleep(freq);

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
                            }
                            last_modification = st.st_mtime;
                        }


                    }
                    free(backup);
                }
            }
            exit(changed_files);
        } else {
            pids[pid_index++] = cpid;
        }
        free(i_buffer);
        free(file_from_list_path);
    }
// AFTER MAIN WHILE:
    int i;
    for (i = 0; i < lines_count; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        printf("status: %d \n", WEXITSTATUS(status));
        int sig = WTERMSIG(status);
        printf("sig: %s\n",strsignal(sig));
        if(i == lines_count -1){
            FILE* fp_raport = fopen("raport.txt", "w+");
            if(!fp_raport){
                fprintf(stderr,"cannot create report \n");
                exit(1);
            }

            struct rusage r;
            if(getrusage(RUSAGE_CHILDREN,&r) < 0){
                fprintf(stderr,"cannot get resources usage \n");
                exit(1);
            }
            if(fprintf(fp_raport,"czas sys [us]: %ld \n",r.ru_stime.tv_usec)){
                fprintf(stderr,"cannot create report \n");
                exit(1);
            }
            if(fprintf(fp_raport,"czas sys [s]: %ld \n",r.ru_stime.tv_sec)){
                fprintf(stderr,"cannot create report \n");
                exit(1);
            }
            if(fprintf(fp_raport,"czas usr [us]: %ld \n",r.ru_utime.tv_usec)){
                fprintf(stderr,"cannot create report \n");
                exit(1);
            }
            if(fprintf(fp_raport,"czas usr [s]: %ld \n",r.ru_utime.tv_usec)){
                fprintf(stderr,"cannot create report \n");
                exit(1);
            }

            fclose(fp_raport);
        }
    }

    if (fclose(fp) == EOF) {
        fprintf(stderr, "cannot close list file :(");
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 6) {
        fprintf(stderr, "too few arguments \n");
        exit(1);
    }

    char *list_path = realpath(argv[1], NULL);
    if (!list_path) {
        fprintf(stderr, "wrong path of list file; cannot resolve path \n");
        exit(1);
    }
    int monitoring_time = convert_to_num(argv[2]);
    int cpu_limit = convert_to_num(argv[4]);
    int vm_limit = convert_to_num(argv[5]);
    if (monitoring_time < 0 ||cpu_limit < 0 || cpu_limit < 0) {
        fprintf(stderr, "wrong type of second argument, integer expected \n");
        exit(1);
    }
    if (strcmp(argv[3], "prog_memory") != 0 && strcmp(argv[3], "exec") != 0) {
        printf("%s", argv[3]);
        fprintf(stderr, "unknown mode; did you mean prog_memory or exec?  \n");
        exit(1);
    }

    monitor(list_path, monitoring_time, argv[3],cpu_limit,vm_limit*1024);


    return 0;
}



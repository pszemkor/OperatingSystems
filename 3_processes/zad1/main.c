#define _XOPEN_SOURCE 700
#include <unistd.h> 
#include <sys/wait.h>
#include <stdio.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <ftw.h>
#include <strings.h>

#define DATE_FORMAT "%d.%m.%Y %H:%M:%S"

size_t global_path_size;
char *global_op_mode;
time_t global_date;


void file_properties(const char *path, const struct stat *stat);

void traverse_files(char *path, const char *date_mode, time_t date);

int check_date(time_t last_mod_time, const char *mode, time_t date);

int show_file_wrapper(const char *path, const struct stat *stat, int typeflag, struct FTW *ftwb);

time_t getTime(char *date);

int main(int argc, char *argv[]) {

    if (argc != 4) {

        fprintf(stderr, "wrong amount of args ");
        exit(1);

    } else {
        char *path = realpath(argv[1], NULL);
        if (!path) {
            fprintf(stderr, "incorrect path");
            exit(1);
        }
        global_path_size = strlen(path);
        char *date_mode = argv[2];
        global_op_mode = date_mode;
        char *date = argv[3];
        time_t time1 = getTime(date);

        global_date = time1;
        nftw(path, show_file_wrapper, 1000, FTW_PHYS);
        free(path);

    }

    return 0;
}



void file_properties(const char *path, const struct stat *stat) {
    if (!stat || !path) {
        fprintf(stderr, "wrong stat or path");
        return;
    }
    int cpid = fork();
    if(cpid == 0){
        printf("path: %s\n", path + global_path_size);
        printf("pid dziecka: %d \n", getpid());
        printf("\n");
        execl("/bin/ls", "ls", "-l",path,NULL);
    }else{
        waitpid(cpid,NULL,0);
    }


    printf("path: %s\n", path + global_path_size);
    if (S_ISREG(stat->st_mode))
        printf("zwykły plik\n");
    else if (S_ISCHR(stat->st_mode))
        printf("specjalny plik znakowy\n");
    else if (S_ISDIR(stat->st_mode))

        printf("katalog\n");
    else if (S_ISBLK(stat->st_mode))
        printf("specjalny plik blokowy\n");
    else if (S_ISFIFO(stat->st_mode))
        printf("FIFO\n");
    else if (S_ISLNK(stat->st_mode))
        printf("dowiązanie symboliczne\n");
    else if (S_ISSOCK(stat->st_mode))
        printf("gniazdo\n");
    else {
        fprintf(stderr,"nieznany rodzaj pliku\n");
    }

    char *buf = (char *) malloc(100);
    if (!buf) {
        fprintf(stderr, "cannot allocate buffor");
        return;
    }
    printf("%ld\n", stat->st_size);
    strftime(buf, 100, DATE_FORMAT, localtime(&stat->st_atime));
    printf("%s\n", buf);
    strftime(buf, 100, DATE_FORMAT, localtime(&stat->st_mtime));
    printf("%s\n", buf);
    free(buf);
}

int check_date(time_t last_mod_time, const char *mode, time_t date) {
    if (strcmp(mode, "=") == 0) {
        if(last_mod_time == date){
            return 1;
        }else{
            return 0;
        }
    } else if (strcmp(mode, ">") == 0) {
        if(last_mod_time > date){
            return 1;
        }else{
            return 0;
        }
    } else if (strcmp(mode, "<") == 0) {
        if(last_mod_time < date){
            return 1;
        }else{
            return 0;
        }
    } else {
        fprintf(stderr,"unknown mode");
        exit(2);
    }

}

time_t getTime(char *date) {
    time_t result;
    struct tm tm_mediator = {0};

    char *tmp = strptime(date, DATE_FORMAT, &tm_mediator);

    //FROM DOC:
    //the return value points
    //to the null byte at the end of the string.
    if (tmp == NULL || *tmp != '\0') {
        fprintf(stderr, "cannot convert date");
        exit(1);
    }

    result = mktime(&tm_mediator);

    return result;
}

int show_file_wrapper(const char *path, const struct stat *stat, int typeflag, struct FTW *ftwb) {
    //avoid first directory
    if (ftwb->level == 0)
        return 0;

    if (!check_date(stat->st_mtime, global_op_mode, global_date))
        return 0;
    else

    //printf("path in sfw: %s ", path);

    file_properties(path, stat);
    return 0;
}
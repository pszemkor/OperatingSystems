#define _XOPEN_SOURCE 700

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
char *global_op_mode;
time_t global_date;

void file_properties(const char *path, const struct stat *stat);

void traverse_files(char *path, const char *date_mode, time_t date);

int check_date(time_t last_mod_time, const char *mode, time_t date);

int show_file_wrapper(const char *path, const struct stat *stat, int typeflag, struct FTW *ftwb);

time_t getTime(char *date);

int main(int argc, char *argv[]) {

    if (argc != 5) {

        fprintf(stderr, "wrong amount of args ");
        exit(1);

    } else {
        char *path = realpath(argv[1], NULL);
        if (!path) {
            fprintf(stderr, "incorrect path");
            exit(1);
        }
        char *date_mode = argv[2];
        global_op_mode = date_mode;
        char *date = argv[3];
        char *program_mode = argv[4];
        time_t time1 = getTime(date);

        global_date = time1;
        if (strcmp(program_mode, "stat") == 0) {

            if (time1 == -1) {
                free(path);
                fprintf(stderr, "incorrect date");
                exit(1);
            }
            traverse_files(path, date_mode, time1);

        } else if (strcmp(program_mode, "nftw") == 0) {

            nftw(path, show_file_wrapper, 1000, FTW_PHYS);

        } else {
            free(path);
            fprintf(stderr, "unknown mode");
            exit(1);
        }
        free(path);

    }

    return 0;
}

void traverse_files(char *path, const char *date_mode, time_t date) {
    if (!path || !date_mode || !date) {
        fprintf(stderr, "incorrect args of traverse_files()");
        return;
    }
    DIR *directory = NULL;
    if ((directory = opendir(path)) == NULL) {
        fprintf(stderr, "cannot open directory");
        return;
    }
    struct dirent *file;
    struct stat stat;

    while ((file = readdir(directory))) {
        if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0)
            continue;

        char *new_path = (char *) malloc(strlen(path) + strlen(file->d_name) + 10);
        if (!new_path) {
            fprintf(stderr, "cannot allocate memory for new path");
            return;
        }
        sprintf(new_path, "%s/%s", path, file->d_name);
        if (lstat(new_path, &stat) == -1) {
            fprintf(stderr, "lstat error");
            return;
        }
        int status = 0;
        if ((status = check_date(stat.st_mtime, date_mode, date)) == -1) {
            fprintf(stderr, "wrong date operator mode");
        }
        if (status) {
            file_properties(new_path, &stat);
        }
        if (S_ISDIR(stat.st_mode)) {
            traverse_files(new_path, date_mode, date);
        }
        free(new_path);
    }
    if (closedir(directory) == -1) {
        fprintf(stderr, "cannot close directory");
        return;
    }
}

void file_properties(const char *path, const struct stat *stat) {
    if (!stat || !path) {
        fprintf(stderr, "wrong stat or path");
        return;
    }

    printf("path: %s\n", path);
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
    file_properties(path, stat);
    return 0;
}
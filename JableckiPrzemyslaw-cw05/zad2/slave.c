#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DATE_SIZE 32
#define BUFF_SIZE 64

int convert_to_num(char *given_string) {
    if (!given_string) {
        return -1;
    }
    char *tmp;
    int result = (int) strtol(given_string, &tmp, 10);
    if (strcmp(tmp, given_string) != 0) {
        return result;
    } else {
        return -1;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "too few arguments \n");
        exit(EXIT_FAILURE);
    }

    char *fifo_path = argv[1];

    int fd = open(fifo_path, O_WRONLY);
    if (fd == -1) {
        fprintf(stderr, "cannot open fifo\n");
        exit(EXIT_FAILURE);
    }

    int N;
    if ((N = convert_to_num(argv[2])) == -1) {
        fprintf(stderr, "wrong type of second argument, integer is required\n");
        exit(EXIT_FAILURE);
    }
    printf("my pid: %d\n", getpid());
    printf("N: %d\n", N);
    printf("fifo path: %s\n",fifo_path);

    int i;
    char date_buf[DATE_SIZE];
    char buf[BUFF_SIZE];
    srand(time(NULL));
    for (i = 0; i < N; i++) {
        FILE* p;
        if ((p = popen("date","r")) == NULL){
            fprintf(stderr, "popen error\n");
            exit(EXIT_FAILURE);
        }

        fread(date_buf, sizeof(char),BUFF_SIZE,p);
        pclose(p);
        date_buf[strlen(date_buf)-1]='\0';
        sprintf(buf,"DATE: %s PID: %d",date_buf,getpid());
        printf("%s\n",buf);

        if(write(fd,buf,BUFF_SIZE) < 0){
            fprintf(stderr, "write error\n");
            exit(EXIT_FAILURE);
        }

        int sleep_time = rand()%4+2;
        sleep(sleep_time);

    }

    close(fd);

    return 0;
}
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SIZE 512

int main(int argc, char *argv[])
{
    if (argc != 2){
        fprintf(stderr, "too few arguments\n");
        exit(EXIT_FAILURE);
    }

    char* fifo_path = argv[1];
    if(mkfifo(fifo_path,0666)  == -1){
        fprintf(stderr, "cannot make fifo\n");
        exit(EXIT_FAILURE);
   }
    int fd = open(fifo_path, O_RDONLY);
    if( fd == -1){
        fprintf(stderr, "cannot open fifo\n");
        exit(EXIT_FAILURE);
    }

    char buffer[SIZE];
    while (1)
    {
        int chars = read(fd, buffer, SIZE);
        if (chars > 0){
            printf("master is reading: \n");
            printf("%s\n", buffer);
        }

        else if (chars < 0)
            exit(EXIT_FAILURE);
    }

    return 0;
}
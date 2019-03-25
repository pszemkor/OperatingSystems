#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <libgen.h>

#define DATE_FORMAT "%d-%m-%Y_%H-%M-%S"

int convert_to_num(char *given_string) {
    char *tmp;
    int result = (int) strtol(given_string, &tmp, 10);
    if (strcmp(tmp, given_string) != 0) {
        return result;
    } else {
        return -1;
    }
}
int digits(int i){
    int result = 0;
    while(i>0){
        i/=10;
        result++;
    }
    return result;
}
char *get_time(time_t time) {
    char *buf = malloc(100);
    if (!buf) {
        fprintf(stderr, "cannot allocate buffor");
        return NULL;
    }
    strftime(buf, 100, DATE_FORMAT, localtime(&time));
    return buf;
}
int main(int argc, char* argv[]){

    srand(0);
    if(argc < 5){
        fprintf(stderr,"too few args\n");
        exit(1);
    }
    char* filepath = argv[1];
    int pmin = convert_to_num(argv[2]);
    int pmax = convert_to_num(argv[3]);
    int bytes = convert_to_num(argv[4]);
    if(pmin < 0 || pmax < 0 || bytes <0 ){
        fprintf(stderr,"bad arg type\n");
        exit(1);
    }
    for(int i = 0; i < 7; i++){
        int seconds = rand() % (pmax - pmin + 1) + pmin;
        int size = 31 + digits(getpid()) + digits(seconds);
        char* chars = malloc((size_t) size);

        time_t  time1 = time(0);
        char* date = get_time(time1);
        sprintf(chars,"random:%d_",seconds);

        int size1 = 4 + digits(getpid());
        char* temp = malloc((size_t) size1);
        sprintf(temp,"pid:%d_",getpid());

        strcat(chars,temp);
        strcat(chars,date);
        FILE* fp = fopen(filepath,"a+");
        if(!fp){
            fprintf(stderr,"cannot open file\n");
            exit(1);
        }
        if(fwrite(chars, sizeof(char), (size_t) size, fp)!=size){
            fprintf(stderr,"cannot write\n");
            exit(1);
        }
        int k;
        for (k = 0; k < bytes; k++) {

            int random = (char) ('a' + rand() % 26);
            int status = putc(random, fp);
            if (status == EOF) {
                fprintf(stderr, "rand generating problem");
                exit(1);
            }
        }

        fclose(fp);
        printf("%s\n",chars);
        sleep(seconds);
    }

    return 0;
}
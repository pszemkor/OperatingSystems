//
// Created by przemek on 22.03.19.
//


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include "utils.h"


int convert_to_num(char *given_string) {
    if(!given_string){
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
long get_file_size(char *filename) {
    FILE *fp;
    long size = 0;

    fp = fopen(filename, "a");
    if (fp == NULL)
        return -1;

    fseek(fp, 0, 2);
    size = ftell(fp);
    fclose(fp);
    return size;
}
long get_line_count(char *list_file_path) {
    FILE *fp = NULL;

    fp = fopen(list_file_path, "r");
    if (!fp) {
        fprintf(stderr, "something went wrong, cannot open list file \n");
        return -1;
    }
    char *line = NULL;
    size_t line_s = 0;
    int result = 0;
    while ((int) getline(&line, &line_s, fp) != -1) {
        result++;
    }

    fclose(fp);
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
char *getFile(char *file_name) {
    struct stat st;
    if (stat(file_name, &st) != 0){
        fprintf(stderr,"stat problem:(\n");
        return NULL;
    }

    FILE* fp = fopen(file_name, "r");
    if (!fp){
        fprintf(stderr,"cannot open file :( \n");
        return NULL;
    }

    char *filebuffer = malloc((size_t) (st.st_size + 1));
    size_t size = (size_t) get_file_size(file_name);
    if (fread(filebuffer, 1, size, fp) != size){
        fprintf(stderr,"reading problem :(\n");
        return NULL;
    }

    filebuffer[size] = '\0';

    if (fclose(fp)){
        fprintf(stderr,"cannot close file\n");
        return NULL;
    }

    return filebuffer;
}
int write1(char *file_from_list_path, char *file_content, int size, time_t last_modification) {


    char *new_file_path = malloc(strlen(file_from_list_path) + 20);
    sprintf(new_file_path, "%s%s", file_from_list_path, get_time(last_modification));
    FILE *fp1 = fopen(new_file_path, "w+");
    if (fp1 == NULL) {
        fprintf(stderr, "cannot open file in order to write backup \n");
        return -1;
    }
    if (size != fwrite(file_content, sizeof(char), (size_t) size, fp1)) {
        fprintf(stderr, "cannot write into file \n");
    }
    fclose(fp1);
    free(new_file_path);
    return 1;
}
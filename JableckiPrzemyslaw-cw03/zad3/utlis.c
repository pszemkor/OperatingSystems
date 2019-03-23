//
// Created by przemek on 22.03.19.
//


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"


int convert_to_num(char *given_string) {
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
void getFile(char **file_content, char *path) {

    FILE * fp;
    char * line = NULL;
    size_t len = 0;

    fp = fopen(path, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while (getline(&line, &len, fp) != -1) {
        strcat(*file_content,line);
    }
    fclose(fp);
}
int write1(char *file_from_list_path, char *file_content, int size, time_t last_modification) {


    char *new_file_path = malloc(strlen(file_from_list_path) + 20);
    sprintf(new_file_path, "%s%s", file_from_list_path, get_time(last_modification));
    FILE *fp1 = fopen(new_file_path, "w+");
    if (fp1 == NULL) {
        printf("path to write: %s", new_file_path);
        fprintf(stderr, "cannot open file in order to write backup \n");
        return -1;
    }
    printf("what was in file: %s\n", file_content);
    if (size != fwrite(file_content, sizeof(char), (size_t) size, fp1)) {
        fprintf(stderr, "cannot write into file \n");
    }
    fclose(fp1);
    return 1;
}
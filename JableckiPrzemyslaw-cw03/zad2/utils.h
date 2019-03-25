//
// Created by przemek on 22.03.19.
//

#ifndef PROCESY_UTILS_H
#define PROCESY_UTILS_H

#include <time.h>
#define DATE_FORMAT "%d-%m-%Y_%H-%M-%S"


char *get_time(time_t time);
int convert_to_num(char *given_string);
long get_file_size(char *filename);
long get_line_count(char *list_file_path);
char *getFile(char *file_name);
int write1(char *file_from_list_path, char *file_content, int size, time_t last_modification);

#endif //PROCESY_UTILS_H

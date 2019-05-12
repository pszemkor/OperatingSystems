//
// Created by przjab98 on 10.05.19.
//

#ifndef SEMS_COMMON_H
#define SEMS_COMMON_H

#include <sys/types.h>
#include <sys/ipc.h>

#define KEY_PATH getenv("HOME")
#define COMMON_KEY ftok(KEY_PATH, 7)
#define MAX_QUEUE_SIZE 10

#define SH_NAME "/assemly_line"

#define LOAD_NAME "/load"
#define COUNT_NAME "/count"
#define INTER_NAME "/inter"
#define LOADERS_NAME "/loaders"


#define MAX_LOADERS 100

struct Package {
    pid_t workerID;
    int weight;
    clock_t time;
};

struct Queue {
    struct Package queue[MAX_QUEUE_SIZE];
    int current_size;
    int curr_load;
};

int convert_to_num(char *given_string);
void raise_error(char *err);
void pop(struct Queue *assembly_line);
void push(struct Queue *assembly_line, struct Package pack);
clock_t print_date_and_message(char* msg);
struct Package peak(struct Queue *assembly_line);

#endif //SEMS_COMMON_H

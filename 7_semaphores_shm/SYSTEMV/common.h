//
// Created by przjab98 on 10.05.19.
//

#ifndef SEMS_COMMON_H
#define SEMS_COMMON_H

#include <sys/types.h>
#include <sys/ipc.h>

#define KEY_PATH getenv("HOME")
#define COMMON_KEY ftok(KEY_PATH, 7)
#define MAX_QUEUE_SIZE 1024

#define MAX_LOADERS 100

struct Package {
    pid_t workerID;
    int weight;
    clock_t time;
};

struct Queue {
    struct Package queue[MAX_QUEUE_SIZE];
    int head;
    int tail;
    int current_size;
    int curr_load;
    int max_size;
    int max_load;
};

int convert_to_num(char *given_string);
void raise_error(char *err);
int pop(struct Queue *assembly_line, struct Package* pack);
int push(struct Queue *assembly_line, struct Package pack);


#endif //SEMS_COMMON_H


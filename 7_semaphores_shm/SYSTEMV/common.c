//
// Created by przjab98 on 10.05.19.
//

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/times.h>
#include "common.h"

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

void raise_error(char *err) {
    fprintf(stderr, "%s\n", err);
    exit(EXIT_FAILURE);
}

int pop(struct Queue *assembly_line, struct Package* pack_poped){
    if (assembly_line->head == -1) return -1;

    struct Package pack = assembly_line->queue[assembly_line->head++];

    if (assembly_line->head == assembly_line->max_size) assembly_line->head = 0;

    if (assembly_line->head == assembly_line->tail) assembly_line->head = -1;

    assembly_line->curr_load -= pack.weight;
    assembly_line->current_size--;
    *pack_poped = pack;
    return 0;
}
int push(struct Queue *assembly_line, struct Package pack){
    if (assembly_line->current_size == assembly_line->max_size || assembly_line->curr_load + pack.weight > assembly_line->max_load) {
        return -1;
    }
    if (assembly_line->head == assembly_line->tail)
        assembly_line->head = assembly_line->tail = 0;

    assembly_line->queue[assembly_line->tail++] = pack;

    assembly_line->curr_load += pack.weight;
    assembly_line->current_size++;
    if (assembly_line->tail == assembly_line->max_size) assembly_line->tail = 0;
    return 0;
}



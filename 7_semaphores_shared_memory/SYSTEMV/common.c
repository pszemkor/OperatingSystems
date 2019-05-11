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

struct Package pop(struct Queue *assembly_line) {
    if (assembly_line == NULL || assembly_line->used_size == 0)
        raise_error("Queue does not exist or has max size 0");
    if (assembly_line->current_size == 0)
        raise_error("Cannot pop any element, queue is empty");


    assembly_line->current_size --;


    return assembly_line->queue[(assembly_line->head++)%assembly_line->used_size];
}

void push(struct Queue *assembly_line, struct Package pack) {
    if (assembly_line == NULL || assembly_line->used_size == 0)
        raise_error("Queue does not exist or has max size 0");
    if (assembly_line->current_size == assembly_line->used_size)
        raise_error("Cannot push any element, queue is full");


    assembly_line->current_size++;


   // assembly_line->queue[assembly_line->tail = assembly_line->tail + 1 >= assembly_line->used_size ? 0: assembly_line->tail++] = pack;
    assembly_line->queue[(assembly_line->tail++)%assembly_line->used_size] = pack;
}

void take_sem(int semID, int semnum, int op){
    struct sembuf buf;
    buf.sem_num = semnum;
    buf.sem_op = -op;
    buf.sem_flg = 0;
    if(semop(semID, &buf, 1) == -1)
        raise_error("cannot block semaphore1");

}
void release_sem(int semID, int semnum, int op){
    struct sembuf buf;
    buf.sem_num = semnum;
    buf.sem_op = op;
    buf.sem_flg = 0;
    if(semop(semID, &buf, 1) == -1)
        raise_error("cannot release semaphore1");
}


void block_full(int semID, int weight){
    struct sembuf ops[2];

    ops[0].sem_op = -weight;
    ops[0].sem_num = 0;
   // ops[0].sem_flg = IPC_NOWAIT;

    ops[1].sem_op = -1;
    ops[1].sem_num = 1;
    //ops[1].sem_flg = IPC_NOWAIT;

//    ops[2].sem_op = -1;
//    ops[2].sem_num = 2;
    //ops[2].sem_flg = IPC_NOWAIT;

    if(semop(semID, ops, 2) == -1)
        raise_error("cannot block semaphore");
}

void release_full(int semID, int weight){
    struct sembuf ops[2];

    ops[0].sem_op = weight;
    ops[0].sem_num = 0;
    //ops[0].sem_flg = IPC_NOWAIT;

    ops[1].sem_op =  1;
    ops[1].sem_num = 1;
    //ops[1].sem_flg = IPC_NOWAIT;

//    ops[2].sem_op =  1;
//    ops[2].sem_num = 2;
    //ops[2].sem_flg = IPC_NOWAIT;

    if(semop(semID, ops, 2) == -1)
        raise_error("cannot release semaphore");
}

clock_t print_date_and_message(char* msg){
    char date[64];
    FILE *f = popen("date", "r");
    int check = fread(date, sizeof(char), 31, f);
    if (check == EOF)
        raise_error("cannot read date \n");
    pclose(f);
    char buffer[256];
    sprintf(buffer,"%s; DATE: %s\n",msg,date);
    printf("%s\n",buffer);
    struct tms buf;
    return times(&buf);
}
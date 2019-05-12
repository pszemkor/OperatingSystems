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
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "common.h"

//todo -> add some atexit()

int optional_cycles = -1;
int package_weight = 0;
int shID = -1;
struct Queue *assembly_line;


sem_t *sem_load = NULL;
sem_t *sem_count = NULL;
sem_t *sem_truck_loader = NULL;
sem_t *sem_loaders = NULL;

int main(int argc, char *argv[]) {

    if (argc != 2 && argc != 3)
        raise_error("wrong amount of arguments");

    if (argc == 2) {
        package_weight = convert_to_num(argv[1]);
    } else {
        package_weight = convert_to_num(argv[1]);
        optional_cycles = convert_to_num(argv[2]);
    }

    if (COMMON_KEY == -1)
        raise_error("key problem");
    //getting shared memory
    shID = shm_open(SH_NAME, O_RDWR, 0666);
    if (shID < 0) {
        raise_error("Cannot create shared memory");
    }
    if (ftruncate(shID, sizeof(assembly_line)) == -1)
        raise_error("cannot get size");
    assembly_line = mmap(NULL, sizeof(assembly_line), PROT_READ | PROT_WRITE, MAP_SHARED, shID, 0);
    if (assembly_line == (void *) -1)
        raise_error("Cannot get address for shared memory");

    assembly_line->current_size = 0;
    assembly_line->curr_load = 0;

    //getting semaphores
    sem_load = sem_open(LOAD_NAME, 0);
    sem_count = sem_open(COUNT_NAME, 0);
    sem_truck_loader = sem_open(INTER_NAME, 0);
    sem_loaders = sem_open(LOADERS_NAME, 0);

    while (optional_cycles == -1 || optional_cycles > 0) {
//        take_sem(semID, 3, 1);
        sem_wait(sem_loaders);
//        if (block_full(semID, package_weight) == 0) {

        sem_wait(sem_truck_loader);
        sem_wait(sem_count);
        int j;
        for (j = 0; j < package_weight; j++) {
            sem_wait(sem_load);
        }
        char msg[128];
        sprintf(msg, "Worker %d is loading package of weight: %d", getpid(), package_weight);

        struct Package package;
        package.time = print_date_and_message(msg);
        package.weight = package_weight;
        package.workerID = getpid();
        push(assembly_line, package);
        printf("in queue is: %d packages \n", assembly_line->current_size);

//        release_sem(semID, 2, 1);
        sem_post(sem_truck_loader);
//        } else {
//            printf("*********waitin'***********\n");
//        }

//        release_sem(semID, 3, 1);
        sem_post(sem_loaders);
        if (optional_cycles != -1)
            optional_cycles--;
        sleep(1);
    }

    printf("Work finished \n");
    return 0;
}

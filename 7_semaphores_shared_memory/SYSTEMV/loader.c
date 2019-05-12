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
#include "common.h"

//todo -> add some atexit()

int optional_cycles = -1;
int package_weight = 0;
int shmID = -1;
int semID = -1;
struct Queue *assembly_line;

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
    shmID = shmget(COMMON_KEY, sizeof(struct Queue), 0);
    if (shmID < 0)
        raise_error("Cannot create shared memory");

    assembly_line = shmat(shmID, NULL, 0);
    if (assembly_line == (void *) (-1))
        raise_error("Cannot get shared memory");

    semID = semget(COMMON_KEY, 0, 0);
    if (semID < 0)
        raise_error("Cannot get semaphore");

    while (optional_cycles == -1 || optional_cycles > 0) {
        take_sem(semID,3,1);
        if (block_full(semID, package_weight) == 0) {
            char msg[128];
            sprintf(msg, "Worker %d is loading package of weight: %d", getpid(), package_weight);

            struct Package package;
            package.time = print_date_and_message(msg);
            package.weight = package_weight;
            package.workerID = getpid();
            push(assembly_line, package);
            printf("in queue is: %d packages \n", assembly_line->current_size);

            release_sem(semID, 2, 1);
        } else {
            printf("*********waitin'***********\n");
        }

        release_sem(semID, 3, 1);
        if (optional_cycles != -1)
            optional_cycles--;
        sleep(1);
    }

    printf("Work finished \n");
    return 0;
}

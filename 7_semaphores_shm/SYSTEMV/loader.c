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
#include <sys/times.h>
#include "common.h"


int optional_cycles = -1;
int package_weight = 0;
int shmID = -1;
int semID = -1;
struct Queue *assembly_line;

void cleaning(){
    if (shmdt(assembly_line) == -1) {printf("Blad podczas odlaczania pamieci wspoldzielonej od przestrzeni adresowej\n");}
    else {printf("Zasoby zostaly zwolnione\n");}
}

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

    int state = atexit(cleaning);
    if (state != 0)
        raise_error("cannot set atexit function");

    struct Package package;
    package.weight = package_weight;
    package.workerID = getpid();

    while (optional_cycles == -1 || optional_cycles > 0) {
        struct sembuf sops;
        sops.sem_num = 2;
        sops.sem_op = -1;
        sops.sem_flg = 0;
        if (semop(semID, &sops, 1) == -1) raise_error( "Blad podczas przywlaszczenia semaforu LOADERS \n");

        sops.sem_num = 1;
        if (semop(semID, &sops, 1) == -1) raise_error( "Blad podczas przywlaszczenia semaforu 1 \n");

        package.time = times(NULL);
        if(push(assembly_line,package) == -1){
            printf("Nie mozna polozyc paczki. Czekam na zwolnienie tasmy \n");
            sops.sem_num = 0;
            sops.sem_op = 1;
            if (semop(semID, &sops, 1) == -1) raise_error( "Blad podczas budzenia 0a \n");

            sops.sem_num = 1;
            sops.sem_op = 1;
            if (semop(semID, &sops, 1) == -1) raise_error( "Blad podczas oddawania semaforu 1 \n");

            sops.sem_num = 0;
            sops.sem_op = -1;
            if (semop(semID, &sops, 1) == -1) raise_error( "Blad podczas odbierania semaforu 0 \n");


            sops.sem_num = 1;
            sops.sem_op = -1;
            if (semop(semID, &sops, 1) == -1) raise_error( "Blad podczas przywlaszczenia semaforu 1 \n");
            package.time = times(NULL);
            if(push(assembly_line,package) == -1) raise_error( "Blad podczas umieszczania paczki na tasmie \n");
        }
        sops.sem_num = 1;
        sops.sem_op = 1;
        if (semop(semID, &sops, 1) == -1) raise_error( "Blad podczas oddawania semaforu 1 \n");

        sops.sem_num = 2;
        sops.sem_op = 1;
        if (semop(semID, &sops, 1) == -1) raise_error( "Blad podczas oddawania semaforu LOADERS \n");
        printf("Polozono paczke o rozmiarze %d o czasie %ld \n",package.weight,package.time);

        if (optional_cycles != -1)
            optional_cycles--;

    }

    printf("Work finished \n");
    return 0;
}


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


static int semID = -1;
static int shID = -1;
struct Queue *assembly_line;

static int packages_on_truck_weight = 0;
int max_truck_load = -1;
int semTaken = 0;

void cleaning() {

    if (shID >= 0) {
        shmctl(shID, IPC_RMID, NULL);
    }
    if (semID >= 0) {
        assembly_line->current_size = 0;
        shmdt(assembly_line);
        semctl(semID, 0, IPC_RMID);
    }
}


void signal_handler(int signo) {

    printf("Got signal %d\n", signo);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 4)
        raise_error("wrong amount of arguments");

    max_truck_load = convert_to_num(argv[1]);
    int max_packages_count = convert_to_num(argv[2]);
    int max_assembly_line_load = convert_to_num(argv[3]);

    if (max_assembly_line_load < 0 || max_truck_load < 0 || max_packages_count < 0)
        raise_error("wrong arguments");

    int state = atexit(cleaning);
    if (state != 0)
        raise_error("cannot set atexit function");

    //creating shared memory
    shID = shmget(COMMON_KEY, sizeof(struct Queue), IPC_EXCL | IPC_CREAT | 0666);
    if (shID < 0) {
        fprintf(stderr, "Cannot create shared memory");
        exit(EXIT_FAILURE);
    };
    assembly_line = shmat(shID, NULL, 0);
    if (assembly_line == (void *) -1)
        raise_error("Cannot get address for shared memory");
    assembly_line->current_size = 0;
    assembly_line->curr_load = 0;
    assembly_line->max_load = max_assembly_line_load;
    assembly_line->max_size = max_packages_count;
    assembly_line->head = -1;
    assembly_line->tail = 0;

    // creating semaphores
    semID = semget(COMMON_KEY, 3, IPC_EXCL | IPC_CREAT | 0666);
    if (semID < 0)
        raise_error("Cannot create semaphore");

    if (semctl(semID, 0, SETVAL, 0) == -1)
        raise_error("cannot set initial semaphore value");
    if (semctl(semID, 1, SETVAL, 1) == -1)
        raise_error("cannot set initial semaphore value");
    if (semctl(semID, 2, SETVAL, 1) == -1)
        raise_error("cannot set initial semaphore value");

    struct sigaction act;
    act.sa_handler = signal_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    int flag = 1;
    struct Package package;
    struct sembuf sops;
    sops.sem_flg = 0;

    printf("First empty truck is coming!\n");
    int status;
    while (flag) {
        sops.sem_num = 0;
        sops.sem_op = -1;
        printf("here!\n");
        if (semop(semID, &sops, 1) == -1) raise_error( "Blad przy przywlaszczeniu semaforu 0 \n");
        printf("here2!\n");
        sops.sem_num = 1;
        sops.sem_op = -1;
        if (semop(semID, &sops, 1) == -1) raise_error( "Blad przy przywlaszczeniu semaforu BELT \n");
        semTaken = 1;
        printf("here3\n");
        status = pop(assembly_line, &package);
        while (status == 0) {
            if (packages_on_truck_weight == max_truck_load) {
                printf("Brak miejsca - nastepuje rozladowanie ciezarowki \n");
                packages_on_truck_weight = 0;
                printf("Ciezarowka zostala rozladowana i podjechala pusta \n");
            }
            packages_on_truck_weight++;
            printf("Zaladowano na ciezarowke paczke o wadze: %d, pid: %d. Minelo %ld czasu od polozenia na tasmie. Pozostalo %d wolnych miejsc \n",
                   package.weight, package.workerID, times(NULL) - package.time/sysconf(_SC_CLK_TCK), max_truck_load - packages_on_truck_weight);
            status = pop(assembly_line, &package);
        }
        sops.sem_num = 1;
        sops.sem_op = 1;
        if (semop(semID, &sops, 1) == -1) raise_error( "Blad przy oddawaniu semaforu BELT \n");
        semTaken = 0;

        sops.sem_num = 0;
        sops.sem_op = 1;
        if (semop(semID, &sops, 1) == -1) raise_error( "Blad przy oddawaniu semaforu 0 \n");

    }
    return 0;
}


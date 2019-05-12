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
int got_signal = 0;

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
    if (semctl(semID, 2, SETVAL, 0) == -1)
        raise_error("Cannot lock the assembly line!");

    while (assembly_line->current_size > 0) {

        struct Package package = peak(assembly_line);

        if (package.weight > max_truck_load - packages_on_truck_weight) {

            print_date_and_message("truck is full");
            sleep(1);
            print_date_and_message("new truck has come");
            packages_on_truck_weight = 0;

        } else {
            pop(assembly_line);
            packages_on_truck_weight += package.weight;
            printf("New package has been loaded, time diff: %f, worker PID: %d, current load: %d \n",
                   (double) (times(NULL) - package.time) / sysconf(_SC_CLK_TCK), package.workerID,
                   packages_on_truck_weight);

        }

    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {

    struct sigaction act;
    act.sa_handler = signal_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

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


    printf("First empty truck is coming!\n");

    //creating shared memory
    shID = shmget(COMMON_KEY, sizeof(struct Queue), IPC_EXCL | IPC_CREAT | 0666);
    if (shID < 0) {
        fprintf(stderr, "Cannot create shared memory");
        exit(EXIT_FAILURE);
    }

    assembly_line = shmat(shID, NULL, 0);
    if (assembly_line == (void *) -1)
        raise_error("Cannot get address for shared memory");
    assembly_line->current_size = 0;
    assembly_line->curr_load = 0;

    // creating semaphores
    semID = semget(COMMON_KEY, 4, IPC_EXCL | IPC_CREAT | 0666);
    if (semID < 0)
        raise_error("Cannot create semaphore");

    if (semctl(semID, 0, SETVAL, max_assembly_line_load) == -1)
        raise_error("cannot set initial semaphore value");
    if (semctl(semID, 1, SETVAL, max_packages_count) == -1)
        raise_error("cannot set initial semaphore value");
    if (semctl(semID, 2, SETVAL, 1) == -1)
        raise_error("cannot set initial semaphore value");
    if (semctl(semID, 3, SETVAL, 1) == -1)
        raise_error("cannot set initial semaphore value");

    int flag = 1;
    while (flag) {
        if (assembly_line->current_size > 0) {

            struct Package package = peak(assembly_line);

            if (package.weight > max_truck_load - packages_on_truck_weight) {
                take_sem(semID, 2, 1);

                print_date_and_message("truck is full");
                sleep(1);
                print_date_and_message("new truck has come");
                packages_on_truck_weight = 0;

                release_sem(semID, 2, 1);
            } else {
                release_full(semID, package.weight);

                pop(assembly_line);
                packages_on_truck_weight += package.weight;
                printf("New package has been loaded, time diff: %f, worker PID: %d, current load: %d \n",
                       (double) (times(NULL) - package.time) / sysconf(_SC_CLK_TCK), package.workerID,
                       packages_on_truck_weight);

                release_sem(semID, 2, 1);
            }
        } else {
            printf("Waiting for packages\n");
        }
        sleep(1);

    }

    return 0;
}
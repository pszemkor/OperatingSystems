#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/times.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include "common.h"


sem_t *sem_load = NULL;
sem_t *sem_count = NULL;
sem_t *sem_truck_loader = NULL;
sem_t *sem_loaders = NULL;
int shID = -1;
struct Queue *assembly_line;

static int packages_on_truck_weight = 0;
int max_truck_load = -1;


void cleaning() {
    sem_wait(sem_truck_loader);

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
    sem_close(sem_load);
    sem_close(sem_loaders);
    sem_close(sem_truck_loader);
    sem_close(sem_count);

    sem_unlink(INTER_NAME);
    sem_unlink(COUNT_NAME);
    sem_unlink(LOADERS_NAME);
    sem_unlink(LOAD_NAME);

    munmap(assembly_line, sizeof(struct Queue));

    shm_unlink(SH_NAME);

}


void signal_handler(int signo) {

    printf("Got signal %d\n", signo);
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
    shID = shm_open(SH_NAME, O_RDWR | O_CREAT | O_TRUNC, 0666);
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

    // creating semaphores
    sem_load = sem_open(LOAD_NAME, O_CREAT, 0666, max_assembly_line_load);
    sem_count = sem_open(COUNT_NAME, O_CREAT, 0666, max_packages_count);
    sem_truck_loader = sem_open(INTER_NAME, O_CREAT, 0666, 1);
    sem_loaders = sem_open(LOADERS_NAME, O_CREAT, 0666, 1);

    int flag = 1;
    while (flag) {
        if (assembly_line->current_size > 0) {

            struct Package package = peak(assembly_line);

            if (package.weight > max_truck_load - packages_on_truck_weight) {
                //take_sem(semID, 2, 1);
                sem_wait(sem_truck_loader);

                print_date_and_message("truck is full");
                sleep(1);
                print_date_and_message("new truck has come");
                packages_on_truck_weight = 0;

                //release_sem(semID, 2, 1);
                sem_post(sem_truck_loader);

            } else {
                //release_full(semID, package.weight);
                sem_wait(sem_truck_loader);
                sem_post(sem_count);
                int j;
                for (j = 0; j < package.weight; j++) {
                    sem_post(sem_load);
                }

                pop(assembly_line);
                packages_on_truck_weight += package.weight;
                printf("New package has been loaded, time diff: %f, worker PID: %d, current load: %d \n",
                       (double) (times(NULL) - package.time) / sysconf(_SC_CLK_TCK), package.workerID,
                       packages_on_truck_weight);

                //release_sem(semID, 2, 1);
                sem_post(sem_truck_loader);
            }
        } else {
            printf("Waiting for packages\n");
        }
        sleep(1);

    }

    return 0;
}
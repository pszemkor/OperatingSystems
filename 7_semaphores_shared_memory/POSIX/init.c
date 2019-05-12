//
// Created by przjab98 on 10.05.19.
//

#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>

int loaders_count = -1;
pid_t programs[MAX_LOADERS];

void signal_handler(int signo) {
    int i;
    for (i = 0; i < loaders_count; i++) {
        kill(programs[i], SIGINT);
    }
    for (i = 0; i < loaders_count; i++) {
        waitpid(programs[i], NULL, 0);
    }
}

int main(int argc, char *argv[]) {

    //todo -> add signal handler
    struct sigaction act;
    act.sa_handler = signal_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    if (argc != 2)
        raise_error("wrong amount of arguments");

    loaders_count = convert_to_num(argv[1]);
    int max_weight = convert_to_num(argv[2]);
    if (loaders_count < 0 || max_weight < 0)
        raise_error("wring type of arguments");

    if (loaders_count > MAX_LOADERS)
        raise_error("too many workers wanted");


    int i;
    for (i = 0; i < MAX_LOADERS; i++) {
        programs[i] = -1;
    }
    time_t t;
    srand(time(&t));

    for (i = 0; i < loaders_count; i++) {
        if ((programs[i] = fork()) == 0) {
            execl("./worker", "worker", rand() % (max_weight - 1) + 1, NULL);
        }
    }


    return 0;
}
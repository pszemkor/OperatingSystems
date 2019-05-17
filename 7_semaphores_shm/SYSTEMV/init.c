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

void start_loaders(int L, int N){
    while(L--) {
        int n = rand()%N + 1;
        if(fork() == 0) {
            char nstr[9];
            sprintf(nstr, "%d", n);
            int c = rand()%20;
            if(c > 10) {
                char cstr[9];
                sprintf(cstr, "%d", c);
                execl("./loader", "./loader", nstr, cstr, NULL);

            } else {
                execl("./loader", "./loader", nstr, NULL);

            }
        }
    }
}

int main(int argc, char **argv) {
    int L = convert_to_num(argv[1]);
    int N = convert_to_num(argv[2]);
    srand(time(0));
    start_loaders(L, N);
    return 0;
}




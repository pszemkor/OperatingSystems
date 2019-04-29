//
// Created by przjab98 on 27.04.19.
//


#include "chat.h"

void raise_error(char *msg){
    fprintf(stderr,"%s",msg);
    exit(1);
}

key_t getServerQueueKey(){
    char* home = getenv("HOME");
    if(!home)
        raise_error("cannot resolve HOME \n");
    key_t s_key = ftok(home, LETTER);
    if(s_key == -1)
        raise_error("cannot get server key\n");
    return s_key;

}

key_t getClientQueueKey(){
    char* home = getenv("HOME");
    if(!home)
        raise_error("cannot resolve HOME \n");
    key_t c_key = ftok(home, getpid());
    if(c_key == -1)
        raise_error("cannot get client key\n");
    return c_key;
}
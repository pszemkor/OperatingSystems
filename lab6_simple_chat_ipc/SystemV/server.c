#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include "chat.h"

//signatures of available commands
void init(int clientPID, char msg[MAX_MSG_LENGTH]);
void echo(int clientID, char msg[MAX_MSG_LENGTH]);
void stop(int clientID);
void list(int clientID);
void friends(int clientID, char msg[MAX_MSG_LENGTH]);
void add(int clientID, char msg[MAX_MSG_LENGTH]);
void delete(int clientID, char msg[MAX_MSG_LENGTH]);
void _2all(int clientID, char msg[MAX_MSG_LENGTH]);
void _2friends(int clientID, char msg[MAX_MSG_LENGTH]);
void _2one(int clientID, char msg[MAX_MSG_LENGTH]);

//utills
void executeCommands(struct msg *msg);

void sendMessage(enum MSG_COMMAND type, char msg[MAX_MSG_LENGTH], int clientID);

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

int working = 1;
int serverQueueID = -1;
int workingClients = 0;

typedef struct {
    int clientQueue;
    int friends[MAX_CLIENTS];
    int curr_friends_number;
    pid_t pid;
} client_t;

client_t clients[MAX_CLIENTS];

void exitHandler(int signo) {

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].clientQueue != -1) {
            //SEND SOME STOP!
            kill(clients[i].pid, SIGINT);
        }
    }

    if (msgctl(serverQueueID, IPC_RMID, NULL) == -1)
        raise_error("cannot delete server queue\n");
    else
        printf("Server queue deleted successfully \n");
    exit(EXIT_SUCCESS);
}

int main() {
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[i].clientQueue = -1;
        clients[i].curr_friends_number = 0;
    }
    //handle SIGINT -> delete queue
    struct sigaction act;
    act.sa_handler = exitHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    if ((serverQueueID = msgget(getServerQueueKey(), IPC_CREAT  | 0666)) == -1)
        raise_error("cannot create server queue \n");

    struct msg msgBuff;
    while (working) {
        if (msgrcv(serverQueueID, &msgBuff, MSGSZ, -COMMAND_TYPES, 0) == -1)
            raise_error("cannot receive message \n");
        executeCommands(&msgBuff);
    }

    if (msgctl(serverQueueID, IPC_RMID, NULL) == -1)
        raise_error("cannot delete server queue\n");
    else
        printf("Server queue deleted successfully \n");

    return 0;
}

void executeCommands(struct msg *msg) {
    printf("Server got request \n");
    long type = msg->mType;
    if (type == STOP) {
        stop(msg->sender);
    } else if (type == INIT) {
        init(msg->sender, msg->msg);
    } else if (type == ECHO) {
        echo(msg->sender, msg->msg);
    } else if (type == FRIENDS) {
        friends(msg->sender, msg->msg);
    } else if (type == LIST) {
        list(msg->sender);
    } else if (type == _2ALL) {
        _2all(msg->sender, msg->msg);
    } else if (type == _2ONE) {
        _2one(msg->sender, msg->msg);
    } else if (type == _2FRIENDS) {
        _2friends(msg->sender,msg->msg);
    } else if (type == ADD) {
        add(msg->sender, msg->msg);
    } else if (type == DEL) {
        delete(msg->sender, msg->msg);
    } else {
        raise_error("unknown message type \n");
    }
}

void sendMessage(enum MSG_COMMAND type, char msg[MAX_MSG_LENGTH], int clientID) {
    if (clientID >= MAX_CLIENTS || clientID < 0 || clients[clientID].clientQueue < 0) {
        raise_error("cannot send message to this client \n");
    }

    struct msg message;
    // sender is useless field in this case
    message.sender = -1;
    message.mType = type;
    strcpy(message.msg, msg);
    
    if (msgsnd(clients[clientID].clientQueue, &message, MSGSZ, IPC_NOWAIT))
        raise_error("cannot send message to client\n");
}

void init(int clientPID, char msg[MAX_MSG_LENGTH]) {
    int id;
    for (id = 0; id < MAX_CLIENTS; id++) {
        if (clients[id].clientQueue == -1)
            break;
    }

    if (id >= MAX_CLIENTS)
        raise_error("too many clients \n");

    int client_queue = -1;
    sscanf(msg, "%d", &client_queue);
    if (client_queue < 0)
        raise_error("cannot read client queue id \n");

    clients[id].clientQueue = client_queue;
    clients[id].pid = clientPID;
    clients[id].curr_friends_number = 0;

    printf("New client initialized, his id: %d \n", id);

    char toClient[MAX_MSG_LENGTH];
    sprintf(toClient, "%d", id);
    sendMessage(INIT, toClient, id);
    workingClients++;


}

void echo(int clientID, char msg[MAX_MSG_LENGTH]) {
    printf("Echo for client: %d is %s\n", clientID, msg);
    char response[MAX_MSG_LENGTH];
    char date[64];
    FILE *f = popen("date", "r");
    int check = fread(date, sizeof(char), 31, f);
    if(check == EOF )
        raise_error("cannot read date \n");
    pclose(f);
    sprintf(response, "%s date: %s", msg, date);
    //printf("what i sent back: %s \n", response);
    sendMessage(ECHO, response, clientID);
}

void stop(int clientID) {
    if(clientID >= 0 && clientID < MAX_CLIENTS){
        clients[clientID].clientQueue = -1;
        clients[clientID].curr_friends_number = 0;
        for (int i = 0; i < MAX_CLIENTS; i++)
            clients[clientID].friends[i] = -1;
        workingClients--;
        if(workingClients == 0){
            kill(getpid(), SIGINT);
        }
    }



}

void list(int clientID) {
    printf("List will be sent to client with id: %d \n", clientID);
    char response[MAX_MSG_LENGTH], buf[MAX_MSG_LENGTH];
    strcpy(response, "");
    int i = 0;
    for (; i < MAX_CLIENTS; i++) {
        if (clients[i].clientQueue >= 0) {
            sprintf(buf, "ID: %d  queueID: %d \n", i, clients[i].clientQueue);
            strcat(response, buf);
        }
    }
    sendMessage(LIST, response, clientID);
}

void makeFriendsList(int clientID, char friends[MAX_MSG_LENGTH]) {
    char *friend = strtok(friends, SPLITTER);

    while (friend && clients[clientID].curr_friends_number < MAX_CLIENTS) {
        //to improve:
        int f = convert_to_num(friend);
        if (f < 0 || f >= MAX_CLIENTS || f < 0 || clientID == f) {
            printf("friend: %s cannot be added (wrong type or value)\n", friend);
        }

        int found = 0;
        int i = 0;

        //friends cannot be repeated
        for (; i < clients[clientID].curr_friends_number; i++)
            if (f == clients[clientID].friends[i]) {
                found = 1;
            }
        if(!found){
            clients[clientID].friends[clients[clientID].curr_friends_number++] = f;
            printf("\tFriend: %d has just been added \n", f);
        }else{
            printf("\tFriend: %d has been already added \n", f);
        }
        friend = strtok(NULL, SPLITTER)
                ;
    }
}

void friends(int clientID, char msg[MAX_MSG_LENGTH]) {
    if (clientID >= MAX_CLIENTS || clientID < 0 || clients[clientID].clientQueue < 0) {
        raise_error("wrong client \n");
    }

    printf("server will prepare new list of client's firends \n");

    char friends[MAX_MSG_LENGTH];
    int state = sscanf(msg, "%s", friends);

    if (state != 1)
        raise_error("cannot elicit friends list \n");

    //override
    clients[clientID].curr_friends_number = 0;
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[clientID].friends[i] = -1;
    }

    makeFriendsList(clientID, friends);
}

void add(int clientID, char msg[MAX_MSG_LENGTH]) {
    if (clientID >= MAX_CLIENTS || clientID < 0 || clients[clientID].clientQueue < 0) {
        raise_error("wrong client \n");
    }

    char friends[MAX_MSG_LENGTH];
    int state = sscanf(msg, "%s", friends);

    if (state != 1)
        raise_error("cannot elicit friends list \n");

    makeFriendsList(clientID, friends);
}

void delete(int clientID, char msg[MAX_MSG_LENGTH]) {
    if (clientID >= MAX_CLIENTS || clientID < 0 || clients[clientID].clientQueue < 0) {
        raise_error("wrong client \n");
    }

    char list[MAX_MSG_LENGTH];
    int num = sscanf(msg, "%s", list);
    char *elem = NULL;
    int id = -1;
    int i = 0;

    if (num == EOF || num == 0) {
        raise_error("cannot scan data \n");
    } else if (num == 1) {
        elem = strtok(list, SPLITTER);
        while (elem != NULL && (clients[clientID].curr_friends_number) > 0) {
            id = (int) strtol(elem, NULL, 10);
            i = 0;
            for (; i < (clients[clientID].curr_friends_number); i++)
                if (id == clients[clientID].friends[i]) break;
            if (i >= (clients[clientID].curr_friends_number))id = -1;
            if (id < MAX_CLIENTS && id >= 0 && id != clientID) {
                clients[clientID].friends[i] = clients[clientID].friends[(clients[clientID].curr_friends_number) - 1];
                (clients[clientID].curr_friends_number)--;
            }
            elem = strtok(NULL, SPLITTER);
        }
    }

    printf("\tfriends after deleting: \n");
    int j;
    for(j = 0; j < clients[clientID].curr_friends_number; j++){
        printf("\t ID: %d\n", clients[clientID].friends[j]);
    }
}

void _2all(int clientID, char msg[MAX_MSG_LENGTH]) {
    printf("Server will send messages to all clients \n");

    char response[MAX_MSG_LENGTH];
    char date[32];
    FILE *p = popen("date", "r");
    int check = fread(date, sizeof(char), 31, p);
    if(check == EOF )
        raise_error("cannot read date \n");
    pclose(p);

    sprintf(response, "message: %s from: %d date: %s \n", msg, clientID, date);

    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].clientQueue == -1 || i == clientID)
            continue;
        sendMessage(_2ALL, response, i);
        kill(clients[i].pid, SIGRTMIN);
    }
}

void _2one(int clientID, char msg[MAX_MSG_LENGTH]) {
    printf("Server will send messages to one client \n");
    char date[32];

    char content[MAX_MSG_LENGTH];
    char response[MAX_MSG_LENGTH];

    int addressee;
    FILE *f = popen("date", "r");
    int check = fread(date, sizeof(char), 31, f);
    if(check == EOF )
        raise_error("cannot read date \n");
    pclose(f);
    int x = sscanf(msg, "%i %s", &addressee, content);
    if (x != 2)
        raise_error("cannot elicit address and content of message\n");
    sprintf(response, "message %s from: %d date: %s \n", content, clientID, date);

    if (addressee >= MAX_CLIENTS || addressee < 0 || clients[addressee].clientQueue < 0) {
        raise_error("wrong addressee \n");
    }

    if(addressee == clientID){
        printf("cannot send message to yourself \n");
        return;
    }

    sendMessage(_2ONE, response, addressee);
    kill(clients[addressee].pid, SIGRTMIN);


}

void _2friends(int clientID, char msg[MAX_MSG_LENGTH]) {
    printf("Server will send messages to friends of client \n");
    char response[MAX_MSG_LENGTH];
    char date[64];
    FILE *f = popen("date", "r");
    int check = fread(date, sizeof(char), 31, f);
    if(check == EOF )
        raise_error("cannot read date \n");
    pclose(f);
    sprintf(response, "message: %s from: %d date: %s\n", msg, clientID, date);
    int i = 0;
    for (; i < clients[clientID].curr_friends_number; i++) {
        int addressee = clients[clientID].friends[i];
        if(clients[clientID].friends[i] == -1)
            continue;
        if (addressee >= MAX_CLIENTS || addressee < 0 || clients[addressee].clientQueue < 0) {
            raise_error("wrong addressee \n");
        }
        printf("%i\t", addressee);
        sendMessage(_2FRIENDS, response, addressee);
        kill(clients[addressee].pid, SIGRTMIN);

    }
    printf("\n");
}



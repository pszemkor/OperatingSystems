#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#define MAX_PROGS_IN_LINE 30
#define MAX_ARGS_IN_PROG 10
#define MAX_ARG_LENGTH 30


typedef struct command {
    char **arguments;
    int arg_count;
} program_t;

typedef struct node {
    struct node *next;
    pid_t pid;
} node_t;

void push(node_t **head_ref, pid_t new_pid) { // at the end

    node_t *newNode = malloc(sizeof(node_t));

    if (newNode == NULL) {
        fprintf(stderr, "Unable to allocate memory for new node\n");
        exit(EXIT_FAILURE);
    }
    newNode->pid = new_pid;
    newNode->next = NULL;

    if ((*head_ref) == NULL) {
        (*head_ref) = newNode;
    } else if ((*head_ref)->next == NULL) {
        (*head_ref)->next = newNode;
    } else {
        node_t *current = (*head_ref);
        while (1) {
            if (current->next == NULL) {
                current->next = newNode;
                break;
            }
            current = current->next;
        }
    }
}

program_t **prepare_array() {
    program_t **programs =malloc(sizeof(program_t *) * MAX_PROGS_IN_LINE);
    int i;
    for (i = 0; i < MAX_PROGS_IN_LINE; i++) {
        programs[i] = malloc(sizeof(program_t));
        programs[i]->arguments = malloc(sizeof(char *) * MAX_ARGS_IN_PROG);
        programs[i]->arg_count = -1;
        int j;
        for (j = 0; j < MAX_ARGS_IN_PROG; j++) {
            programs[i]->arguments[j] = malloc(sizeof(char) * MAX_ARG_LENGTH);
        }
    }

    return programs;
}

program_t **parse_programs(char *line, program_t **programs, int *prog_index) {
    char *token;
    token = strtok(line, " \n");
    *prog_index = 0;
    int arg_index = 0;

    while (token) {
//didn't work without second condition in if statement :>
        if (token[0] == '|' && token[1] == '\0') {
            arg_index = 0;
            (*prog_index)++;

        } else {
            strncpy(programs[*prog_index ]
                            ->arguments[arg_index++], token, MAX_ARG_LENGTH);
            programs[*prog_index ]->
                    arg_count = arg_index;
        }
        token = strtok(NULL, " \n");
    }
    return programs;
}

void delete_array(program_t** programs){
    int i;
    for (i = 0; i < MAX_PROGS_IN_LINE; i++) {
        int j;
        for (j = 0; j < MAX_ARGS_IN_PROG; j++){
            free(programs[i]->arguments[j]);
        }
        free(programs[i]->arguments);
        free(programs[i]);
    }
    free(programs);
}


void delete_list(node_t* head){
    while(head){
        node_t* to_delete = head;
        head = head->next;
        free(to_delete);
    }
}
int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "too few arguments \n");
        exit(EXIT_FAILURE);
    }
    char *path = argv[1];

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "cannot open file \n");
        exit(EXIT_FAILURE);
    }
    node_t *head = NULL;

    //prepare array for one line which store programs and their arguments
    program_t **programs = prepare_array();

    //start reading lines of linux commands with |
    while ((read = getline(&line, &len, fp)) != -1) {
        int prog_index = -1;
        programs = parse_programs(line, programs, &prog_index);;

        int fd_prev[2];
        int fd_curr[2];
        int i;
        for (i = 0; i <= prog_index; i++) {
            if (pipe(fd_curr) == -1) {
                fprintf(stderr, "pipe error \n");
                exit(EXIT_FAILURE);
            }

            pid_t pid = fork();
            if (pid == 0) { //child
                //to prevent from printing result of first program
                if (i > 0) {
                    dup2(fd_prev[0], STDIN_FILENO);
                    close(fd_prev[1]);
                    close(fd_prev[0]);
                }
                //copy write end of pipe to output for next process
                if (i < prog_index) {
                    dup2(fd_curr[1], STDOUT_FILENO);
                    close(fd_curr[0]);
                    close(fd_curr[1]);
                }

                programs[i]->arguments[programs[i]->arg_count] = NULL;
                execvp(programs[i]->arguments[0], programs[i]->arguments);
                exit(EXIT_SUCCESS);

            } else if (pid > 0) {
                push(&head, pid);
                if (i > 0) {
                    close(fd_prev[0]);
                    close(fd_prev[1]);
                }
                if (i < prog_index) {
                    fd_prev[1] = fd_curr[1];
                    fd_prev[0] = fd_curr[0];
                }
            } else { //error
                fprintf(stderr, "fork error \n");
                exit(EXIT_FAILURE);
            }
        }

        node_t *walker = head;
        while (walker) {
            waitpid(walker->pid, NULL, 0);
            walker = walker->next;
        }
    }

    fclose(fp);
    if (line)
        free(line);

    delete_array(programs);
    delete_list(head);

    return 0;
}
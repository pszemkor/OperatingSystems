//
// Created by przemek on 15.03.19.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "handleFiles.h"

int convert_to_num(char *given_string) {
    char *tmp;
    int result = (int) strtol(given_string, &tmp, 10);
    if (strcmp(tmp, given_string) != 0) {
        return result;
    } else {
        return -1;
    }
}

void sortFile(int record_size, int amount, char *filename, char *mode) {


    if (strcmp(mode, "lib") == 0) {
        FILE *fp = NULL;
        if ((fp = fopen(filename, "r+")) == NULL) {
            //PRINT SOME ERROR
            exit(1);
        }
        int *lowest = (int *) malloc(record_size * sizeof(int));
        int *current_record = (int *) malloc(record_size * sizeof(int));
        int curr_min;
        int j;
        for (j = 0; j < amount - 1; j++) {
            fseek(fp, j * record_size, 0);
            curr_min = getc(fp);
            int index = j * record_size;
            int k;
            for (k = j * record_size; k < amount * record_size; k += record_size) {
                fseek(fp, k, 0);
                int curr_char = getc(fp);
                //printf("in loop: %c, iteration: %d,%d",(char)curr_char,k,j);
                if (curr_min > curr_char) {
                    index = k;
                    curr_min = curr_char;
                }
            }


            //GET RECORDS
            fseek(fp, j * record_size, 0);
            if (fread(current_record, sizeof(char), (size_t) record_size, fp) != record_size) {
                //ERROR
            }

            fseek(fp, index, 0);
            if (fread(lowest, sizeof(char), (size_t) record_size, fp) != record_size) {
                //ERROR
            }

            //WRITE INTO RIGHT PLACE
            fseek(fp, j * record_size, 0);
            if (fwrite(lowest, sizeof(char), (size_t) record_size, fp) != record_size) {
                //ERROR
            }

            fseek(fp, index, 0);
            if (fwrite(current_record, sizeof(char), (size_t) record_size, fp) != record_size) {
                //ERROR

            }
        }
        free(lowest);
        free(current_record);

        if (fclose(fp) != 0) {
            exit(1);
            //PRINT SOME ERROR
        }
    } else {


        char *buf1 = (char *) malloc(record_size * sizeof(char));
        char *buf2 = (char *) malloc(record_size * sizeof(char));
        int file = open(filename, O_RDWR);
        if (file == -1) {
            //error
        }
        char curr_min;
        int j;
        for (j = 0; j < amount - 1; j++) {
            lseek(file, j * record_size, 0);
            if (read(file, buf1, record_size * sizeof(char)) != record_size) {
                //ERROR
            }
            curr_min = buf1[0];
            int index = j * record_size;
            int k;
            for (k = j * record_size; k < amount * record_size; k += record_size) {
                lseek(file, k, 0);
                char curr_char;
                if (read(file, &curr_char, sizeof(char)) != 1) {
                    //ERROR
                }
                if (curr_min > curr_char) {
                    index = k;
                    curr_min = curr_char;
                }
            }

            //GET RECORDS
            lseek(file, index, 0);
            if (read(file, buf2, sizeof(char) * record_size) != record_size) {
                //ERROR
            }

            //WRITE INTO RIGHT PLACE
            lseek(file, j * record_size, 0);
            if (write(file, buf2, sizeof(char) * record_size) != record_size) {
                //ERROR
            }

            lseek(file, index, 0);
            if (write(file, buf1, sizeof(char) * record_size) != record_size) {
                //ERROR
            }

        }
        free(buf1);
        free(buf2);
    }

}


long get_file_size(char *filename) {
    FILE *fp;
    long size = 0;

    fp = fopen(filename, "a");
    if (fp == NULL)
        return -1;

    fseek(fp, 0, 2);
    size = ftell(fp);
    fclose(fp);
    return size;
}

void generate(int record_amount, int record_size, char *name) {
    srand(0);
    FILE *fp = NULL;
    if ((fp = fopen(name, "w+")) == NULL) {
        //PRINT SOME ERROR
        exit(1);
    }
    int A = (int) 'A';
    int subtractor = 0;
    int j;
    for (j = 0; j < record_amount; j++) {
        int k;
        for (k = 0; k < record_size; k++) {
            if (k == record_size - 1) {
                putc('\n', fp);
                break;
            }
            int cos = (char) (A + rand() % 26);
            int status = putc(cos, fp);
            if (status == EOF) {
                //PRINT SOME ERROR
            }
        }
        subtractor++;
    }


//                int j;
//                for (j = 0; j < array->record_amount; j++) {
//                    int k;
//                    if(j == 1){
//                        fprintf(fp,"siema");
//                    }else
//                    for (k = 0; k < array->record_size; k++) {
//                        int A = (int)'A';
//                        int cos = (char)(A + rand() % 26);
//
//                        int status = putc(cos,fp);
//                        if(status == EOF){
//                            //PRINT SOME ERROR
//                        }
//                    }
//                }

    if (fclose(fp) != 0) {
        //PRINT SOME ERROR
        exit(1);
    }


}

int check_prerequisites(int curr_arg_index, int argc, char **argv,
                        int *record_size, int *amount, char **filename, char **mode) {

    if (curr_arg_index + 4 >= argc) {
        //PRINT SOME ERROR
        return -1;
    }

    *filename = argv[++curr_arg_index];

    if ((*amount = convert_to_num(argv[++curr_arg_index])) == -1) {
        //PRINT SOME ERROR
        return -1;
    }
    if ((*record_size = convert_to_num(argv[++curr_arg_index])) == -1) {
        //PRINT SOME ERROR
        return -1;
    }

    long chars_number = get_file_size(*filename);
    if (chars_number % *record_size != 0 || chars_number % *amount != 0) {
        //PRINT S0ME ERROR
        //CANNOT DIVIDE FILE INTO THESE RECORDS
        return -1;
    }

    *mode = argv[++curr_arg_index];

    if (strcmp(*mode, "lib") != 0 && strcmp(*mode, "sys") != 0) {
        //PRINT SOME ERROR
        //WRONG MODE
        return -1;
    }

    return curr_arg_index;

}

int check_generate_prereq(int curr_arg_index, int argc, char **argv, int *record_size, int *amount, char **filename) {
    if (curr_arg_index + 3 >= argc) {
        //PRINT SOME ERROR
        return -1;
    }

    *filename = argv[++curr_arg_index];
    if ((*amount = convert_to_num(argv[++curr_arg_index])) == -1) {
        //PRINT SOME ERROR
        return -1;
    }
    if ((*record_size = convert_to_num(argv[++curr_arg_index])) == -1) {
        //PRINT SOME ERROR
        return -1;
    }
    return curr_arg_index;
}

int check_cpy_prerequisites(int curr_arg_index, int argc, char **argv,
                            int *record_size, int *amount, char **filename1, char **filename2, char **mode) {
    if (curr_arg_index + 4 >= argc) {
        //PRINT SOME ERROR
        return -1;
    }

    *filename1 = argv[++curr_arg_index];
    *filename2 = argv[++curr_arg_index];

    if ((*amount = convert_to_num(argv[++curr_arg_index])) == -1) {
        //PRINT SOME ERROR
        return -1;
    }
    if ((*record_size = convert_to_num(argv[++curr_arg_index])) == -1) {
        //PRINT SOME ERROR
        return -1;
    }

    long chars_number_1;
    long chars_number_2;

    if ((chars_number_1 = get_file_size(*filename1)) == -1) {
        return -1;
    }

    if ((chars_number_2 = get_file_size(*filename2)) == -1) {
        return -1;
    }

    if (chars_number_1 < chars_number_2 || chars_number_1 % *amount != 0 || chars_number_1 % *record_size != 0) {
        //PRINT S0ME ERROR
        //CANNOT DIVIDE FILE INTO THESE RECORDS
        return -1;
    }

    *mode = argv[++curr_arg_index];

    if (strcmp(*mode, "lib") != 0 && strcmp(*mode, "sys") != 0) {
        //PRINT SOME ERROR
        //WRONG MODE
        return -1;
    }


    return curr_arg_index;

}

void copy(int record_size, int amount, char *filename1, char *filename2, char *mode) {

    if (strcmp(mode, "lib") == 0) {
        FILE *fp1 = NULL;
        if ((fp1 = fopen(filename1, "r")) == NULL) {
            //PRINT SOME ERROR
            exit(1);
        }
        FILE *fp2 = NULL;
        if ((fp2 = fopen(filename2, "r+")) == NULL) {
            //PRINT SOME ERROR
            exit(1);
        }

        int i;
        char *buff = (char *) malloc(sizeof(char) * record_size);
        for (i = 0; i < amount; i++) {
            int j;
            for (j = 0; j < record_size; j++) {
                buff[j] = (char) getc(fp1);
            }
            for (j = 0; j < record_size; j++) {
                putc(buff[j], fp2);
            }
        }

        free(buff);

        if (fclose(fp1) != 0) {
            exit(1);
            //PRINT SOME ERROR
        }
        if (fclose(fp2) != 0) {
            exit(1);
            //PRINT SOME ERROR
        }


    } else {

        int file1;
        int file2;
        if ((file1 = open(filename1, O_RDWR)) == -1) {
            //PRINT SOME ERROR
            exit(1);
        }
        if ((file2 = open(filename2, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
            //PRINT SOME ERROR
            exit(1);
        }

        int i;
        char *buff = (char *) malloc((size_t) record_size);
        for (i = 0; i < amount; i++) {
            // lseek(file1,i*record_size,0);
            if (read(file1, buff, (size_t) record_size) != record_size) {
                //PRINT SOME ERROR
                exit(1);
            }
            if (write(file2, buff, (size_t) record_size) == -1) {
                //PRINT SOME ERROR
                exit(1);
            }

        }

        free(buff);

        if (close(file1) == -1) {
            //PRINT SOME ERROR
            exit(1);
        }
        if (close(file2) == -1) {
            //PRINT SOME ERROR
            exit(1);
        }


    }
}
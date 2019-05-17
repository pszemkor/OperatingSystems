//
// Created by przjab98 on 17.05.19.
//

#ifndef THREADS_ASCII_CONVOLUTION_IM_H
#define THREADS_ASCII_CONVOLUTION_IM_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <zconf.h>

#define BLOCK 0
#define INTERLEAVED 1

void raise_error(char *text);


typedef struct Thread_Info {
    double **filtered_matrix;
    int **input_matrix;
    int N;
    int k;
    int m;
    int height;
    int c;
    double **filter;

} info_t;


int convert_to_num(char *given_string);

void parse_args(int *thread_count, int *mode, char **input, char *argv[]);

int **get_matrix_to_filter(char *input_matrix, int *height_out, int *width_out, int *max_col);

double **generate_filter(int c);

double **parse_filter(char *filtername);

void write_to_file(double **filtered, char *output_file, int height, int width, int max_colour);


#endif //THREADS_ASCII_CONVOLUTION_IM_H

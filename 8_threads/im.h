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
#include <unistd.h>

#define BLOCK 0
#define INTERLEAVED 1


typedef struct TimeMeasurement{
    clock_t real;
    clock_t sys;
    clock_t usr;
}measure_t;

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

double convolution(int x, int y, int c, int height, int width, double **filter, int **matrix);

void raise_error(char *text);

int calculate_index(int a, int i, int c);

int max(int a, int b);

int convert_to_num(char *given_string);

void parse_args(int *thread_count, int *mode, char **input, char *argv[]);

int **get_matrix_to_filter(char *input_matrix, int *height_out, int *width_out, int *max_col);

double **generate_filter(int c);

double **parse_filter(char *filtername, int *c);

void write_to_file(double **filtered, char *output_file, int height, int width, int max_colour);


#endif //THREADS_ASCII_CONVOLUTION_IM_H

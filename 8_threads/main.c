#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <zconf.h>
#include "im.h"


static void *block(void *arg);

static void *interleaved(void *arg);

int main(int argc, char *argv[]) {
    int c = 8;
    char *output = "filtered";
    char *input = "lena.ascii.pgm";
    char *filtername = NULL;
    int thread_count = 4, mode = 1;
    double **filter;

    if (argc == 5) {
        parse_args(&thread_count, &mode, &input, argv);
        output = argv[4];
        filter = generate_filter(c);

    } else if (argc == 6) {
        filtername = argv[4];
        output = argv[5];
        filter = parse_filter(filtername);
    } else {
        raise_error(
                "expected: 1. thread count 2. mode: block / interleaved, 3: input image, 4.(optional) filter, 5. output filename");
    }
    int height;
    int width;
    int max_colour;
    int **matrix_I = get_matrix_to_filter(input, &height, &width, &max_colour);

    double **filtered_matrix = (double **) malloc(height * sizeof(double *));
    int i;
    for (i = 0; i < height; i++)
        filtered_matrix[i] = (double *) malloc(width * sizeof(double));

    for (i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            printf("%d ", (int) matrix_I[i][j]);
        }
        printf("\n");
    }

    info_t *thread_args = malloc(thread_count * sizeof(info_t));
    pthread_t *pthreads = malloc(thread_count * sizeof(pthread_t));
//    printf("size: h: %d. w: %d \n", height, width);
    for (i = 0; i < thread_count; i++) {
        thread_args[i].k = i + 1;
        thread_args[i].m = thread_count;
        thread_args[i].filtered_matrix = filtered_matrix;
        thread_args[i].input_matrix = matrix_I;
        thread_args[i].N = width;
        thread_args[i].height = height;
        thread_args[i].c = c;
        thread_args[i].filter = filter;
        if (mode == BLOCK)
            pthread_create(&pthreads[i], NULL, &block, &thread_args[i]);
        else {
            pthread_create(&pthreads[i], NULL, &interleaved, &thread_args[i]);
        }
        sleep(1);

    }

    info_t *result = NULL;;
    for (i = 0; i < thread_count; i++) {
        void *res;
        pthread_join(pthreads[i], &res);
        result = res;
    }

    printf("*****************RESULT READY********************\n");
//    for (i = 0; i < height; i++) {
//        for (int j = 0; j < width; j++) {
//            printf("%d ", (int) result->filtered_matrix[i][j]);
//        }
//        printf("\n");
//    }

    write_to_file(result->filtered_matrix, output, height, width, max_colour);
    return 0;
}

int max(int a, int b) {
    if (a > b)
        return a;
    else
        return b;
}

int calculate_index(int a, int i, int c) {
    return max(1, a - (int) ceil((double) c / 2) + i);
}

double convolution(int x, int y, int c, int height, int width, double **filter, int **matrix) {
    double result = 0.0;

    int i, j;
    for (i = 0; i < c; i++)
        for (j = 0; j < c; j++) {
            int row = calculate_index(x, i, c);
            int col = calculate_index(y, j, c);
            if ((row >= 0 && row < height) && (col >= 0 && col < width)) {
                result += ((double) (matrix[row][col]) * filter[i][j]);
            }

        }
    return round(result);
}

static void *block(void *arg) {
    info_t *argument = arg;
    int k = argument->k;
    int N = argument->N;
    int m = argument->m;
    int height = argument->height;

    int x_start = (k - 1) * (int) ceil((double) N / m);
    int x_end = k * (int) ceil((double) N / m) - 1;
    int i;

    for (i = x_start; i <= x_end; i++) {
        if (i >= N)
            break;
        int j;
        for (j = 0; j < height; j++) {
            double conv = convolution(j, i, argument->c, height, N, argument->filter,
                                      argument->input_matrix);
            argument->filtered_matrix[j][i] = conv;
        }
    }
    pthread_exit(argument);
}

static void *interleaved(void *arg) {
    info_t *argument = arg;
    int k = argument->k;
    int N = argument->N;
    int m = argument->m;
    int height = argument->height;

    int x = k - 1;
    while (x < N) {
        int j;
        for (j = 0; j < height; j++) {
            double conv = convolution(j, x, argument->c, height, N, argument->filter,
                                      argument->input_matrix);
            argument->filtered_matrix[j][x] = conv;
        }
        x += m;
    }

    pthread_exit(argument);

}
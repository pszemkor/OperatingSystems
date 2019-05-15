#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

#define BLOCK 0
#define INTERLEAVED 1

void raise_error(char *text) {
    fprintf(stderr, "%s\n", text);
    exit(EXIT_FAILURE);
}


typedef struct Thread_Info {
    double **filtered_matrix;
    int N;
    int k;
    int m;
    int height;
    double **filter;
    double **input_matrix;
} info_t;

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

void parse_args(int *thread_count, int *mode, char **input, char *argv[]) {
    *thread_count = convert_to_num(argv[1]);
    if (strcmp(argv[2], "block") == 0)
        *mode = BLOCK;
    else if (strcmp(argv[2], "interleaved") != 0)
        *mode = INTERLEAVED;
    else
        raise_error("Unknown mode");
    *input = argv[3];
}

double **get_matrix_to_filter(char *input_matrix, int *height_out, int *width_out) {

    FILE *fp = fopen(input_matrix, "r");
    if (!fp)
        raise_error("cannot read input matrix");
    int height;
    int width;
    double **matrix = NULL;

    printf("\n");
    char *line = NULL;
    size_t len = 0;
    int start_line = 0;
    int row = 0;
    while (getline(&line, &len, fp) != -1) {

        if (start_line++ < 3) {
            if (start_line == 2) {
                char *width1 = strtok(line, " ");
                char *height1 = strtok(NULL, " ");
//                printf("%s %s\n", height1, width1);
                *height_out = height = convert_to_num(height1);
                *width_out = width = convert_to_num(width1);
                matrix = (double **) malloc(height * sizeof(double *));
                int i;
                for (i = 0; i < height; i++)
                    matrix[i] = (double *) malloc(width * sizeof(double));
            }
            continue;
        }
        int col = 0;
        char *character = strtok(line, " ");
        matrix[row][col] = convert_to_num(character);
        //printf("new line: %s", line);
        //printf("char: %s",character );
        while (character) {
            character = strtok(NULL, " ");
            if (character) {
                matrix[row][++col] = convert_to_num(character);
            }
        }
        row++;
    }

    fclose(fp);

    return matrix;
}

static void *block(void *arg);

static void *interleaved(void *arg);

double **generate_filter(int c) {

    double **filter = (double **) malloc(c * sizeof(int *));
    int i;
    for (i = 0; i < c; i++)
        filter[i] = (double *) malloc(c * sizeof(double));

    double factor = 1.0 / (c * c);
    int j, k;
    double sum = 0;
    for (j = 0; j < c; j++) {
        for (k = 0; k < c; k++) {
            filter[j][k] = factor;
            sum += factor;
        }
    }

    return filter;
}

void write_to_file(double **filtered, char *output_file, int height, int width) {
    FILE *fp = fopen(output_file, "w+");
    if (!fp)
        raise_error("Cannot open result file!");

    int i, j;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            fprintf(fp, "%d", (int) filtered[i][j]);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}

int main(int argc, char *argv[]) {

    char *output = NULL;
    char *input = "/home/przjab98/CLionProjects/sysops/threads_ascii_convolution/file1.pgm";
    char *filter = NULL;
    int thread_count = 0, mode = 0;

//    if (argc == 5) {
//        parse_args(&thread_count, &mode, &input, argv);
//        output = argv[4];
//        filter = "tmp";
//        //todo -> generate random filter
//
//    } else if (argc == 6) {
//        filter = argv[4];
//        output = argv[5];
//        //todo -> parse filter
//    } else {
//        raise_error(
//                "expected: 1. thread count 2. mode: block / interleaved, 3: input image, 4.(optional) filter, 5. output filename");
//    }
    int height;
    int width;
    double **matrix_I = get_matrix_to_filter(input, &height, &width);
    double **filtered_matrix = (double **) malloc(height * sizeof(double *));
    int i;
    for (i = 0; i < height; i++)
        filtered_matrix[i] = (double *) malloc(width * sizeof(double));

//    for (i = 0; i < height; i++) {
//        for (int j = 0; j < width; j++) {
//            printf("%f ", matrix_I[i][j]);
//        }
//        printf("\n");
//    }

    info_t *thread_args = malloc(thread_count * sizeof(info_t));
    pthread_t *pthreads = malloc(thread_count * sizeof(pthread_t));

    for (i = 0; i < thread_count; i++) {
        thread_args[i].k = i + 1;
        thread_args[i].m = thread_count;
        thread_args[i].filtered_matrix = filtered_matrix;
        thread_args[i].input_matrix = matrix_I;
        thread_args[i].N = width;
        thread_args[i].height = height;
        if (!filter)
            thread_args[i].filter = generate_filter(i);
        else
            //todo !!!
            printf("");
        if (mode == BLOCK)
            pthread_create(&pthreads[i], NULL, &block, &thread_args[i]);
        else {
            pthread_create(&pthreads[i], NULL, &interleaved, &thread_args[i]);
        }

    }
    info_t *result = NULL;
    for (i = 0; i < thread_count; i++) {
        void *res;
        pthread_join(pthreads[i], &res);
        result = res;
    }


    for (i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            printf("%f ", result->filtered_matrix[i][j]);
        }
        printf("\n");
    }

    write_to_file(result->filtered_matrix, output, height, width);


    return 0;
}

double convolution(int x, int y) {
    double result = 0.0;


    return result;
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

    for (i = x_start; i < x_end; i++) {
        if (i >= N)
            break;
        int j;
        for (j = 0; j < argument->height; j++) {
            argument->filtered_matrix[i][j] = convolution(i, j);
        }
    }


    pthread_exit(argument);
}

static void *interleaved(void *arg) {
    info_t *argument = arg;


    pthread_exit(argument);
}
//
// Created by przjab98 on 17.05.19.
//

#include "im.h"

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

int **get_matrix_to_filter(char *input_matrix, int *height_out, int *width_out, int *max_col) {

    FILE *fp = fopen(input_matrix, "r");
    if (!fp)
        raise_error("cannot read input matrix");
    int height;
    int width;
    int **matrix = NULL;

    printf("\n");
    char *line = NULL;
    size_t len = 0;
    int start_line = 0;
    int row = 0;
    char buffer[255];
    fscanf(fp, "%s", buffer);
    if (strcmp("P2", buffer) != 0)
        raise_error("wrong type of file!");
    int max_colour;
    fscanf(fp, "%d %d %d", &width, &height, &max_colour);
    matrix = (int **) malloc(height * sizeof(int *));
    int i;
    for (i = 0; i < height; i++)
        matrix[i] = (int *) malloc(width * sizeof(int));

    int j;
    for (i = 0; i < height; i++)
        for (j = 0; j < width; j++) {
            fscanf(fp, "%d", &matrix[i][j]);
        }

    *height_out = height;
    *width_out = width;
    *max_col = max_colour;
    fclose(fp);

    return matrix;
}

void raise_error(char *text) {
    fprintf(stderr, "%s\n", text);
    exit(EXIT_FAILURE);
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

void write_to_file(double **filtered, char *output_file, int height, int width, int max_colour) {
    FILE *fp = fopen(output_file, "w+");
    if (!fp)
        raise_error("Cannot open result file!");
    fprintf(fp, "P2\n");
    fprintf(fp, "%d %d\n", width, height);
    fprintf(fp, "%d\n", max_colour);
    int in_line = 0;
    int i, j;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            fprintf(fp, "%d ", (int) filtered[i][j]);
            in_line++;
            if (in_line >= 70) {
                fprintf(fp, "\n");
                in_line = 0;
            }
        }
    }

    fclose(fp);
}

double **parse_filter(char *filtername) {
    int c = 0;
    FILE *fp = fopen(filtername, "r");
    if (!fp)
        raise_error("cannot open filter file");
    fscanf(fp, "%d", &c);
    double **filter = (double **) malloc(c * sizeof(int *));
    int i, j;
    for (i = 0; i < c; i++)
        filter[i] = (double *) malloc(c * sizeof(double));

    for (i = 0; i < c; i++) {
        for (j = 0; j < c; j++) {
            fscanf(fp, "%lf", &filter[i][j]);
        }
    }

    fclose(fp);
    return filter;

}
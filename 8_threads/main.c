#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

typedef struct thread_info{
    int* array;
    int index;
}info_t;

static void* foo(void* arg){
    info_t* todo = arg;
    for(int i = todo->index*1000; i<(todo->index + 1)*1000; i++)
        todo->array[i] = 2*todo->index;
    pthread_exit(todo);
}


int main() {

    int* array = malloc(2000 * sizeof(int));
    int threads_count = 2;

    info_t* info = malloc(threads_count * sizeof(info_t));


    pthread_t* threads = malloc(2 * sizeof(pthread_t));
    int i;
    for(i = 0; i < threads_count; i++){
        info[i].array = array;
        info[i].index = i;
         pthread_create(&threads[i], NULL, &foo, &info[i]);
    }

    for(i = 0; i < threads_count; i++){
        void* res;
        pthread_join(threads[i], &res);
        info_t* res_info = res;
        for(int k = 0; k<2000; k++)
            printf("%d", res_info->array[k]);
        break;

    }
    printf("\n");
    int x_start = (3-1)*ceil((double)5/3);
    printf("%d", x_start);

    return 0;
}
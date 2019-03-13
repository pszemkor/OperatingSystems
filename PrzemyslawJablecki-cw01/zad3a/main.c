#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/times.h>
#include <ctype.h>
#include <time.h>


#ifndef DLL
#include "cw1.h"
#endif

#ifdef DLL
#include <dlfcn.h>
typedef struct search_properties{
    char* directory;
    char* filename;
    char* tmp_file;
}params_t;

#endif

void fprint_times(FILE* fp, char* comment, struct tms *tmsstart, struct tms *tmsend,struct timespec* start1,struct timespec* end1);

struct timespec diff(struct timespec start, struct timespec end);

long convert_to_num(char *given_string){
    char* tmp = calloc(strlen(given_string),sizeof(char));
    long result = strtol(given_string,&tmp,10);
    if(strcmp(tmp,given_string) != 0){
        return result;
    }else{
        return -1;
    }
}

int main(int argc ,char* argv[]){

#ifdef DLL
    void* handle = dlopen("libmyLib.so", RTLD_LAZY);
        if(handle == NULL){
            fprintf(stderr,"cannot open dynamic library");
            exit(EXIT_FAILURE);
        }
   
    char** (*create_table)(int);
    void (*change_dir_file)(params_t* ,char* , char* );
    void (*browse_directory)(params_t*);
    int (*add_new_block)(char**, int, char*);
    void (*delete_block)(char** , int ,  int );
    void (*delete_array)(char **, int );
        create_table = dlsym(handle, "create_table");
        change_dir_file = dlsym(handle,"change_dir_file");
        browse_directory = dlsym(handle,"browse_directory");
        add_new_block = dlsym(handle,"add_new_block");
        delete_block = dlsym(handle,"delete_block");
        delete_array = dlsym(handle,"delete_array");
        if(create_table == NULL|| change_dir_file == NULL ||  browse_directory == NULL || add_new_block == NULL || delete_block == NULL ||
            delete_block == NULL || delete_array ==NULL){
            fprintf(stderr,"cannot find function");
            exit(EXIT_FAILURE);
        }
#endif
        
    char** array = NULL;
    int array_size = -1;
    params_t* params = (params_t*)malloc(sizeof(params_t));

    FILE* raport;
    raport = fopen("raport3b.txt","a");
    
    if( raport == NULL){
        exit(EXIT_FAILURE);
    }


    fprintf(raport,"ROZPOCZECIE POMIARU CZASU:\n ");


    struct tms tmsstart, tmsend;
    struct timespec start1,end1;
    struct tms wholeprogram_start,wholeprogram_end;
    struct timespec program_start,program_end;
    if( times(&wholeprogram_start) == -1)
        exit(EXIT_FAILURE);
    clock_gettime(CLOCK_REALTIME, &program_start);


    if(argc > 1){
        //int size = argc;
        int i;
        for( i = 1; i < argc; i++){
            if(!strcmp(argv[i],"create_table")){

                if( times(&tmsstart) == -1)
                    exit(EXIT_FAILURE);
                clock_gettime(CLOCK_REALTIME, &start1);

                if(i + 1 == argc){
                    fprintf(stderr,"wrong amount of arguments");
                    exit(EXIT_FAILURE);
                }
                if(array_size > -1){
                    fprintf(stderr,"array has been already allocated");
                    exit(EXIT_FAILURE);
                }
                i++;
                int given_size;
                if((given_size=convert_to_num(argv[i])) != -1){
                    if(given_size <= 0){
                        fprintf(stderr, "wrong size");
                        exit(EXIT_FAILURE);
                    }
                    array_size = given_size;
                }else{
                    fprintf(stderr,"wrong argument\n");
                    exit(EXIT_FAILURE);
                }

                array = create_table(given_size);

                if( array == NULL){
                    fprintf(stderr,"creating array error");
                    exit(EXIT_FAILURE);
                }
                if( times(&tmsend) == -1)
                    exit(EXIT_FAILURE);
                clock_gettime(CLOCK_REALTIME, &end1);
                fprint_times(raport,"tworzenie tablicy (pamieci programu):\n",&tmsstart,&tmsend,&start1,&end1);
            }
            else if(!strcmp(argv[i],"search_directory")){

                if( times(&tmsstart) == -1)
                    exit(EXIT_FAILURE);
                clock_gettime(CLOCK_REALTIME, &start1);

                if(i + 3 == argc){
                    fprintf(stderr,"wrong amount of arguments");
                    exit(EXIT_FAILURE);
                }
                params->directory = argv[i+1];
                params->filename = argv[i+2];
                params->tmp_file = argv[i+3];
                i+=3;
                browse_directory(params);
                if( times(&tmsend) == -1)
                    exit(1);
                clock_gettime(CLOCK_REALTIME, &end1);
                char info [1024];
                sprintf(info,"pojedyncze przeszukiwanie i zapisanie do pliku tymczasowego dla katalogu: %s, pliku %s :\n", params->directory, params->filename);
                fprint_times(raport,info,&tmsstart,&tmsend,&start1,&end1);
            }
            else if (!strcmp(argv[i],"add")){ //takes 1 arg - tmp_file_name

                if( times(&tmsstart) == -1){
                    fprintf(stderr,"time problem");
                    exit(EXIT_FAILURE);
                }

                clock_gettime(CLOCK_REALTIME, &start1);
                if(array_size < 0){
                    fprintf(stderr,"array has not been allocated yet");
                    exit(EXIT_FAILURE);
                }
                if(i+1 == argc){
                    fprintf(stderr,"wrong amount of arguments");
                    exit(EXIT_FAILURE);
                }
                int ind;
                ind = add_new_block(array,array_size,argv[++i]);
                if(ind < 0){
                    fprintf(stderr,"no space in array");
                    exit(EXIT_FAILURE);
                }
                if( times(&tmsend) == -1){
                    fprintf(stderr,"time problem");
                    exit(EXIT_FAILURE);
                }
                clock_gettime(CLOCK_REALTIME, &end1);
                char info [1024];
                sprintf(info,"koszt pojedynczego dodania do pamieci programu z pliku %s :\n", argv[i]);
                fprint_times(raport,info,&tmsstart,&tmsend,&start1,&end1);
            }
            else if(!strcmp(argv[i],"remove_block")){

                if( times(&tmsstart) == -1)
                    exit(EXIT_FAILURE);
                clock_gettime(CLOCK_REALTIME, &start1);

                if(array_size < 0){
                    fprintf(stderr,"array has not been allocated yet");
                    exit(EXIT_FAILURE);
                }
                if(i + 1 == argc){
                    fprintf(stderr,"wrong amount of arguments");
                    exit(EXIT_FAILURE);
                }
                i++;
                int given_index;
                if((given_index=convert_to_num(argv[i])) != -1){
                    if(given_index < 0 || given_index >= array_size){
                        fprintf(stderr, "wrong index");
                        exit(EXIT_FAILURE);
                    }
                }else{
                    fprintf(stderr,"wrong argument \n");
                    exit(EXIT_FAILURE);
                }

                if(given_index < 0 || given_index >= array_size){
                    fprintf(stderr,"wrong index");
                    exit(EXIT_FAILURE);
                }
                delete_block(array,array_size,given_index);
                if( times(&tmsend) == -1)
                    exit(1);
                clock_gettime(CLOCK_REALTIME, &end1);
                fprint_times(raport,"usuwanie pojedynczego bloku:\n",&tmsstart,&tmsend,&start1,&end1);
            }
            else{
                fprintf(stderr,"wrong command name");
                exit(1);
            }
        }
    }

    if (array != NULL) delete_array(array,array_size);

    if( times(&wholeprogram_end) == -1)
        exit(1);
    clock_gettime(CLOCK_REALTIME, &program_end);
    fprint_times(raport,"czas wszystkich wykonanych operacji:\n",&wholeprogram_start,&wholeprogram_end,&program_start,&program_end);
#ifdef DLL
    dlclose(handle);
#endif
    fclose(raport);
    free(params);
    return 0;
}

void fprint_times(FILE* fp,char* comment, struct tms *tmsstart, struct tms *tmsend,struct timespec* start1,struct timespec* end1){
    static long clktck = 0;
    if(clktck == 0)
        if((clktck = sysconf(_SC_CLK_TCK)) < 0){
            exit(4);
        }
    if (fprintf(fp,"%s\n",comment) < 0){
        fprintf(stderr,"saving in raport error");
        exit(10);
    }
    if (fprintf(fp,"sys: %7.2f s\n",(tmsend->tms_cstime - tmsstart->tms_cstime)/(double)clktck) < 0){
        fprintf(stderr,"saving in raport error");
        exit(10);
    }
    if (fprintf(fp,"user: %7.2f s\n",(tmsend->tms_cutime - tmsstart->tms_cutime)/(double)clktck) < 0){
        fprintf(stderr,"saving in raport error");
        exit(10);
    }

    struct timespec diff_time = diff(*start1,*end1);

    if (fprintf(fp,"real: %7.2ld s\n",diff_time.tv_sec) < 0){
        fprintf(stderr,"saving in raport error");
        exit(10);
    }
    if (fprintf(fp,"real : %7.2ld ns\n\n",diff_time.tv_nsec)< 0){
        fprintf(stderr,"saving in raport error");
        exit(10);
    }


}

struct timespec diff(struct timespec start, struct timespec end)
{
    struct timespec temp;

    if ((end.tv_nsec-start.tv_nsec)<0)
    {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    }
    else
    {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

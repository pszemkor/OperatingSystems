#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>
#include "handleFiles.h"


int main(int argc, char **argv) {


    struct tms start;
    struct tms end;

    if (argc > 1) {
        int i;
        for (i = 1; i < argc; i++) {

            times(&start);
            char* op_name = NULL;

            if (strcmp("generate", argv[i]) == 0) {

                op_name = (char*)malloc(strlen("generate"));
                sprintf(op_name,"generate");

                int status;
                int record_amount;
                char* name;
                int record_size;
                if((status = check_generate_prereq(i,argc,argv,&record_size,&record_amount,&name)) == -1){
                    //LIPA
                }else{
                    i = status;
                    generate(record_amount,record_size,name);
                }
            }
            else if(strcmp("sort", argv[i]) == 0){
                op_name = (char*)malloc(strlen("sort"));
                sprintf(op_name,"sort");

                int status;
                int record_size;
                int amount;
                char* filename;
                char* mode;
                if((status = check_prerequisites(i,argc,argv,&record_size,&amount,&filename,&mode)) == -1){
                        //ERROR
                }else{
                    i = status;
                    //Check errors -> TO DO
                    sortFile(record_size,amount,filename,mode);
                }

            }
            else if(strcmp("copy", argv[i]) == 0){
                op_name = (char*)malloc(strlen("copy"));
                sprintf(op_name,"copy");
                int record_size;
                int amount;
                char* filename1;
                char* filename2;
                char* mode;
                int status;
                if((status = check_cpy_prerequisites(i,argc,argv,&record_size,&amount,&filename1,&filename2,&mode)) == -1){
                    //ERROR
                }else{
                    i = status;

                    if(strcmp(filename1,filename2) != 0){
                        copy(record_size,amount,filename1,filename2,mode);
                    }else{
                        //PRINT SOME ERROR
                        //TWO SAME FILES
                    }
                }

            }else{
                //PRINT SOME ERROR
                fprintf(stderr,"Wrong command");
                i = argc;
            }

            times(&end);

            if(op_name!=NULL){

                printf("%s:  sys:     %0.2fs\n",op_name, (double)(end.tms_stime - start.tms_stime)/sysconf(_SC_CLK_TCK));
                printf("%s:  usr:    %0.2fs\n",op_name, (double)(end.tms_utime - start.tms_utime)/sysconf(_SC_CLK_TCK));
                free(op_name);
            }


        }
    } else {
        fprintf(stderr,"too little args");
    }


    return 0;
}


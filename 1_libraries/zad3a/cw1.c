#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cw1.h"

char** create_table(int size){
    if(size > 0){
        char** array = (char**)calloc(size,sizeof(char*));
        return array;
    }
    return NULL;
}

void change_dir_file(params_t* params,char* dir, char* file){
    params->directory = dir;
    params->filename = file;
}

void browse_directory(params_t* params){
    char command [1024];
    sprintf(command,"find %s -name \"%s\" > %s", params->directory, params->filename, params->tmp_file);
    int status = -1;
    printf("command: %s\n",command);
    if((status = system(command)) < 0){
        fprintf(stderr,"system() error");
        exit(EXIT_FAILURE);
    }

}

int add_new_block(char** blocks, int size, char* tmp_file_name){
    FILE* fp;
    char* line = NULL;
    size_t len = 0;
    fp = fopen(tmp_file_name, "r");
    if (fp == NULL){
        fprintf(stderr,"cannot open file ");
        exit(EXIT_FAILURE);
    }

    //seeking size:
    fseek(fp, 0L, SEEK_END);
    long int filesize = ftell(fp); 
    fclose(fp);

    fp = fopen(tmp_file_name, "r");

   


    int found = 0;
    int index = -1;
    int i ;
    for(i = 0; i < size; i++){
        if (blocks[i] == NULL){
            blocks[i] = (char*)malloc(filesize);
            strcpy(blocks[i],"");
            while ( getline(&line, &len, fp) != -1){
                strcat(blocks[i],line);   
                free(line);
                line = NULL;
            }

            found = 1;
            index = i;
            break;
        }
    }
        fclose(fp);
    if(found == 0)
        return -1; //to small array
    return index;
}

void delete_block(char** blocks, int size,  int index){
    if(index >= 0 && index < size){
        if(blocks[index] != NULL){
            free(blocks[index]);
            blocks[index] = NULL;
        }
    }
}

void delete_array(char **blocks, int size){
	if (blocks != NULL) {
	int i;
		for ( i = 0; i <size; i++) {
			if (blocks[i] != NULL)
				free(blocks[i]);
		}
		free(blocks);
	}
}

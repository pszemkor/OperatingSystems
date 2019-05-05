//
// Created by przemek on 15.03.19.
//

#ifndef PLIKI_HANDLEFILES_H
#define PLIKI_HANDLEFILES_H

void sortFile(int record_size, int amount, char* filename, char*mode);
int convert_to_num(char *given_string);
long get_file_size(char* filename);
int check_prerequisites(int curr_arg_index, int argc, char** argv,
                        int *record_size, int* amount, char** filename, char** mode);
int check_generate_prereq(int curr_arg_index, int argc, char** argv, int* record_size, int* amount, char** filename);
void generate(int record_amount, int record_size, char*name);
int check_cpy_prerequisites(int curr_arg_index, int argc, char** argv,
                            int *record_size, int* amount, char** filename1,char** filename2, char** mode);
void copy(int record_size, int amount, char* filename1, char*filename2, char*mode);
#endif //PLIKI_HANDLEFILES_H

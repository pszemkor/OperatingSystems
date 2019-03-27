#ifndef sysopslib
#define sysopslib

typedef struct search_properties{
    char* directory;
    char* filename;
    char* tmp_file;
}params_t;


char** create_table(int size);
void change_dir_file(params_t* params, char* dir, char* file);
void browse_directory(params_t* params);
int add_new_block(char** blocks, int size, char* tmp_file_name);
void delete_block(char** blocks, int size,  int index);
void delete_array(char **blocks, int size);

#endif //sysopslib

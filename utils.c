//
// Created by Loïc Colat on 24/01/2020.
//

#include "utils.h"

int get_file_quantity(DIR *dr){
    struct dirent *en;
    int file_quantity = 0;
    while ((en = readdir(dr)) != NULL) {
        if(strcmp(en->d_name,".") && strcmp(en->d_name,"..")){
            file_quantity++;
        }
    }
    seekdir(dr, 0);
    return file_quantity;
}

char **get_file_list(DIR *dr, int file_quantity){
    char **file_list = NULL;
    file_list = malloc(file_quantity * sizeof(char*));
    int i = 0;
    struct dirent *en;
    while ((en = readdir(dr)) != NULL) {
        if(strcmp(en->d_name,".") && strcmp(en->d_name,"..")){
            file_list[i] = malloc((strlen(en->d_name) + 1) * sizeof(char));
            strcpy(file_list[i], en->d_name);
            i++;
        }
    }
    return file_list;
}

FileListWithQuantity get_images(char dir_path[]){
    char **file_list = NULL;
    int file_quantity = 0;
    DIR *dr;
    dr = opendir(dir_path);
    if (dr) {
        file_quantity = get_file_quantity(dr);
        file_list = get_file_list(dr, file_quantity);
        closedir(dr); //close all directory
    }

    FileListWithQuantity files;
    files.quantity = file_quantity;
    files.file_list = file_list;
    return files;
}

void flush_dir(char dir_path[]){
    DIR *dr;
    dr = opendir(dir_path);
    if (dr) {
        struct dirent *en;
        while ((en = readdir(dr)) != NULL) {
            if(strcmp(en->d_name,".") && strcmp(en->d_name,"..")){
                char *complete_path = calloc(strlen(dir_path) + strlen(en->d_name) + 2, sizeof(char));
                strcpy(complete_path,dir_path);
                strcat(complete_path,"/");
                strcat(complete_path,en->d_name);
                remove(complete_path);
            }
        }
        closedir(dr);
    }
}
//
// Created by Loïc Colat on 24/01/2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "bitmap.h"

typedef struct image_and_path {
    Image* image;
    char* path;
} ImageWithPath;

typedef struct FileListWithQuantity {
    char** file_list;
    int quantity;
    char* input_folder;
    char* output_folder;
    char* algorithm;
} FileListWithQuantity;

int verify_arg_qty(int argc);
int verify_algorithm(char *algorithm);
int get_file_quantity(DIR *dr);
char **get_file_list(DIR *dr, int file_quantity);
FileListWithQuantity get_images(char dir_path[]);
void flush_dir(char dir_path[]);

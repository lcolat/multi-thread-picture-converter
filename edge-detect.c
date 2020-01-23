/*
	//gcc edge-detect.c bitmap.c -O2 -ftree-vectorize -fopt-info -mavx2 -fopt-info-vec-all
	//UTILISER UNIQUEMENT DES BMP 24bits
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "bitmap.h"
#include <stdint.h>
#include <dirent.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

#define DIM 3
#define LENGHT DIM
#define OFFSET DIM /2
#define FILE_NAME_LENGTH_MAX 32

const float KERNEL[DIM][DIM] = {{-1, -1,-1},
							   {-1,8,-1},
							   {-1,-1,-1}};
//const float KERNEL[DIM][DIM] = {{0, -1,0},
//                                {-1,5,-1},
//                                {0,-1,0}};

//const float BOX[DIM][DIM] = 1/9{{1,1,1},
//                                {1,1,1},
//                                {1,1,1}};

#define STACK_MAX 10
void apply_effect(Image* original, Image* new_i);
typedef struct image_and_path {
    Image* image;
    char* path;
} ImageWithPath;

typedef struct FileListWithQuantity {
    char** file_list;
    int quantity;
    char* input_folder;
    char* output_folder;
} FileListWithQuantity;

typedef struct stack_t {
    ImageWithPath data[STACK_MAX];
    int count;
    int max;
    pthread_mutex_t lock;
    pthread_cond_t can_consume;
    pthread_cond_t can_produce;
} Stack;

typedef struct Color_t {
	float Red;
	float Green;
	float Blue;
} Color_e;

//on pourrait passer une structure en parametres
//pour eviter les variables globales
static Stack stack;

void stack_init() {
    pthread_cond_init(&stack.can_produce, NULL);
    pthread_cond_init(&stack.can_consume, NULL);
    pthread_mutex_init(&stack.lock, NULL);
    stack.max = STACK_MAX;
    stack.count = 0;
}
void* producer(void* arg);
void* producer(void* arg) {

    FileListWithQuantity* images = (FileListWithQuantity *) arg;
    int i = 0;
    while (true) {
        pthread_t t = pthread_self();
        pthread_mutex_lock(&stack.lock);
            if(stack.count < stack.max && i < images->quantity) {
                printf("Image file %s\n", images->file_list[i]);
                char *image_path = NULL;
                image_path = calloc((strlen(images->file_list[i]) + strlen(images->input_folder) + 2), sizeof(char));
                strcat(image_path, images->input_folder);
                strcat(image_path, "/");
                strcat(image_path,images->file_list[i]);

                printf("Producer %08x Input image path: %s\n", t, image_path);
                Image img = open_bitmap(image_path);
                free(image_path);

                Image new_i;
                apply_effect(&img, &new_i);

                char *image_out_path = NULL;
                image_out_path = calloc((strlen(images->file_list[i]) + strlen(images->output_folder) + 2), sizeof(char));
                strcat(image_out_path, images->output_folder);
                strcat(image_out_path, "/");
                strcat(image_out_path,images->file_list[i]);

                stack.data[stack.count].image = &new_i;
                stack.data[stack.count].path = image_out_path;
                stack.count++;
//                free(images->file_list[i]);
//                images->file_list[i] = NULL;
                printf("%d , %s\n", i, images->file_list[i]);
                i++;
//                printf("thread %08x produce !\n", t);
                pthread_cond_signal(&stack.can_consume);
            }
            else {
//                printf("Thread %08x can't produce\n", t);
                while(stack.count >= stack.max) {
                    pthread_cond_wait(&stack.can_produce, &stack.lock);
                }
//                printf("Thread %08x can produce again\n", t);
            }
        pthread_mutex_unlock(&stack.lock);
    }

    return NULL;
}


void* consumer(void* arg);
void* consumer(void* arg) {

    int total = 0;
    int *image_quantity = (int *) arg;
    pthread_t t = pthread_self();
    printf("Consumer %08x: %d images to write out \n",t , *image_quantity);
    while(true) {
        pthread_mutex_lock(&stack.lock);
        while(stack.count == 0) {
            printf("Waiting for consume :( \n");
            pthread_cond_wait(&stack.can_consume, &stack.lock);
        }

        stack.count--;
        printf("Writing image %s\n", stack.data[stack.count].path);
        save_bitmap(*(stack.data[stack.count].image), stack.data[stack.count].path);
        total++;
//        free(stack.data[stack.count].path);
        if(total >= *image_quantity) {
            printf("All images converted\n");
            //on ne débloque pas le mutex, pour qu'ils arretent de produire
            break;
        }
        pthread_cond_signal(&stack.can_produce);
        pthread_mutex_unlock(&stack.lock);
    }

    return NULL;
}



void apply_effect(Image* original, Image* new_i) {

	int w = original->bmp_header.width;
	int h = original->bmp_header.height;

	*new_i = new_image(w, h, original->bmp_header.bit_per_pixel, original->bmp_header.color_planes);

	for (int y = OFFSET; y < h - OFFSET; y++) {
		for (int x = OFFSET; x < w - OFFSET; x++) {
			Color_e c = { .Red = 0, .Green = 0, .Blue = 0};

			for(int a = 0; a < LENGHT; a++){
				for(int b = 0; b < LENGHT; b++){
					int xn = x + a - OFFSET;
					int yn = y + b - OFFSET;

					Pixel* p = &original->pixel_data[yn][xn];

					c.Red += ((float) p->r) * KERNEL[a][b];
					c.Green += ((float) p->g) * KERNEL[a][b];
					c.Blue += ((float) p->b) * KERNEL[a][b];
				}
			}

			Pixel* dest = &new_i->pixel_data[y][x];
			dest->r = (uint8_t)  (c.Red <= 0 ? 0 : c.Red >= 255 ? 255 : c.Red);
			dest->g = (uint8_t) (c.Green <= 0 ? 0 : c.Green >= 255 ? 255 : c.Green);
			dest->b = (uint8_t) (c.Blue <= 0 ? 0 : c.Blue >= 255 ? 255 : c.Blue);
		}
	}
}

void test_list(void* arg){
    FileListWithQuantity* images = (FileListWithQuantity *) arg;
    for (int i = 0; i < images->quantity; i++){
        printf("int funct %s length: %d  in : %s out : %s\n", images->file_list[i], strlen(images->file_list[i]), images->input_folder, images->output_folder);
    }
}

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
    dr = opendir(dir_path); //open all or present directory
    if (dr) {
        file_quantity = get_file_quantity(dr);
        file_list = get_file_list(dr, file_quantity);
        closedir(dr); //close all directory
    }

    FileListWithQuantity test;
    test.quantity = file_quantity;
    test.file_list = file_list;
    return test;
}

int main(int argc, char** argv) {
    FileListWithQuantity images = get_images("./in");
    images.input_folder = "./in";
    images.output_folder = "./out";
//    test_list(&images);


    pthread_t threads_id[5];
    stack_init();
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    for(int i = 0; i < 4; i++) {
        pthread_create(&threads_id[i], &attr, producer, &images);
    }
    pthread_create(&threads_id[4], NULL, consumer, &images.quantity);
    pthread_join(threads_id[4] ,NULL);

    free(images.file_list);
    return 0;
}
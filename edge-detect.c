/*
	//gcc edge-detect.c bitmap.c -O2 -ftree-vectorize -fopt-info -mavx2 -fopt-info-vec-all
	//UTILISER UNIQUEMENT DES BMP 24bits
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "bitmap.h"
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <ftw.h>
#include "effects.h"
#include "utils.h"


#define STACK_MAX 10


typedef struct stack_t {
    ImageWithPath data[STACK_MAX];
    int count;
    int max;
    pthread_mutex_t lock;
    pthread_cond_t can_consume;
    pthread_cond_t can_produce;
} Stack;

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
    pthread_t t = pthread_self();

    int i = 0;
    while (true) {
        if(stack.count < stack.max && i < images->quantity && images->file_list[i] != NULL) {
            pthread_mutex_lock(&stack.lock);
                char *test = images->file_list[i];
                images->file_list[i] = NULL;
            pthread_mutex_unlock(&stack.lock);

            char *image_path = NULL;
            image_path = calloc((strlen(test) + strlen(images->input_folder) + 2), sizeof(char));
            strcat(image_path, images->input_folder);
            strcat(image_path, "/");
            strcat(image_path,test);
            fprintf(stdout, "<-Producer %08x: Input image %s\n", t, image_path);
            Image img = open_bitmap(image_path);
            free(image_path);

            Image new_i;
            apply_effect(&img, &new_i);

            char *image_out_path = NULL;
            image_out_path = calloc((strlen(test) + strlen(images->output_folder) + 2), sizeof(char));
            strcat(image_out_path, images->output_folder);
            strcat(image_out_path, "/");
            strcat(image_out_path,test);

            fprintf(stdout, "<-Producer %08x: image %s converted\n", t, image_out_path);

            pthread_mutex_lock(&stack.lock);
                stack.data[stack.count].image = &new_i;
                stack.data[stack.count].path = image_out_path;
                stack.count++;
            pthread_mutex_unlock(&stack.lock);
            i++;
            pthread_cond_signal(&stack.can_consume);
        }
        else if (images->file_list[i] == NULL){
            i++;
        }
        else {
            while(stack.count >= stack.max) {
                pthread_cond_wait(&stack.can_produce, &stack.lock);
            }
        }
    }

    return NULL;
}


void* consumer(void* arg);
void* consumer(void* arg) {

    int total = 0;
    int *image_quantity = (int *) arg;
    pthread_t t = pthread_self();
    fprintf(stdout, "->Consumer %08x: %d images to write out \n",t , *image_quantity);
    while(true) {
        pthread_mutex_lock(&stack.lock);
        while(stack.count == 0) {
            fprintf(stdout, "->Consumer %08x: Waiting\n", t);
            pthread_cond_wait(&stack.can_consume, &stack.lock);
        }

        stack.count--;
        save_bitmap(*(stack.data[stack.count].image), stack.data[stack.count].path);
        total++;
        free(stack.data[stack.count].path);
        fprintf(stdout, "->Consumer %08x: %d/%d \n", t, total, *image_quantity);
        if(total >= *image_quantity) {
            fprintf(stdout, "->Consumer %08x: All images converted\n", t);
            break;
        }
        pthread_cond_signal(&stack.can_produce);
        pthread_mutex_unlock(&stack.lock);
    }

    return NULL;
}

int main(int argc, char** argv) {
    FileListWithQuantity images = get_images("./in");
    images.input_folder = "./in";
    images.output_folder = "./out";

    fprintf(stdout, "Flushing output directory\n");
    flush_dir("./out");

    pthread_t threads_id[8];
    stack_init();
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    for(int i = 0; i < 7; i++) {
        pthread_create(&threads_id[i], &attr, producer, &images);
    }
    pthread_create(&threads_id[7], NULL, consumer, &images.quantity);
    pthread_join(threads_id[7] ,NULL);

    free(images.file_list);
    return 0;
}
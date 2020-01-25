/*
	//gcc edge-detect.c bitmap.c -O2 -ftree-vectorize -fopt-info -mavx2 -fopt-info-vec-all
	//UTILISER UNIQUEMENT DES BMP 24bits
*/
#include "core.h"

static Stack stack;

void stack_init() {
    pthread_cond_init(&stack.can_produce, NULL);
    pthread_cond_init(&stack.can_consume, NULL);
    pthread_mutex_init(&stack.lock, NULL);
    stack.max = STACK_MAX;
    stack.count = 0;
}

void* producer(void* arg) {

    FileListWithQuantity* images = (FileListWithQuantity *) arg;
    pthread_t t = pthread_self();

    int i = 0;
    while (true) {
        if(stack.count < stack.max && i < images->quantity && images->file_list[i] != NULL) {
            pthread_mutex_lock(&stack.lock);
                char *image_name = images->file_list[i];
                images->file_list[i] = NULL;
            pthread_mutex_unlock(&stack.lock);

            char *image_in_path = NULL;
            image_in_path = calloc((strlen(image_name) + strlen(images->input_folder) + 2), sizeof(char));
            strcat(image_in_path, images->input_folder);
            strcat(image_in_path, "/");
            strcat(image_in_path,image_name);
            fprintf(stdout, "<- Producer - %08x -> Converting image %s\n", t, image_in_path);
            Image img = open_bitmap(image_in_path);
            free(image_in_path);
            image_in_path = NULL;

            Image new_i;
            apply_effect(&img, &new_i, images->algorithm);
            destroy_image(&img);
            char *image_out_path = NULL;
            image_out_path = calloc((strlen(image_name) + strlen(images->output_folder) + 2), sizeof(char));
            strcat(image_out_path, images->output_folder);
            strcat(image_out_path, "/");
            strcat(image_out_path,image_name);

            fprintf(stdout, "<- Producer - %08x -> Image %s converted\n", t, image_out_path);

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
        else if (i > images->quantity){
            fprintf(stdout, "<- Producer - %08x -> All images consumed\n",t);
//            return NULL;
//            pthread_cond_wait();
        }
        else {
            while(stack.count >= stack.max) {
                fprintf(stdout, "<- Producer - %08x -> Waiting to produce\n");
                pthread_cond_wait(&stack.can_produce, &stack.lock);
            }
        }
    }

    return NULL;
}

void* consumer(void* arg) {

    int total = 0;
    int *image_quantity = (int *) arg;
    pthread_t t = pthread_self();
    fprintf(stdout, "-> Consumer - %08x <- %d images to write out \n",t , *image_quantity);
    while(true) {
        pthread_mutex_lock(&stack.lock);
            while(stack.count == 0) {
                fprintf(stdout, "-> Consumer - %08x <- Waiting\n", t);
                pthread_cond_wait(&stack.can_consume, &stack.lock);
            }
            stack.count--;
            fprintf(stdout, "-> Consumer - %08x <- Writing %s\n", t, stack.data[stack.count].path);
            save_bitmap(*(stack.data[stack.count].image), stack.data[stack.count].path);
            total++;
            fprintf(stdout, "-> Consumer - %08x <- %d/%d %s wrote\n", t, total, *image_quantity, stack.data[stack.count].path);
            destroy_image(stack.data[stack.count].image);
            free(stack.data[stack.count].path);
            stack.data[stack.count].path = NULL;
            if(total >= *image_quantity) {
                fprintf(stdout, "-> Consumer - %08x <- All images converted\n", t);
                break;
            }
            pthread_cond_signal(&stack.can_produce);
        pthread_mutex_unlock(&stack.lock);
    }

    return NULL;
}

int core(char *input_folder, char *output_folder, int producer_qty, char *algorithm) {
    FileListWithQuantity images = get_images(input_folder);
    if(producer_qty >= images.quantity){
        fprintf( stderr, "You should have less producer than images %d/%d\n", producer_qty, images.quantity);
        return EXIT_FAILURE;
    }
    if(images.quantity == 0){
        fprintf( stderr, "The input folder is empty\n");
        return EXIT_FAILURE;
    }
    images.input_folder = input_folder;
    images.output_folder = output_folder;
    images.algorithm = algorithm;

    fprintf(stdout, "Flushing output directory\n");
    flush_dir(output_folder);

    pthread_t threads_id[producer_qty + 1];
    stack_init();
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    for(int i = 0; i < producer_qty; i++) {
        pthread_create(&threads_id[i], &attr, producer, &images);
    }
    pthread_create(&threads_id[producer_qty], NULL, consumer, &images.quantity);
    pthread_join(threads_id[producer_qty] ,NULL);

    free(images.file_list);
    return EXIT_SUCCESS;
}
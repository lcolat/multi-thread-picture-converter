//
// Created by Loïc Colat on 24/01/2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "bitmap.h"
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
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

void* producer(void* arg);
void* consumer(void* arg);
int core(char *input_folder, char *output_folder, int producer_qty, char *algorithm);

//
// Created by Loïc Colat on 24/01/2020.
//

#include <string.h>
#include "bitmap.h"

#define DIM 3
#define LENGHT DIM
#define OFFSET DIM /2

typedef struct Color_t {
    float Red;
    float Green;
    float Blue;
} Color_e;

void fill_conv_matrix(char *algorithm, float matrix[DIM][DIM]);
void inline apply_convolution(Color_e* c, int a, int b, int x, int y, Image* img, char *algorithm) __attribute__((always_inline));
void apply_effect(Image* original, Image* new_i, char *algorithm);

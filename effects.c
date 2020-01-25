//
// Created by Loïc Colat on 24/01/2020.
//

#include "effects.h"

void fill_conv_matrix(char *algorithm,float matrix[DIM][DIM]) {
    if (strcmp(algorithm, "box_blur") == 0) {
        float box_blur[DIM][DIM] = {{1, 1, 1},
                                    {1, 1, 1},
                                    {1, 1, 1}};

        for (int i = 0; i < DIM; i++) {
            for (int j = 0; j < DIM; j++)
                matrix[i][j] = box_blur[i][j] * (1.0/9.0);
        }
    } else if (strcmp(algorithm, "edge_detect") == 0) {
        float edge_detect[DIM][DIM] = {{-1, -1, -1},
                                       {-1, 8,  -1},
                                       {-1, -1, -1}};
        for (int i = 0; i < DIM; i++) {
            for (int j = 0; j < DIM; j++)
                matrix[i][j] = edge_detect[i][j];
        }
    } else if (strcmp(algorithm, "sharpen") == 0) {
        float sharpen[DIM][DIM] = {{0,  -1, 0},
                                   {-1, 5,  -1},
                                   {0,  -1, 0}};
        for (int i = 0; i < DIM; i++) {
            for (int j = 0; j < DIM; j++)
                matrix[i][j] = sharpen[i][j];
        }

    }

}

void apply_convolution(Color_e* restrict c, int a, int b, int x, int y, Image* restrict img, char *algorithm) {
    int xn = x + a - OFFSET;
    int yn = y + b - OFFSET;

    float conv_matrix[DIM][DIM];
    fill_conv_matrix(algorithm, conv_matrix);

    Pixel* p = &img->pixel_data[yn][xn];

    c->Red += ((float) p->r) * conv_matrix[a][b];
    c->Green += ((float) p->g) * conv_matrix[a][b];
    c->Blue += ((float) p->b) * conv_matrix[a][b];
}

void apply_effect(Image *original, Image *new_i, char *algorithm) {

    int w = original->bmp_header.width;
    int h = original->bmp_header.height;


    *new_i = new_image(w, h, original->bmp_header.bit_per_pixel, original->bmp_header.color_planes);

    for (int y = OFFSET; y < h - OFFSET; y++) {
        for (int x = OFFSET; x < w - OFFSET; x++) {
            Color_e c = { .Red = 0, .Green = 0, .Blue = 0};

            apply_convolution(&c, 0, 0, x, y, original, algorithm);
            apply_convolution(&c, 0, 1, x, y, original, algorithm);
            apply_convolution(&c, 0, 2, x, y, original, algorithm);

            apply_convolution(&c, 1, 0, x, y, original, algorithm);
            apply_convolution(&c, 1, 1, x, y, original, algorithm);
            apply_convolution(&c, 1, 2, x, y, original, algorithm);

            apply_convolution(&c, 2, 0, x, y, original, algorithm);
            apply_convolution(&c, 2, 1, x, y, original, algorithm);
            apply_convolution(&c, 2, 2, x, y, original, algorithm);

            Pixel* dest = &new_i->pixel_data[y][x];
            dest->r = (uint8_t)  (c.Red <= 0 ? 0 : c.Red >= 255 ? 255 : c.Red);
            dest->g = (uint8_t) (c.Green <= 0 ? 0 : c.Green >= 255 ? 255 : c.Green);
            dest->b = (uint8_t) (c.Blue <= 0 ? 0 : c.Blue >= 255 ? 255 : c.Blue);
        }
    }
}
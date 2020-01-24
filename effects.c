//
// Created by Loïc Colat on 24/01/2020.
//

#include "effects.h"

const float EDGE_DETECT[DIM][DIM] = {{-1, -1,-1},
                              {-1,8,-1},
                              {-1,-1,-1}};
//const float SHARPEN[DIM][DIM] = {{0, -1,0},
//                                {-1,5,-1},
//                                {0,-1,0}};

//const float BOX[DIM][DIM] = 1/9{{1,1,1},
//                                {1,1,1},
//                                {1,1,1}};

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

                    c.Red += ((float) p->r) * EDGE_DETECT[a][b];
                    c.Green += ((float) p->g) * EDGE_DETECT[a][b];
                    c.Blue += ((float) p->b) * EDGE_DETECT[a][b];
                }
            }

            Pixel* dest = &new_i->pixel_data[y][x];
            dest->r = (uint8_t)  (c.Red <= 0 ? 0 : c.Red >= 255 ? 255 : c.Red);
            dest->g = (uint8_t) (c.Green <= 0 ? 0 : c.Green >= 255 ? 255 : c.Green);
            dest->b = (uint8_t) (c.Blue <= 0 ? 0 : c.Blue >= 255 ? 255 : c.Blue);
        }
    }
}
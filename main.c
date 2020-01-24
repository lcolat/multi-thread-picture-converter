//
// Created by Loïc Colat on 21/01/2020.
//
#include "main.h"


int main (int argc, char *argv[])
{
    if(verify_arg_qty(argc) == EXIT_FAILURE){
        return EXIT_FAILURE;
    }

    int producer_qty = atoi(argv[3]);

    if(producer_qty == 0){
        fprintf( stderr, "Not enough producer\n");
        return EXIT_FAILURE;
    }

    if(verify_algorithm(argv[4]) == EXIT_FAILURE){
        return EXIT_FAILURE;
    }

    return core(argv[1], argv[2], producer_qty, argv[4]);
}
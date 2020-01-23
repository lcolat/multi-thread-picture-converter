//
// Created by Loïc Colat on 21/01/2020.
//
#include <stdio.h>
#include <stdlib.h>
#include "main.h"


int main (int argc, char *argv[])
{
    char executionExample[] = "./apply-effect \"<inputFolder>\" \"<outputFolder>\" <threadQuantity> <algorithm>\n"
                              "Example :\n"
                              "./apply-effect \"./in/\" \"./out/\" 3 boxblur\n";
    if(argc < 5){
        fprintf( stderr, "Too few arguments, respect this format : \n%s", executionExample);
        return EXIT_FAILURE;
    }else if(argc > 5){
        fprintf( stderr, "Too much arguments, respect this format : \n%s", executionExample);
        return EXIT_FAILURE;
    }



    return EXIT_SUCCESS;
}
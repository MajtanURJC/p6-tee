#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


int 
main(int argc, char *argv[]) {
    
    argc--;
    argv++;

    if (argc != 1) {
        fprintf(stderr,"Numero de parametros incorrecto");
        exit(1);
    }

    pid = fork();

    switch (pid) {
            case -1:
                err(EXIT_FAILURE, "fork failed!");
                break;

            case 0:
                execl("/usr/bin/gunzip", "gunzip", argv[0], NULL);
                break;

            default:
}
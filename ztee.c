#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {

    argv ++;
    argc --;

    int pipe_fd[2];
    int pid1;
    int pid2;
    int fd;
    char buffer[BUFFER_SIZE];
    int n;
    int sts;
    char *nombre_fichero;
    int i;

    if (argc != 1) {
        fprintf(stderr, "Uso: %s <archivo_salida>\n", argv[0]);
        exit(1);
    }

    nombre_fichero = argv[0];

    if (pipe(pipe_fd) == -1) {
        fprintf(stderr, "Error al crear el pipe\n");
        exit(1);
    }

    pid1 = fork();
    switch (pid1) {
        case -1:
            fprintf(stderr, "Error en fork 1\n");
            exit(1);
        
        case 0:
            close(pipe_fd[0]);

            if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
                fprintf(stderr, "Error en dup2 del hijo 1\n");
                exit(1);
            }

            close(pipe_fd[1]);

            execl("/usr/bin/gunzip", "gunzip", "-c", NULL);
            fprintf(stderr, "Error al ejecutar gunzip\n");
            exit(1);
    }

    pid2 = fork();
    switch (pid2) {
        case -1:
            fprintf(stderr, "Error en fork 2\n");
            exit(1);
        case 0:
            close(pipe_fd[1]);

            fd = open(nombre_fichero, O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if (fd == -1) {
                fprintf(stderr, "Error al abrir el archivo de salida\n");
                close(pipe_fd[0]);
                exit(1);
            }

            while ((n = read(pipe_fd[0], buffer, BUFFER_SIZE)) > 0) {
                if (write(STDOUT_FILENO, buffer, n) != n) {
                    fprintf(stderr, "Error al escribir en stdout\n");
                    close(fd);
                    close(pipe_fd[0]);
                    exit(1);
                }

                if (write(fd, buffer, n) != n) {
                    fprintf(stderr, "Error al escribir en el archivo\n");
                    close(fd);
                    close(pipe_fd[0]);
                    exit(1);
                }

            }

            if (n < 0) {
                fprintf(stderr, "Error al leer del pipe\n");
                close(fd);
                close(pipe_fd[0]);
                exit(1);
            }

            close(fd);
            close(pipe_fd[0]);
            exit(0);
    }

    close(pipe_fd[0]);
    close(pipe_fd[1]);

    for (i = 0; i < 2; i++) {
        if (wait(&sts) == -1) {
            fprintf(stderr, "Error al esperar a un hijo\n");
            exit(1);
        }
        if (!WIFEXITED(sts) || WEXITSTATUS(sts) != 0) {
            fprintf(stderr, "Un hijo terminó con error\n");
            exit(1);
        }
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <err.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

enum { Bsize = 1024 };

void
child(int *fd)
{
    close(fd[1]); // cerrar escritura
    if (dup2(fd[0], STDIN_FILENO) < 0) {
        err(EXIT_FAILURE, "cannot dup pipe to stdin");
    }
    close(fd[0]);

    execl("/usr/bin/gunzip", "gunzip", "-c", NULL);
    err(EXIT_FAILURE, "cannot exec gunzip");
}

int
main(int argc, char *argv[])
{
    int fd[2];
    char buf[Bsize];
    int gzfile;
    ssize_t n;
    int status;

    if (argc != 2) {
        err(EXIT_FAILURE, "usage: %s <file.gz>", argv[0]);
    }

    if (pipe(fd) < 0) {
        err(EXIT_FAILURE, "cannot make a pipe\n");
    }

    switch (fork()) {
    case -1:
        err(EXIT_FAILURE, "cannot fork\n");
    case 0:
        child(fd);
        exit(EXIT_SUCCESS);
    default:
        close(fd[0]); 

        gzfile = open(argv[1], O_RDONLY);
        if (gzfile < 0) {
            err(EXIT_FAILURE, "cannot open input file\n");
        }

        while ((n = read(gzfile, buf, sizeof(buf))) > 0) {
            if (write(fd[1], buf, n) != n) {
                err(EXIT_FAILURE, "error writing to pipe\n");
            }
        }

        if (n < 0) {
            err(EXIT_FAILURE, "error reading input file");
        }

        close(gzfile);
        close(fd[1]); 

        if (wait(&status) == -1) {
            err(EXIT_FAILURE, "wait failed");
        }

        if (WIFEXITED(status) != 0) {

                fprintf(stderr, "child exited succesfully\n");

        } else {

            fprintf(stderr, "child terminated wrongly\n");
    
        }

        exit(EXIT_SUCCESS);
    }
}


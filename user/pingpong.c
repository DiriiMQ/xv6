#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    if(argc != 1){
        fprintf(2, "usage: pingpong\n");
        exit(1);
    }
    int n = 42;
    int p[2];
    pipe(p);
    if(fork() == 0){ // child
        close(p[1]); // close write end
        read(p[0], &n, sizeof(n)); // read from read end
        fprintf(1, "%d: received ping\n", getpid());
        close(p[0]); // close read end
        write(p[1], &n, sizeof(n)); // send pong
        close(p[1]);
        exit(0);
    }
    close(p[0]);
    write(p[1], &n, sizeof(n)); // send ping
    close(p[1]);
    wait(0); // wait for child to exit
    read(p[0], &n, sizeof(n));
    fprintf(1, "%d: received pong\n", getpid());
    close(p[0]);
    exit(0);
}
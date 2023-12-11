#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    if(argc > 1) {
        fprintf(2, "Usage: too many arguments...\n");
        exit(1);
    }

    int p[2];
    
    if(pipe(p) == -1) {
        fprintf(2, "pipe build error.");
        exit(1);
    }

    if(fork() == 0) {
        // child. print "<pid>: received ping"
        int pid;
        pid = getpid();
        fprintf(1, "%d: received ping\n", pid);

        // write a byte on the pipe to the parent, exit.
        char byte = 'A';

        if(write(p[1], &byte, 1) == -1) {
            fprintf(2, "write error\n");
            exit(1);
        }
        
    } else {
        // parent. print "<pid>: received pong"
        wait(0);
        int pid;
        pid = getpid();
        fprintf(1, "%d: received pong\n", pid);
        // read from pipe
        char buf[1];
        if(read(p[0], buf, 1) == -1) {
            fprintf(2, "read error\n");
        }
        fprintf(1, "%s\n", buf);
        close(p[0]);
        close(p[1]);
        
    }
    exit(0);
}
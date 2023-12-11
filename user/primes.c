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
    int num_child_processes = 35;

    int pipes[num_child_processes][2];
    int buffer[2][35];

    fprintf(1, "prime %d\n", 2);
    if (pipe(pipes[0]) == -1) {
        fprintf(2, "pipe create error.\n");
        exit(1);
    }

    int idx = 0;
    if (fork() == 0) {
        int tmp_child;
        int tmp_parent;
        int tmp_idx;

        close(pipes[0][1]);
        while(read(pipes[0][0], &tmp_child, 1) != 0) {
            buffer[0][idx] = tmp_child;
            idx++;
        }
        
        close(pipes[0][0]);
        // enter 2-35 child process
        for (int i=1; i<35; i++) {
            if (pipe(pipes[i]) == -1) {
                fprintf(2, "pipe create error.\n");
                exit(1);
            }
            tmp_idx=idx;
            idx = 0;
            int pid = fork();
            if (pid != 0) {
                close(pipes[i][0]);
                // write the condition-matched-number into pipei from buffer[0/1][idx];
                for (int j=0; j<tmp_idx; j++) {
                    tmp_parent = buffer[(i-1)%2][j];
                    if (tmp_parent % (i+1) != 0) {
                        write(pipes[i][1], &tmp_parent, 1);
                    }
                }
                close(pipes[i][1]);
                wait(0);
                exit(0);
            } else if (pid == 0) {
                close(pipes[i][1]);
                // read from i into buffer[1/0][35];
                if (read(pipes[i][0], &tmp_child, 1) == 0) {
                    exit(0);
                } else {
                    fprintf(1, "prime %d\n", tmp_child);
                }
                while(read(pipes[i][0], &tmp_child, 1) != 0) {
                    buffer[i%2][idx] = tmp_child;
                    idx++;
                }
                close(pipes[i][0]);
            }
        }

    } else {
        // the first process writes number 3-35 to the right
        for (int j=3; j<36; j++) {
            write(pipes[0][1], &j, 1);
        }
        close(pipes[0][1]);
    }
    wait(0);
    exit(0);
}
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

char *subArgv[MAXARG];
char *realArgv[MAXARG];
int subIdx = 0;

void strcat(char arr1[], char arr2[])
{
    int i = 0;
    int j = 0;

    while (arr1[i] != '\0')
    {
        i++;
    }

    while(arr2[j] != '\0')
    {
        arr1[i] = arr2[j];
        i++;
        j++;
    }
    
    arr1[i] = '\0';
}

void splitString(char *str)
{
    char *p;
    p = str;

    while (*p != '\0' && *p != '\n')
    {
        char *word = (char *)malloc(DIRSIZ + 1);
        char *p2;
        p2 = word;
        while (*p != ' ' && *p != '\0' && *p != '\n')
        {
            *p2 = *p;
            p++;
            p2++;
        }
        *p2 = '\0';
        subArgv[subIdx] = word;
        subIdx++;
        if (*p != '\0' || *p != '\n')
        {
            p++;
        }
    }
}

int main(int argc, char *argv[])
{
    if (fork() == 0) // child
    {
        char buf[512], *p;
        p = buf;
        int idx = -1;

        while (1)
        {
            if (read(0, p, 1) == -1)
            {
                fprintf(2, "read error.");
            }
            p++;
            idx++;
            if (buf[idx] == '\0')
            {
                break;
            }
        }

        splitString(buf);

        int realArgc = 0;
        for(int i=1; i<argc; i++)
        {
            realArgv[i-1] = argv[i];
            realArgc++;
        }

        for(int i=0; i<subIdx; i++)
        {
            realArgv[realArgc + i] = subArgv[i];
        }

        char commandDir[DIRSIZ] = "/";
        strcat(commandDir, realArgv[0]);

        exec(commandDir, realArgv);

        exit(0);
    }
    else // parent
    {
        wait(0);
    }

    exit(0);
}
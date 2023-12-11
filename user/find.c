#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include <stddef.h>

void
find(char *dir, char *filenameOrg)
{

    // determine ent->next type, file? dir?
    // file -> compare; dir -> put into queue
    char buf[512], *p;
    strcpy(buf, dir);

    p = buf + strlen(dir);
    *p++ = '/';

    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(dir, O_RDONLY)) < 0){
        fprintf(2, "find: cannot open %s\n", dir);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", dir);
        close(fd);
        return;
    }
    
    if(st.type != T_DIR){
        fprintf(2, "find: %s is not a dir\n", dir);
        close(fd);
        return;
    }

    while(read(fd, &de, sizeof(de))  == sizeof(de)){
        if(de.inum == 0)
            continue;

        char *name = de.name;
        if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            continue;

        memmove(p, name, DIRSIZ);

        if(stat(buf, &st) < 0){
            printf("find: cannot stat %s\n", name);
            continue;
        }

        if(st.type == T_DIR){
            find(buf, filenameOrg);
        } else if (strcmp(name, filenameOrg) == 0){
            printf("%s\n", buf);
        }
    }
    close(fd);
}

int
main(int argc, char *argv[])
{
    if(argc != 3) {
        fprintf(2, "Usage: name your args\n");
        exit(1);
    }


    find(argv[1], argv[2]);
    exit(0);
}
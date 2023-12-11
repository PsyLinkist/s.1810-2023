#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

int main(int argc, char *argv[])
{
    char *str = "/";
    char *p;
    p = str + strlen(str);

    char *buf[2];
    buf[0] = "hello\n";
    
    memmove(p, buf[0], 6);

    printf("buf[0]: %s\n", buf[0]);
    exit(0);
}
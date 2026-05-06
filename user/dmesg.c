#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

#define DMESG_USER_BUFSIZE 20000

int main(int argc, char *argv[])
{
    char buf[DMESG_USER_BUFSIZE];

    if (dmesg(buf, sizeof(buf)) < 0)
    {
        fprintf(2, "dmesg failed\n");
        exit(1);
    }

    printf("%s", buf);

    exit(0);
}
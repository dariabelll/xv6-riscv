#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

#define DMESG_USER_BUFSIZE 20000

static char buf[DMESG_USER_BUFSIZE];

int main(int argc, char *argv[])
{
    if (dmesg(buf, sizeof(buf)) < 0)
    {
        fprintf(2, "dmesg failed\n");
        exit(1);
    }

    printf("%s", buf);

    exit(0);
}
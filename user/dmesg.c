#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "kernel/riscv.h"
#include "user/user.h"

#define DMESG_BUF_SIZE (DMESG_PAGECOUNT * PGSIZE + 1)

int main(int argc, char *argv[])
{
    char buf[DMESG_BUF_SIZE];

    if (dmesg(buf, sizeof(buf)) < 0)
    {
        fprintf(2, "dmesg failed\n");
        exit(1);
    }

    printf("%s", buf);

    exit(0);
}
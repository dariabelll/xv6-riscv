#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int main(int argc, char **argv)
{

    if(argc != 3)
    {
        fprintf(2, "invalid count of args\n");
        exit(1);
    }

    int n = atoi(argv[1]);
    if(n < 0)
    {
        fprintf(2, "invalid count of bytes\n");
        exit(1);
    }

    int fd = open(argv[2], O_RDONLY);
    if(fd < 0){
        fprintf(2, "failed openning %s\n", argv[2]);
        exit(1);
    }

    char hex[] = "0123456789ABCDEF";

    uint8 buf[n];
    int r = read(fd, &buf, n);
    if (r < 0)
    {
        fprintf(2, "failed reading %s\n", argv[2]);
        close(fd);
        exit(1);
    }

    for (int i = 0; i < r; ++i)
    {
        printf("%c%c ", hex[buf[i] >> 4], hex[buf[i] & 15]);
    }
        
    printf("\n");

    if (close(fd) < 0)
    {
        fprintf(2, "failed closing %s\n", argv[2]);
        exit(1);
    }
    exit(0);
}

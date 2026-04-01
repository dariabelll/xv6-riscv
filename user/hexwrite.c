#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int hex_to_uint(char c)
{
    if ('0' <= c && c <= '9')
    {
        return c - '0';
    }
    if ('A' <= c && c <= 'F')
    {
        return c - 'A' + 10;
    }
    if ('a' <= c && c <= 'f')
    {
        return c - 'a' + 10;
    }

    return -1;
}

int main(int argc, char **argv)
{

    if(argc != 3)
    {
        fprintf(2, "invalid count of args\n");
        exit(1);
    }

    int fd = open(argv[2], O_WRONLY);
    if(fd < 0){
        fprintf(2, "failed openning %s\n", argv[2]);
        exit(1);
    }

    char *arg = argv[1];
    int arg_len = strlen(arg);
    if (arg_len % 2 == 1)
    {
        fprintf(2, "invalid text %s\n", arg);
        close(fd);
        exit(1);
    }

    int n = arg_len / 2;
    uint8 buf[n];

    for (int i = 0; i < n; ++i)
    {
        int a, b;
        a = hex_to_uint(arg[2*i]);
        if (a < 0)
        {
            fprintf(2, "invalid text %s\n", arg);
            close(fd);
            exit(1);
        }

        b = hex_to_uint(arg[2*i+1]);
        if (b < 0)
        {
            fprintf(2, "invalid text %s\n", arg);
            close(fd);
            exit(1);
        }

        buf[i] = a * 16 + b;
    }
    int w = write(fd, buf, n);

    if (w != n)
    {
        fprintf(2, "write error %s\n", argv[2]);
        close(fd);
        exit(1);
    }

    if (close(fd) < 0)
    {
        fprintf(2, "failed closing %s\n", argv[2]);
        exit(1);
    }
    exit(0);
}

#include "kernel/types.h"
#include "user/user.h"

#define BUFFER_SIZE 256

static int write_from_buffer(int fd, const char *buff, int size)
{
    int offset = 0;
    while (offset != size) 
    {
        int w_st = write(fd, buff + offset, size - offset);
        if (w_st < 0) return -1;

        offset += w_st;
    }

    return 0;
}


int main(int argc, char **argv)
{
    int pipefd[2];

    if (pipe(pipefd) < 0)
    {
        fprintf(2, "pipe failed\n");
        exit(1);
    }

    int pid = fork();

    if (pid < 0)
    {
        close(pipefd[0]);
        close(pipefd[1]);
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0)
    {
        if (close(pipefd[1]) < 0)
        {
            fprintf(2, "close write-end failed\n");
            exit(1);
        }

        close(0);

        if (dup(pipefd[0]) < 0)
        {
            fprintf(2, "dup failed\n");
            exit(1);
        }

        if (close(pipefd[0]) < 0)
        {
            fprintf(2, "close read-end failed\n");
            exit(1);
        }

        char *wcargv[] = {"/wc", 0};

        exec("/wc", wcargv);

        fprintf(2, "exec failed\n");
        exit(1);
    }

    if (close(pipefd[0]) < 0)
    {
        fprintf(2, "close read-end failed\n");
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    int buff_used = 0;

    for (int i = 1; i < argc; ++i)
    {
        int string_len = strlen(argv[i]);

        if (string_len + 1 > BUFFER_SIZE)
        {
            if (write_from_buffer(pipefd[1], buffer, buff_used) < 0)
            {
                close(pipefd[1]);
                fprintf(2, "write failed\n");
                exit(1);
            }
            if (write_from_buffer(pipefd[1], argv[i], string_len) < 0) 
            {
                close(pipefd[1]);
                fprintf(2, "write failed\n");
                exit(1);
            }
            if (write_from_buffer(pipefd[1], "\n", 1) < 0)
            {
                close(pipefd[1]);
                fprintf(2, "write failed\n");
                exit(1);
            }
            buff_used = 0;

            continue;
        }

        if (string_len + buff_used + 1 > BUFFER_SIZE)
        {
            if (write_from_buffer(pipefd[1], buffer, buff_used) < 0)
            {
                close(pipefd[1]);
                fprintf(2, "write failed\n");
                exit(1);
            }
            buff_used = 0;
        }

        memcpy(buffer + buff_used, argv[i], string_len);
        buff_used += string_len + 1;
        buffer[buff_used - 1] = '\n';
    }

    if (write_from_buffer(pipefd[1], buffer, buff_used) < 0)
    {
        close(pipefd[1]);
        fprintf(2, "write failed\n");
        exit(1);
    }

    if (close(pipefd[1]) < 0)
    {
        fprintf(2, "close write-end failed\n");
        exit(1);
    }

    int status;
    if (wait(&status) < 0)
    {
        fprintf(2, "wait failed\n");
        exit(1);
    }

    exit(0);

}
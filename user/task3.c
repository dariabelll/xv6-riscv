#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 16384

static void write_from_buffer(int fd, const char *buff, int size)
{
    int offset = 0;
    while (offset < size)
    {
        ssize_t w_st = write(fd, buff + offset, (size_t)(size - offset));
        if (w_st < 0)
        {
            if (errno == EINTR)
                continue;

            fprintf(stderr, "write failed: %s\n", strerror(errno));
            exit(1);
        }

        offset += (int)w_st;
    }
}

int main(int argc, char **argv)
{
    int pipefd[2];

    if (pipe(pipefd) < 0)
    {
        fprintf(stderr, "pipe failed: %s\n", strerror(errno));
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        close(pipefd[0]);
        close(pipefd[1]);
        fprintf(stderr, "fork failed: %s\n", strerror(errno));
        exit(1);
    }

    if (pid == 0)
    {
        if (close(pipefd[1]) < 0)
        {
            fprintf(stderr, "close write-end failed: %s\n", strerror(errno));
            _exit(1);
        }

        char buffer[BUFFER_SIZE];

        while (1)
        {
            ssize_t r = read(pipefd[0], buffer, sizeof(buffer));
            if (r < 0)
            {
                if (errno == EINTR)
                    continue;
                    
                fprintf(stderr, "read failed: %s\n", strerror(errno));
                _exit(1);
            }
            if (r == 0)
                break;

            write_from_buffer(1, buffer, (int)r);
        }

        if (close(pipefd[0]) < 0)
        {
            fprintf(stderr, "close read-end failed: %s\n", strerror(errno));
            _exit(1);
        }

        _exit(0);
    }

    if (close(pipefd[0]) < 0)
    {
        fprintf(stderr, "close read-end failed: %s\n", strerror(errno));
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    int buff_used = 0;

    for (int i = 1; i < argc; ++i)
    {
        int string_len = (int)strlen(argv[i]);

        if (string_len + 1 > BUFFER_SIZE)
        {
            write_from_buffer(pipefd[1], buffer, buff_used);
            write_from_buffer(pipefd[1], argv[i], string_len);
            write_from_buffer(pipefd[1], "\n", 1);
            buff_used = 0;
            continue;
        }

        if (string_len + buff_used + 1 > BUFFER_SIZE)
        {
            write_from_buffer(pipefd[1], buffer, buff_used);
            buff_used = 0;
        }

        memmove(buffer + buff_used, argv[i], (size_t)string_len);
        buff_used += string_len + 1;
        buffer[buff_used - 1] = '\n';
    }

    write_from_buffer(pipefd[1], buffer, buff_used);

    if (close(pipefd[1]) < 0)
    {
        fprintf(stderr, "close write-end failed: %s\n", strerror(errno));
        exit(1);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0)
    {
        fprintf(stderr, "waitpid failed: %s\n", strerror(errno));
        exit(1);
    }

    exit(0);
}

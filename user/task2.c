#include "kernel/types.h"
#include "user/user.h"


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

    for (int i = 1; i < argc; ++i)
    {
        int string_len = strlen(argv[i]);
        int offset = 0;
        while (offset != string_len) 
        {
            int w_st = write(pipefd[1], argv[i] + offset, string_len - offset);
            if (w_st < 0)
            {
                close(pipefd[1]);
                fprintf(2, "write failed\n");
                exit(1);
            }

            offset += w_st;

        }

        if (write(pipefd[1], "\n", 1) != 1)
        {
            close(pipefd[1]);
            fprintf(2, "write newline failed\n");
            exit(1);
        }
        
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
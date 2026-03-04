#include "kernel/types.h"
#include "user/user.h"

#define SLEEP_TIMEOUT 100

int main(void)
{
    int status;
    int pid = fork();

    if (pid < 0)
    {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0)
    {
        int p = pause(SLEEP_TIMEOUT);

        if (p < 0)
        {
            fprintf(2, "pause failed\n");
            exit(1);
        }

        exit(1);
    }
    else 
    {
        printf("parent's pid: %d\n", getpid());
        printf("child's pid: %d\n", pid);

        if (kill(pid) < 0)
        {
            fprintf(2, "kill failed\n");
            exit(1);
        }
        
        
        int w = wait(&status);
        if (w < 0)
        {
            fprintf(2, "wait failed\n");
            exit(1);
        }

        printf("wait returned pid %d\n", w);
        printf("child exited with status %d\n", status);

        exit(0);
    }
}
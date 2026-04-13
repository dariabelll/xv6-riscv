#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"



static void test_no_mutex(int argc, char *argv[])
{
    int pid;

    printf("=== test without mutex ===\n");

    pid = fork();
    if(pid < 0)
    {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    for(int i = 1; i < argc; i++)
    {
        for(int j = 0; argv[i][j] != 0; j++)
        {
            printf("%d", getpid());
            printf(": arg %d", i);
            printf(", char '%c'\n", argv[i][j]);
        }
    }

    if(pid == 0) exit(0);

    wait(0);
}

static void test_with_mutex(int argc, char *argv[])
    {
    int pid;
    int mfd;

    printf("=== test with mutex ===\n");

    mfd = mutex();
    if (mfd < 0)
    {
        fprintf(2, "mutex create failed\n");
        exit(1);
    }

    pid = fork();
    if (pid < 0)
    {
        fprintf(2, "fork failed\n");
        close(mfd);
        exit(1);
    }

    for (int i = 1; i < argc; i++)
    {
        for (int j = 0; argv[i][j] != 0; j++)
        {
            if (mutex_lock(mfd) < 0)
            {
                fprintf(2, "mutex_lock failed\n");
                exit(1);
            }

            printf("%d", getpid());
            printf(": arg %d", i);
            printf(", char '%c'\n", argv[i][j]);

            if (mutex_unlock(mfd) < 0)
            {
                fprintf(2, "mutex_unlock failed\n");
                exit(1);
            }
        }
    }

    if (pid == 0)
    {
        close(mfd);
        exit(0);
    } 
    else 
    {
        wait(0);
        close(mfd);
    }
}

int main(int argc, char *argv[])
{
  if(argc < 2)
  {
    fprintf(2, "invalid test args\n");
    exit(1);
  }

  test_no_mutex(argc, argv);
  test_with_mutex(argc, argv);

  exit(0);
}

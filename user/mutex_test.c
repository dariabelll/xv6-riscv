#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static void test_read_write()
{
    printf("===read write test====\n");
    char buff = 'b';
    int m = mutex();
    if (m < 0) 
    {
        fprintf(2, "create a mutex failed\n");
        exit(1);
    }
    
    int r = read(m, &buff, 1);
    if(r == -1) printf("read on mutex: ok\n");
    else printf("read on mutex: fail (returned %d)\n", r);

    r = write(m, &buff, 1);
    if(r == -1) printf("write on mutex: ok\n");
    else printf("write on mutex: fail (returned %d)\n", r);

    r = close(m);
    if(r == 0) printf("close mutex: ok\n");
    else printf("close mutex: fail (returned %d)\n", r);
}

static void test_close_by_owner(void)
{
    int m, pid, r;

    printf("=== close locked mutex by owner ===\n");

    m = mutex();
    if (m < 0)
    {
        fprintf(2, "create a mutex failed\n");
        exit(1);
    }

    pid = fork();
    if (pid < 0)
    {
        fprintf(2, "fork failed\n");
        close(m);
        exit(1);
    }

    if (pid == 0)
    {
        pause(20);

        r = mutex_lock(m);
        if(r == 0) printf("child lock: ok\n");
        else printf("child lock: fail (returned %d)\n", r);

        if (r == 0)
        {
            r = mutex_unlock(m);
            if(r == 0)  printf("child unlock: ok\n");
            else printf("child unlock: fail (returned %d)\n", r);
        }

        r = close(m);
        if(r == 0) printf("child close: ok\n");
        else printf("child close: fail (returned %d)\n", r);

        exit(0);
    }

    r = mutex_lock(m);
    if (r == 0) printf("parent lock: ok\n");
    else printf("parent lock: fail (returned %d)\n", r);

    r = close(m);
    if (r == 0) printf("parent close: ok\n");
    else printf("parent close: fail (returned %d)\n", r);

    wait(0);
}

static void test_close_by_other()
    {
    int m, pid, r;

    printf("=== close locked mutex by other process ===\n");

    m = mutex();
    if (m < 0)
    {
        fprintf(2, "create a mutex failed\n");
        exit(1);
    }

    r = mutex_lock(m);
    if (r == 0) printf("parent lock: ok\n");
    else printf("parent lock: fail (returned %d)\n", r);

    pid = fork();
    if (pid < 0)
    {
        fprintf(2, "fork failed\n");
        close(m);
        exit(1);
    }

    if(pid == 0)
    {
        pause(10);

        r = close(m);
        if(r == 0) printf("child close: ok\n");
        else printf("child close: fail (returned %d)\n", r);

        exit(0);
    }

    pause(20);

    r = mutex_unlock(m);
    if(r == 0) printf("parent unlock after child close: ok\n");
    else printf("parent unlock after child close: fail (returned %d)\n", r);

    r = close(m);
    if(r == 0) printf("parent close: ok\n");
    else printf("parent close: fail (returned %d)\n", r);

    wait(0);
}

static void test_exit_with_locked_mutex(void)
{
    int m, pid, r;

    printf("=== exit with locked mutex ===\n");

    m = mutex();
    if(m < 0)
    {
        fprintf(2, "create a mutex failed\n");
        exit(1);
    }

    pid = fork();
    if(pid < 0)
    {
        fprintf(2, "fork failed\n");
        close(m);
        exit(1);
    }

    if(pid == 0)
    {
        r = mutex_lock(m);
        if(r == 0) printf("child lock: ok\n");
        else printf("child lock: fail (returned %d)\n", r);

        printf("child exits without unlock/close\n");
        exit(0);
    }

    wait(0);

    r = mutex_lock(m);
    if(r == 0) printf("parent lock after child exit: ok\n");
    else printf("parent lock after child exit: fail (returned %d)\n", r);

    if(r == 0)
    {
        r = mutex_unlock(m);
        if(r == 0) printf("parent unlock: ok\n");
        else printf("parent unlock: fail (returned %d)\n", r);
    }

    r = close(m);
    if(r == 0) printf("parent close: ok\n");
    else printf("parent close: fail (returned %d)\n", r);
}

static void test_unlock_by_other(void)
    {
    int m, pid, r;

    printf("=== unlock by other process ===\n");

    m = mutex();
    if(m < 0)
    {
        fprintf(2, "create a mutex failed\n");
        exit(1);
    }

    r = mutex_lock(m);
    if (r == 0) printf("parent lock: ok\n");
    else
    {
        printf("parent lock: fail (returned %d)\n", r);
        close(m);
        exit(1);
    }

    pid = fork();
    if (pid < 0)
    {
        fprintf(2, "fork failed\n");
        close(m);
        exit(1);
    }

    if(pid == 0)
    {
        pause(10);

        r = mutex_unlock(m);
        if (r < 0) printf("child unlock other proc mutex: ok\n");
        else printf("child unlock other process mutex: fail (returned %d)\n", r);

        r = close(m);
        if(r == 0) printf("child close: ok\n");
        else printf("child close: fail (returned %d)\n", r);

        exit(0);
    }

    pause(20);

    r = mutex_unlock(m);
    if (r == 0) printf("parent unlock: ok\n");
    else printf("parent unlock: fail (returned %d)\n", r);

    r = close(m);
    if (r == 0) printf("parent close: ok\n");
    else printf("parent close: fail (returned %d)\n", r);

    wait(0);
}

int
main(void)
{

    test_read_write();
    test_close_by_owner();
    test_close_by_other();
    test_exit_with_locked_mutex();
    test_unlock_by_other();

    exit(0);
}
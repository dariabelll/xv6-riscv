#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"
#include "mutex.h"


int mutexalloc(struct file **f)
{
    struct mutex *m;
    m = 0;
    *f = 0;
    if((*f = filealloc()) == 0)
        goto bad;
    if((m = (struct mutex*)kalloc()) == 0)
    {
        printf("mutexalloc: kalloc failed\n");
        goto bad;
    }

    initsleeplock(&m->lock, "mutex");
    m->owner = 0;

    (*f)->type = FD_MUTEX;
    (*f)->readable = 0;
    (*f)->writable = 0;
    (*f)->mutex = m;

    printf("mutexalloc: %p\n", m);

    return 0;
    
    bad:
    if(m)
        kfree((char*)m);
    if(*f)
        fileclose(*f);
    return -1;
}

void mutexclose(struct mutex *m)
{
    printf("mutexclose: %p\n", m);
    kfree((char*)m);
}

int mutexlock(struct file *f)
{
    if (f == 0 || f->type != FD_MUTEX || f->mutex == 0)
        return -1;

    if (f->mutex->owner == myproc())
        return -1;

    acquiresleep(&f->mutex->lock);
    f->mutex->owner = myproc();

    return 0;
}

int mutexunlock(struct file *f)
{
    if (f == 0 || f->type != FD_MUTEX || f->mutex == 0)
        return -1;

    if (f->mutex->owner != myproc())
        return -1;

    f->mutex->owner = 0;
    releasesleep(&f->mutex->lock);

    return 0;
}
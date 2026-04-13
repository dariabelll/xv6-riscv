#include "types.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "proc.h"
#include "file.h"
#include "defs.h"
#include "mutex.h"

static struct spinlock mutex_id_lock;
static uint next_mutex_id = 1;

void mutexinit(void)
{
    initlock(&mutex_id_lock, "mutex_id");
}

static uint mutexidalloc(void)
{
    uint id;

    acquire(&mutex_id_lock);
    id = next_mutex_id++;
    release(&mutex_id_lock);

    return id;
}

int
mutexalloc(struct file **f)
{
    struct mutex *m;
    m = 0;
    *f = 0;

    if((*f = filealloc()) == 0)
        goto bad;

    if((m = (struct mutex*)kalloc()) == 0)
        goto bad;

    initsleeplock(&m->lock, "mutex");
    m->id = mutexidalloc();

    (*f)->type = FD_MUTEX;
    (*f)->ref = 1;
    (*f)->readable = 0;
    (*f)->writable = 0;
    (*f)->pipe = 0;
    (*f)->ip = 0;
    (*f)->off = 0;
    (*f)->major = 0;
    (*f)->mutex = m;

    printf("mutexalloc: id=%d addr=%p pid=%d\n", m->id, m, myproc()->pid);
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
    if(m == 0)
        return;

    printf("mutexclose: id=%d addr=%p pid=%d\n", m->id, m, myproc() ? myproc()->pid : -1);

    kfree((char*)m);
}

int mutexlock(struct file *f)
{
    if (f == 0 || f->type != FD_MUTEX || f->mutex == 0)
        return -1;

    if(holdingsleep(&f->mutex->lock))
        return -1;

    acquiresleep(&f->mutex->lock);

    printf("mutexlock: id=%d addr=%p pid=%d\n",
            f->mutex->id, f->mutex, myproc()->pid);
    return 0;
}

int mutexunlock(struct file *f)
{
    if (f == 0 || f->type != FD_MUTEX || f->mutex == 0)
        return -1;

    if (!holdingsleep(&f->mutex->lock))
        return -1;

    printf("mutexunlock: id=%d addr=%p pid=%d\n", f->mutex->id, f->mutex, myproc()->pid);

    releasesleep(&f->mutex->lock);

    return 0;
}

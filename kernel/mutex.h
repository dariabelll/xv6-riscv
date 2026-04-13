#ifndef _MUTEX_H_
#define _MUTEX_H_

#include "sleeplock.h"

struct mutex
{
    struct sleeplock lock;
    uint id;
};

int  mutexalloc(struct file **f);
void mutexclose(struct mutex *m);
int  mutexlock(struct file *f);
int  mutexunlock(struct file *f);

#endif
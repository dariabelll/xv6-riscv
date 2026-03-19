#ifndef _PROCINFO_H_
#define _PROCINFO_H_

#define MAX_LEN_PROC_NAME 16

struct procinfo
{
    int pid;
    char name[MAX_LEN_PROC_NAME];
    int state;
    int parent_id;
};

#endif
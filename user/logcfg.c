#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

int main(int argc, char **argv)
{
    int mask = 0;
    int duration = 0;

    if(argc < 2 || argc > 3)
    {
        fprintf(2, "Query must be: logcfg [off|all|interraptions|syscalls|processes|exec] [opt:ticks count]\n");
        exit(1);
    }

    if(strcmp(argv[1], "off") == 0) mask = 0;
    else if(strcmp(argv[1], "all") == 0) mask = LOG_ALL;
    else if(strcmp(argv[1], "intr") == 0) mask = LOG_INTERRUPTS;
    else if(strcmp(argv[1], "sys") == 0) mask = LOG_SYSCALLS;
    else if(strcmp(argv[1], "proc") == 0) mask = LOG_PROCESSES;
    else if(strcmp(argv[1], "exec") == 0) mask = LOG_EXEC;
    else 
    {
        fprintf(2, "unknown log type: %s\n", argv[1]);
        exit(1);
    }

    if (argc == 3) duration = atoi(argv[2]);

    if (logcfg(mask, duration) < 0){
        fprintf(2, "logcfg failed\n");
        exit(1);
    }

    exit(0);
}
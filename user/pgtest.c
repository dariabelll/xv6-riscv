#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"

volatile int g_single = 17;

static void print_info(const char *name, void *buf, int len)
{
    int a  = checkpgflags(buf, len, PTE_A);
    int d  = checkpgflags(buf, len, PTE_D);
    int ad = checkpgflags(buf, len, PTE_A | PTE_D);

    printf("%s: len=%d A=%d D=%d AD=%d", name, len, a, d, ad);

    if (a < 0 || d < 0 || ad < 0) 
    {
        printf(" error\n");
        return;
    }

    if (a == 0 && d == 0 && ad == 0) 
    {
        printf(" clear\n");
        return;
    }

    if (a == 1 && d == 0 && ad == 1)
    {
        printf(" read\n");
        return;
    }

    if (a == 1 && d == 1 && ad == 1) 
    {
        printf(" write\n");
        return;
    }

    printf(" mixed\n");
}

int main(void)
{
    int stack_single = 20;
    int stack_arr[64];
    int i;

    for (i = 0; i < 64; i++) stack_arr[i] = i;

    printf("start\n");
    printpgtable();

    int heap_size = 3 * PGSIZE;
    char *heap = sbrk(heap_size);
    if (heap == (char *)-1) 
    {
        printf("sbrk failed\n");
        exit(1);
    }

    for (i = 0; i < heap_size; i++) heap[i] = (char)(i & 0xff);

    printf("after alloc\n");
    printpgtable();

    if (clearpgflags((void *)&g_single, sizeof(g_single), PTE_A | PTE_D) < 0) printf("clear global failed\n");
    if (clearpgflags((void *)&stack_single, sizeof(stack_single), PTE_A | PTE_D) < 0) printf("clear stack failed\n");
    if (clearpgflags((void *)&stack_arr[17], sizeof(stack_arr[17]), PTE_A | PTE_D) < 0) printf("clear stack[17] failed\n");
    if (clearpgflags((void *)heap, heap_size, PTE_A | PTE_D) < 0) printf("clear heap failed\n");

    printf("after clear\n");
    print_info("global", (void *)&g_single, sizeof(g_single));
    print_info("stack", (void *)&stack_single, sizeof(stack_single));
    print_info("stack[17]", (void *)&stack_arr[17], sizeof(stack_arr[17]));
    print_info("heap[0]", (void *)&heap[0], 1);
    print_info("heap block", (void *)heap, heap_size);
    printpgtable();

    volatile int read_sum = 0;
    read_sum += g_single;
    read_sum += stack_single;
    read_sum += stack_arr[17];
    read_sum += heap[0];
    read_sum += heap[PGSIZE];
    read_sum += heap[2 * PGSIZE];

    printf("after read\n");
    print_info("global", (void *)&g_single, sizeof(g_single));
    print_info("stack", (void *)&stack_single, sizeof(stack_single));
    print_info("stack[17]", (void *)&stack_arr[17], sizeof(stack_arr[17]));
    print_info("heap[0]", (void *)&heap[0], 1);
    print_info("heap block", (void *)heap, heap_size);
    printpgtable();

    g_single += 1;
    stack_single += 1;
    stack_arr[17] += 1;
    heap[0] += 1;
    heap[PGSIZE] += 1;
    heap[2 * PGSIZE] += 1;

    printf("after write\n");
    print_info("global", (void *)&g_single, sizeof(g_single));
    print_info("stack", (void *)&stack_single, sizeof(stack_single));
    print_info("stack[17]", (void *)&stack_arr[17], sizeof(stack_arr[17]));
    print_info("heap[0]", (void *)&heap[0], 1);
    print_info("heap block", (void *)heap, heap_size);
    printpgtable();

    if (sbrk(-heap_size) == (char *)-1) 
    {
        printf("free failed\n");
        exit(1);
    }

    printf("after free\n");
    printpgtable();

    (void)read_sum;
    exit(0);
}

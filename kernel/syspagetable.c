#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"


uint64 sys_checkpgflags(void)
{
    uint64 buf;
    int len;
    int mask;

    argaddr(0, &buf); 
    argint(1, &len);
    argint(2, &mask);

    if (len <= 0) return -1;
    if (mask == 0) return -1;
    if ((mask & ~(PTE_A | PTE_D)) != 0) return -1;
    if (buf >= MAXVA) return -1;

    struct proc *p;
    pte_t *pte;

    uint64 start = PGROUNDDOWN(buf);

    uint64 last_va = buf + len - 1;
    if (last_va >= MAXVA) return -1;
    if (last_va < buf) return -1;

    uint64 end = PGROUNDDOWN(last_va);

    p = myproc();

    for (uint64 va = start; va <= end; va += PGSIZE)
    {
        pte = walk(p->pagetable, va, 0);

        if (pte == 0) return -1;
        if ((*pte & PTE_V) == 0) return -1;

        if (((*pte) & mask) != 0) return 1;
    }


    return 0;
}

uint64 sys_clearpgflags(void)
{
    uint64 buf;
    int len;
    int mask;

    argaddr(0, &buf); 
    argint(1, &len);
    argint(2, &mask);

    if (len <= 0) return -1;
    if (mask == 0) return -1;
    if ((mask & ~(PTE_A | PTE_D)) != 0) return -1;
    if (buf >= MAXVA) return -1;

    struct proc *p;
    pte_t *pte;

    uint64 start = PGROUNDDOWN(buf);

    uint64 last_va = buf + len - 1;
    if (last_va >= MAXVA) return -1;
    if (last_va < buf) return -1;

    uint64 end = PGROUNDDOWN(last_va);

    p = myproc();

    for (uint64 va = start; va <= end; va += PGSIZE)
    {
        pte = walk(p->pagetable, va, 0);

        if (pte == 0) return -1;
        if ((*pte & PTE_V) == 0) return -1;

    }

    pte_t other_flags = ~(pte_t)mask;

    for (uint64 va = start; va <= end; va += PGSIZE)
    {
        pte = walk(p->pagetable, va, 0);

        *pte &= other_flags;
    }

    sfence_vma();


    return 0;
}

static void pte_to_format(pte_t pte)
{
    printf("%c", (pte & PTE_R) ? 'R' : '_');
    printf("%c", (pte & PTE_W) ? 'W' : '_');
    printf("%c", (pte & PTE_X) ? 'X' : '_');
    printf("%c", (pte & PTE_U) ? 'U' : '_');
    printf("%c", (pte & PTE_G) ? 'G' : '_');
    printf("%c", (pte & PTE_A) ? 'A' : '_');
    printf("%c", (pte & PTE_D) ? 'D' : '_');
}

static void print_hex3(int x)
{
    char hex[] = "0123456789abcdef";
    printf("0x%c%c%c", hex[(x >> 8) & 0xF], hex[(x >> 4) & 0xF], hex[x & 0xF]);
}

static void print_pgtable_rec(pagetable_t pgtable, int depth)
{
    for (int i = 0; i < 512; i++) 
    {
        pte_t pte = pgtable[i];

        if ((pte & PTE_V) == 0)
            continue;

        if (depth > 0)
        {
            for (int d = 0; d < depth; ++d) printf(".........");
            printf(" ");
        }

        print_hex3(i);
        printf(" -> %p ", (void *)PTE2PA(pte));
        pte_to_format(pte);
        printf("\n");

        if (!((pte & (PTE_R | PTE_W | PTE_X)) != 0)) 
        {
            pagetable_t child = (pagetable_t)PTE2PA(pte);
            print_pgtable_rec(child, depth + 1);
        }
    }
}

uint64 sys_printpgtable(void)
{
    struct proc *p = myproc();
    printf("\nPAGETABLE %p\n", (void *)p->pagetable);

    print_pgtable_rec(p->pagetable, 0);

    printf("\n");

    return 0;
}

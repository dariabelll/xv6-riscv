#include "stdarg.h"
#include "types.h"
#include "riscv.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

#define RING_BUFFER_SIZE (PGSIZE * DMESG_PAGECOUNT)

struct logconf {
    struct spinlock lock;
    int mask;
    uint ticks_bound;
} logconf;

struct ringbuffer {
    char buffer[RING_BUFFER_SIZE];
    struct spinlock lock;
    int head;
    int tail;
    int count;
} dmesg_buffer;

void init_ring_buffer() 
{
    dmesg_buffer.head = 0;
    dmesg_buffer.tail = 0;
    dmesg_buffer.count = 0;
    initlock(&dmesg_buffer.lock, "ringbuffer lock");

    logconf.mask = 0;
    logconf.ticks_bound = 0;
    initlock(&logconf.lock, "logconf");

    write_byte_to_ringbuffer('\n');
}

static void write_byte_to_ringbuffer(const char c)
{
    dmesg_buffer.buffer[dmesg_buffer.tail] = c;
    dmesg_buffer.tail = (dmesg_buffer.tail + 1) % RING_BUFFER_SIZE;

    if (dmesg_buffer.count < RING_BUFFER_SIZE)
    {
        dmesg_buffer.count += 1;
    }
    else
    {
        dmesg_buffer.head = (dmesg_buffer.head + 1) % RING_BUFFER_SIZE;
    }
}

static void write_number_to_ringbuffer(uint64 num, uint base)
{
    const char* digits = "0123456789abcdef"; 
    if (num == 0)
    {
        write_byte_to_ringbuffer(digits[0]);
        return;
    }

    char rev_number[32];
    uint len = 0;
    while (num != 0)
    {
        rev_number[len] = digits[num % base];
        num /= base;
        len += 1;
    }

    while (len > 0)
    {
        write_byte_to_ringbuffer(rev_number[len - 1]);
        len -= 1;
    }

}

static void write_signed_number_to_ringbuffer(int64 num)
{
    if (num < 0)
    {
        write_byte_to_ringbuffer('-');
        write_number_to_ringbuffer((uint64)(-(num + 1)) + 1, 10);
    }
    else
    {
        write_number_to_ringbuffer(num, 10);
    }
}

static void write_pointer_to_ringbuffer(uint64 num)
{
    write_byte_to_ringbuffer('0');
    write_byte_to_ringbuffer('x');
    write_number_to_ringbuffer(num, 16);
}

void pr_msg (const char *fmt, ...)
{
    acquire(&tickslock);
    uint t = ticks;
    release(&tickslock);


    acquire(&dmesg_buffer.lock);

    write_byte_to_ringbuffer('[');

    write_number_to_ringbuffer(t, 10);

    write_byte_to_ringbuffer(']');
    write_byte_to_ringbuffer(' ');

    va_list ap;
    int i, cx, c0, c1, c2;
    char *s;


    va_start(ap, fmt);
    for(i = 0; (cx = fmt[i] & 0xff) != 0; i++)
    {
        if(cx != '%')
        {
            write_byte_to_ringbuffer(cx);
            continue;
        }
        i++;

        c0 = fmt[i+0] & 0xff;
        c1 = c2 = 0;

        if(c0) c1 = fmt[i+1] & 0xff;
        if(c1) c2 = fmt[i+2] & 0xff;
        if(c0 == 'd')
        {
            write_signed_number_to_ringbuffer(va_arg(ap, int));
        } 
        else if(c0 == 'l' && c1 == 'd')
        {
            write_signed_number_to_ringbuffer(va_arg(ap, int64));
            i += 1;
        } 
        else if(c0 == 'l' && c1 == 'l' && c2 == 'd')
        {
            write_signed_number_to_ringbuffer(va_arg(ap, int64));
            i += 2;
        } 
        else if(c0 == 'u')
        {
            write_number_to_ringbuffer(va_arg(ap, uint32), 10);
        } 
        else if(c0 == 'l' && c1 == 'u')
        {
            write_number_to_ringbuffer(va_arg(ap, uint64), 10);
            i += 1;
        }
        else if(c0 == 'l' && c1 == 'l' && c2 == 'u')
        {
            write_number_to_ringbuffer(va_arg(ap, uint64), 10);
            i += 2;
        } 
        else if(c0 == 'x')
        {
            write_number_to_ringbuffer(va_arg(ap, uint32), 16);
        } 
        else if(c0 == 'l' && c1 == 'x')
        {
            write_number_to_ringbuffer(va_arg(ap, uint64), 16);
            i += 1;
        } 
        else if(c0 == 'l' && c1 == 'l' && c2 == 'x')
        {
            write_number_to_ringbuffer(va_arg(ap, uint64), 16);
            i += 2;
        } 
        else if(c0 == 'p')
        {
            write_pointer_to_ringbuffer(va_arg(ap, uint64));
        } 
        else if(c0 == 'c')
        {
            write_byte_to_ringbuffer(va_arg(ap, int));
        } 
        else if(c0 == 's')
        {
            if((s = va_arg(ap, char*)) == 0) s = "(null)";
            for(; *s; s++) write_byte_to_ringbuffer(*s);
        } else if(c0 == '%')
        {
            write_byte_to_ringbuffer('%');
        } 
        else if(c0 == 0)
        {
            break;
        } 
        else 
        {
            write_byte_to_ringbuffer('%');
            write_byte_to_ringbuffer(c0);
        }

    }
    va_end(ap);

    write_byte_to_ringbuffer('\n');

    release(&dmesg_buffer.lock);
}

int dmesg_read(uint64 buf, int size)
{
    struct proc *p = myproc();

    if (size <= 0) return -1;

    acquire(&dmesg_buffer.lock);
    int n = dmesg_buffer.count;
    if (dmesg_buffer.count >= size) n = size - 1;

    int symb_num = dmesg_buffer.head;
    for (int i = 0; i < n; ++i)
    {
        if(copyout(p->pagetable, buf + i, &dmesg_buffer.buffer[symb_num], 1) < 0) 
        {
            release(&dmesg_buffer.lock);
            return -1;
        }
        symb_num = (symb_num + 1) % RING_BUFFER_SIZE;
    }

    char null = '\0';
    if(copyout(p->pagetable, buf + n, &null, 1) < 0) 
    {
        release(&dmesg_buffer.lock);
        return -1;
    }


    release(&dmesg_buffer.lock);
    return n;
}

int log_set(int mask, int duration)
{
    if (mask < 0 || mask > LOG_ALL) return -1;

    acquire(&logconf.lock);

    logconf.mask = mask;

    if (duration > 0)
    {
        acquire(&tickslock);
        logconf.ticks_bound = ticks + duration;
        release(&tickslock);
    }
    else logconf.ticks_bound = 0;

    release(&logconf.lock);

    return 0;
}

int log_is_enable(int event_type)
{

    if (event_type != LOG_INTERRUPTS && event_type != LOG_SYSCALLS &&
        event_type != LOG_PROCESSES && event_type != LOG_EXEC) 
    {
        panic("log_is_enable: invalid event type");
    }

    acquire(&tickslock);
    uint t = ticks;
    release(&tickslock);

    int enable = 0;
    
    acquire(&logconf.lock);

    if (logconf.ticks_bound != 0 && t > logconf.ticks_bound)
    {
        logconf.mask = 0;
        logconf.ticks_bound = 0;
    }

    if ((event_type & logconf.mask) != 0)
    {
        enable = 1;
    }

    release(&logconf.lock);

    return enable;
}
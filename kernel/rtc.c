#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"

#define RTCReg(offset) ((volatile uint32 *)(RTC0 + (offset)))

static struct spinlock rtc_lock;

void rtcinit(void)
{
  initlock(&rtc_lock, "rtc");
}

static uint32 rtc_low_read(void)
{
    return *RTCReg(RTC_LOW);
}

static uint32 rtc_high_read(void)
{
    return *RTCReg(RTC_HIGH);
}

int64 rtc_read(void)
{
    uint32 low, high;

    acquire(&rtc_lock);

    low = rtc_low_read();
    high = rtc_high_read();
    
    release(&rtc_lock);

    return ((int64)high << 32) | low;
}
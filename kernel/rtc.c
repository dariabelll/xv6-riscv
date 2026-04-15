#include "types.h"
#include "memlayout.h"

#define RTC_LOW 0x00
#define RTC_HIGH 0x04

#define RTCReg(offset) ((volatile uint32 *)(RTC0 + (offset)))

static uint32 rtc_low_read(void)
{
    return *RTCReg(RTC_LOW);
}

static uint32 rtc_high_read(void)
{
    return *RTCReg(RTC_HIGH);
}

uint64 rtc_read(void)
{
    uint32 low = rtc_low_read();
    uint32 high = rtc_high_read();

    return ((uint64)high << 32) | low;
}
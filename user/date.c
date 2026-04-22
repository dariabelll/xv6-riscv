#include "kernel/types.h"
#include "user/user.h"

#define NANOSECS_IN_SECS 1000000000LL
#define SECS_IN_MINUTES 60
#define MINUTES_IN_HOURS 60
#define HOURS_IN_DAYS 24
#define SECS_IN_DAYS 86400LL
#define DAYS_IN_YEAR 365
#define DAYS_IN_LEAP_YEAR 366

static void print_nanosecs(uint nanosecs)
{
    uint div = 100000000;

    while (div > 0)
    {
        printf("%d", nanosecs / div);
        nanosecs %= div;
        div /= 10;
    }
}

static int is_leap_year(int year)
{
    if (year % 100 == 0 && year % 400 != 0) return 0;
    if (year % 4 == 0) return 1;
    return 0;
}

int main(void)
{
    int64 nanosecs = rtc();

    int64 secs = nanosecs / NANOSECS_IN_SECS;
    int64 nanosecs_mod = nanosecs % NANOSECS_IN_SECS;

    if (nanosecs_mod < 0)
    {
        nanosecs_mod += NANOSECS_IN_SECS;
        --secs;
    }

    int64 days = secs / SECS_IN_DAYS;
    int64 secs_in_day = secs % SECS_IN_DAYS;

    if (secs_in_day < 0)
    {
        secs_in_day += SECS_IN_DAYS;
        --days;
    }

    uint64 minutes = secs_in_day / SECS_IN_MINUTES;
    uint secs_mod = secs_in_day % SECS_IN_MINUTES;

    uint64 hours = minutes / MINUTES_IN_HOURS;
    uint minutes_mod = minutes % MINUTES_IN_HOURS;

    uint hours_mod = hours % HOURS_IN_DAYS;

    int curr_year;
    int curr_month;
    int curr_day;

    uint month_to_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (days >= 0)
    {
        curr_year = 1970;

        while (1)
        {
            int curr_year_days = is_leap_year(curr_year) ? DAYS_IN_LEAP_YEAR : DAYS_IN_YEAR;

            if (days < curr_year_days) break;

            days -= curr_year_days;
            ++curr_year;
        }
    }
    else
    {
        curr_year = 1969;

        while (1)
        {
            int curr_year_days = is_leap_year(curr_year) ? DAYS_IN_LEAP_YEAR : DAYS_IN_YEAR;

            if (days >= -curr_year_days)
            {
                days += curr_year_days;
                break;
            }

            days += curr_year_days;
            --curr_year;
        }
    }

    curr_month = 1;
    while (1)
    {
        int curr_month_days = month_to_days[curr_month - 1];
        if (curr_month == 2 && is_leap_year(curr_year)) ++curr_month_days;

        if (days < curr_month_days) break;

        days -= curr_month_days;
        ++curr_month;
    }

    curr_day = days + 1;

    printf("%d-", curr_year);
    printf("%d%d-", curr_month / 10, curr_month % 10);
    printf("%d%d", curr_day / 10, curr_day % 10);
    printf(" %d%d:", hours_mod / 10, hours_mod % 10);
    printf("%d%d:", minutes_mod / 10, minutes_mod % 10);
    printf("%d%d.", secs_mod / 10, secs_mod % 10);
    print_nanosecs(nanosecs_mod);
    printf("\n");

    exit(0);
}
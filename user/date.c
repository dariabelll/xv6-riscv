#include "kernel/types.h"
#include "user/user.h"

#define NANOSECS_IN_SECS 1000000000ULL
#define SECS_IN_MINUTES 60
#define MINUTES_IN_HOURS 60
#define HOURS_IN_DAYS 24
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

static int is_leap_year(uint year)
{
    if (year % 100 == 0 && year % 400 != 0) return 0;

    if (year % 4 == 0) return 1;

    return 0;
}

int main(void)
{
    uint64 nanosecs = rtc();

    uint64 secs = nanosecs / NANOSECS_IN_SECS;
    uint64 nanosecs_mod = nanosecs % NANOSECS_IN_SECS;

    uint64 minutes = secs / SECS_IN_MINUTES;
    uint secs_mod = secs % SECS_IN_MINUTES;

    uint64 hours = minutes / MINUTES_IN_HOURS;
    uint minutes_mod = minutes % MINUTES_IN_HOURS;

    uint64 days = hours / HOURS_IN_DAYS;
    uint hours_mod = hours % HOURS_IN_DAYS;

    uint curr_year = 1970;

    while (1)
    {
        uint curr_year_days = is_leap_year(curr_year) ? DAYS_IN_LEAP_YEAR : DAYS_IN_YEAR;

        if (days < curr_year_days) break;

        days -= curr_year_days;
        ++curr_year;
    }

    uint month_to_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    uint curr_month = 1;
    while (1)
    {
        uint curr_month_days = month_to_days[curr_month - 1];
        if (curr_month == 2 && is_leap_year(curr_year)) ++curr_month_days;

        if (days < curr_month_days) break;

        days -= curr_month_days;
        ++curr_month;
    }

    uint curr_day = days + 1;

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
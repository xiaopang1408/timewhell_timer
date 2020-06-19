#ifndef NTX_TIME_H
#define NTX_TIME_H

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <string>

struct timeval ntx_later_time(struct timeval* tv, uint32_t later_seconds, uint32_t later_milliseconds) 
{
    uint32_t usec = tv->tv_usec + later_milliseconds*1000;
    uint32_t add_seconds = usec/1000000;
    uint32_t left_usec = usec - add_seconds*1000000;

    struct timeval result_tv;
    result_tv.tv_sec = tv->tv_sec + later_seconds + add_seconds;
    result_tv.tv_usec = left_usec;
    return result_tv;
}

std::string timestamp2string(time_t time)
{
    struct tm* tm_time = localtime(&time);
    char sz_time[128] = {0};
    strftime(sz_time, sizeof(sz_time), "%Y-%m-%d %H:%M:%S", tm_time);
    return sz_time;
}

uint32_t now_time()
{
    return (uint32_t)time(NULL);
}

#endif

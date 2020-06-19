#ifndef NTX_TIMER_H
#define NTX_TIMER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

const uint32_t kMaxTimerDays = 400;

typedef void (*ntx_timer_cb_t)(void*);

//定时器节点：双向链表便于删除和插入元素
//暂不支持循环定时器的情况，如果是循环定时器，需要在触发后，
//在应用层立即在创建一个相同的定时器
struct ntx_timer_node_t 
{
    uint32_t timer_id; //需要timer_id吗？
    uint32_t seconds;
    uint32_t milliseconds;
    struct timeval timeout_tv;
    struct timeval create_tv; //创建定时器的时间
    void* owner;
    void* data;
    ntx_timer_cb_t callback;
    struct ntx_timer_node_t* prev;
    struct ntx_timer_node_t* next;
};

typedef struct ntx_timer_node_t* ntx_timer_list_t;


//时间轮定时器管理器
class ntx_timer_manager {
public:
    void init();
    void add_timer(void* owner, void* data, ntx_timer_cb_t cb, uint32_t later_seconds, uint32_t later_milli_seconds); 
    void check_timeout();
    void display_timers_info();

private:
    //已经触发的定时器链表: 将会被处理
    ntx_timer_list_t m_timeout_timers;
    
    //毫秒定时器链表
    ntx_timer_list_t m_millisecond_timers[100];
    ntx_timer_list_t m_second_timers[60];
    ntx_timer_list_t m_minute_timers[60];
    ntx_timer_list_t m_hour_timers[24];
    ntx_timer_list_t m_day_timers[400];

    uint32_t round; 
};

#endif

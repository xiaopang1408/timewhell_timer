#include "ntx_timer.h"
#include "ntx_time.h"

void ntx_timer_manager::init()
{
    round = 0; 

    for( int i=0; i<100; i++ ) {
        m_millisecond_timers[i] = new ntx_timer_node_t;
        m_millisecond_timers[i]->next = NULL;
    }

    for( int i=0; i<60; i++ ) {
        m_second_timers[i] = new ntx_timer_node_t;
        m_second_timers[i]->next = NULL;
    }

    for( int i=0; i<60; i++ ) {
        m_minute_timers[i] = new ntx_timer_node_t;
        m_minute_timers[i]->next = NULL;
    }

    for( int i=0; i<24; i++ ) {
        m_hour_timers[i] = new ntx_timer_node_t;
        m_hour_timers[i]->next = NULL;
    }

    for( int i=0; i<400; i++ ) {
        m_day_timers[i] = new ntx_timer_node_t;
        m_day_timers[i]->next = NULL;
    }

    m_timeout_timers = new ntx_timer_node_t;
    m_timeout_timers->next = NULL;
}

void ntx_timer_manager::add_timer(void* owner, void* data, ntx_timer_cb_t cb, uint32_t later_seconds, uint32_t later_milli_seconds)  
{
    if (later_seconds == 0 && later_milli_seconds == 0 ) {
        //TODO: ERROR LOG HERE
        //printf("ERROR: invalid args addtimer sec:%d millisec:%d\n", later_seconds, later_milli_seconds);
        return;
    }

    struct timeval now_tv;
    gettimeofday(&now_tv, NULL); 
    
    //分配一个新的节点
    ntx_timer_node_t* new_tm_node = new ntx_timer_node_t;
    new_tm_node->owner = owner;
    new_tm_node->data = data;
    new_tm_node->seconds = later_seconds;
    new_tm_node->milliseconds = later_milli_seconds;
    new_tm_node->timeout_tv = ntx_later_time(&now_tv, later_seconds, later_milli_seconds); 
    new_tm_node->create_tv = now_tv;
    new_tm_node->callback = cb;

    ntx_timer_list_t tm_list = NULL; 
    
    //加入毫秒定时器链表
    if (later_seconds == 0 && later_milli_seconds > 0 ) {
        if (later_milli_seconds >= 1000 ) {
            //TODO: ERROR LOG HERE
            return;
        }

        uint32_t idx = later_milli_seconds / 10;
        tm_list = m_millisecond_timers[idx];

    } else if ( later_seconds < 60 ) {
        //加入秒定时器
        uint32_t idx = later_seconds - 1;
        tm_list = m_second_timers[idx];

    } else if ( later_seconds >= 60 && later_seconds < 3600 ) {
        //加入分钟定时器
        uint32_t idx = later_seconds / 60;
        tm_list = m_minute_timers[idx-1];

    } else if ( later_seconds >= 3600 && later_seconds < 3600*24 ) {
        uint32_t idx = later_seconds / 3600;
        tm_list = m_hour_timers[idx-1];

    } else if ( later_seconds >= 3600*24 && later_seconds < 3600*24*kMaxTimerDays ) {
        uint32_t idx = later_seconds / (3600*24);
        tm_list = m_day_timers[idx-1];

    } else if ( later_seconds >= 3600*24*kMaxTimerDays ) {
        printf("addtimer over max timeout seconds %d\n", later_seconds);
    }    

    if (NULL == tm_list) {
        //TODO: LOG_ERROR 
        return;
    }


    new_tm_node->next = tm_list->next;
    tm_list->next = new_tm_node;
}

void ntx_timer_manager::check_timeout()
{
    display_timers_info();

    //假设tick=10ms，也就是round每隔10ms加1 

    std::string now_time_string = timestamp2string(now_time());

    //处理过期定时器
    ntx_timer_node_t* timeout_node = m_timeout_timers->next;
    while(true) {
        if (NULL == timeout_node) {
            break;
        }

        ntx_timer_node_t* cur_node = timeout_node;

        std::string timeout_timer_string = timestamp2string(cur_node->timeout_tv.tv_sec);
        std::string create_timer_string = timestamp2string(cur_node->create_tv.tv_sec);
        printf("ntxtimer fired here sec:%d usec:%d round:%d, nowtm:%s timer_timeout_tm:%s, create_tm:%s\n", 
            timeout_node->seconds, timeout_node->milliseconds, round, now_time_string.c_str(), timeout_timer_string.c_str(), create_timer_string.c_str());
        timeout_node->callback(timeout_node->data);
        timeout_node = timeout_node->next;

        delete cur_node;
        cur_node = NULL;
    }
    m_timeout_timers->next = NULL;

    //将m_millisecond_timers中槽位0的定时器转移到m_timeout_timers中-->槽位0的定时器时间已经到达，要在下一帧触发了
    ntx_timer_node_t* milli_node = m_millisecond_timers[0]->next;
    while(true) {
        if ( NULL == milli_node ) {
            break;
        }
        
        ntx_timer_node_t* next_milli_node = milli_node->next;

        m_millisecond_timers[0]->next = milli_node->next;

        milli_node->next = m_timeout_timers->next;
        m_timeout_timers->next = milli_node;

        milli_node = next_milli_node;

    }
    m_millisecond_timers[0]->next = NULL;

    //将m_millisecond_timers中槽位1-99的定时器转移到前一个槽位
    for( int i=0; i<99; i++ ) {
        ntx_timer_node_t* head_node = m_millisecond_timers[i];
        ntx_timer_node_t* next_head_node = m_millisecond_timers[i+1];
        
        head_node->next = next_head_node->next;
        next_head_node->next = NULL;
    }

    if ( round && (round % 100 == 0) ) {
        //过了一秒
        //将m_second_timers[0]转移到 m_millisecond_timers中
        while(true) {
            ntx_timer_node_t* tm_node = m_second_timers[0]->next;
            if ( NULL == tm_node ) {
                break;
            }
            uint32_t idx = tm_node->milliseconds / 10;

            m_second_timers[0]->next = tm_node->next;
            tm_node->next = m_millisecond_timers[idx]->next;
            m_millisecond_timers[idx]->next = tm_node;
        }
        m_second_timers[0]->next = NULL;

        //然后将m_second_timers[1-59]的位置往前移动一个位置
        for( int i=0; i<59; i++ ) {
            m_second_timers[i]->next = m_second_timers[i+1]->next;
            m_second_timers[i+1]->next = NULL;
        }  
    }

    if( round && (round % (60*100) == 0) ) {
        //过了60秒 分盘转动 将分盘[0]下标的定时器移动到秒盘 后面的往前移动一个格子
        while(true) {
            ntx_timer_node_t* tm_node = m_minute_timers[0]->next;
            if( NULL == tm_node ) {
                break;
            }
            uint32_t sec = tm_node->seconds - (tm_node->seconds/60)*60;
            uint32_t idx = sec % 60;
            m_minute_timers[0]->next = tm_node->next;
            tm_node->next = m_second_timers[idx]->next;
            m_second_timers[idx]->next = tm_node;
        }
        m_minute_timers[0]->next = NULL;

        for(int i=0; i<59; i++) {
            m_minute_timers[i]->next = m_minute_timers[i+1]->next;
            m_minute_timers[i+1]->next = NULL;
        }
    }

    if( round && (round % (60*60*100) == 0) ) {
        //如果过了3600秒 时盘转动 将时盘[0]下标的定时器移动到分盘 时盘后面格子往前移动一个格子
        while(true) {
            ntx_timer_node_t* tm_node = m_hour_timers[0]->next;
            if( NULL == tm_node ) {
                break; 
            }
            uint32_t sec = tm_node->seconds - (tm_node->seconds/3600)*3600;
            uint32_t idx = sec / 60;
            m_hour_timers[0]->next = tm_node->next;
            tm_node->next = m_minute_timers[idx]->next;
            m_minute_timers[idx]->next = tm_node;
        }
        m_hour_timers[0]->next = NULL;

        for( int i=0; i<23; i++ ) {
            m_hour_timers[i]->next = m_hour_timers[i+1]->next;
            m_hour_timers[i+1]->next = NULL;
        }
    }

    if( round && (round % (24*60*60*100) == 0) ) {
        //如果超过3600*24秒， 也就是1天，则天盘[0]下标的定时器移动到时盘，天盘后面的格子往前移动一个格子
        while(true) {
            ntx_timer_node_t* tm_node = m_day_timers[0]->next;
            if( NULL == tm_node ) {
                break;
            }
            uint32_t sec = tm_node->seconds - (tm_node->seconds/86400)*86400;
            uint32_t idx = sec/3600;
            m_day_timers[0]->next = tm_node->next;
            tm_node->next = m_hour_timers[idx]->next;
            m_hour_timers[idx]->next = tm_node;
        }
        m_day_timers[0]->next = NULL;

        for( int i=0; i<int(kMaxTimerDays)-1; i++ ) {        
            m_day_timers[i]->next = m_day_timers[i+1]->next;
            m_day_timers[i+1]->next = NULL;
        }
    }

    round++;
}

void ntx_timer_manager::display_timers_info()
{
    for( int i=0; i<100; i++ ) {
        ntx_timer_node_t* tm_node = m_millisecond_timers[i]->next;
        while(true) {
            if( NULL == tm_node ) {
                break;
            }
            tm_node = tm_node->next;
        }    
    } 

    ntx_timer_node_t* node = m_timeout_timers->next;
    while(true) {
        if( NULL == node ) {
            break; 
        } 
        node = node->next;
    }
}

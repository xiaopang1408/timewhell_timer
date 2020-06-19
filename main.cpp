#include "ntx_global_data.h"
#include "ntx_timer.h"
#include "ntx_random.h"
#include "ntx_utils.h"
#include <unistd.h>

struct user_t
{
    uint32_t userid;
    char name[64];
};

void user_timer_callback(void* data)
{
    user_t* user = (user_t*)data;
    
    if( NULL != user ) {
        //printf("timer arrived, userid:%d, name:%s\n", user->userid, user->name);
    }
}

void test_rand_by_range()
{
    uint32_t min_num = 1, max_num = 100;
    for( uint32_t i=0; i<10000; i++ ) {
        uint32_t result = rand_by_range(min_num, max_num); 
        printf("test_rand_by_range, min:%d max:%d result:%d\n", min_num, max_num, result);
    }
}


int main()
{
    //test_rand_by_range();

    //return 0;

    g_ntx_timer_manager->init();

    uint32_t timer_cnt = 100;
    for( uint32_t i=0; i<timer_cnt; i++ ) {
        uint32_t min_seconds = 1, max_seconds = 180;
        uint32_t min_milliseconds = 0, max_milliseconds = 1000;
        uint32_t seconds = rand_by_range(min_seconds, max_seconds); 
        uint32_t milliseconds = rand_by_range(min_milliseconds, max_milliseconds);

        user_t* user = new user_t;
        user->userid = i+1;

        char userid_str[32];
        num2string(user->userid, userid_str, 32);
        sprintf(user->name, "username_%s", userid_str);

        g_ntx_timer_manager->add_timer(NULL, (void*)user, user_timer_callback, seconds, milliseconds);
    }

    while(true) {
        usleep(10000);
        g_ntx_timer_manager->check_timeout();
    }

    return 0;
}

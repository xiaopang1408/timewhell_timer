#include "ntx_random.h"
#include <sys/time.h>


uint32_t rand_by_range(uint32_t imin, uint32_t imax)
{
    uint32_t min = imin;
    uint32_t max = imax;
    if( min > max ) {
        min = imax;
        max = imin;
    }
    if( min == 0 && max == 0 ) {
        return 0;
    }

    struct timeval us;
    gettimeofday(&us, NULL);

    srand((int)us.tv_usec);
    
    uint32_t res = min + rand()%(max-min+1);
    return res;
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ntx_utils.h"


void num2string(uint32_t num, char* str, int stringlength) 
{
    memset(str, 0, stringlength);
    sprintf(str, "%d", num);
}

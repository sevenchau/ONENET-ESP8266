#include "dr_private.h"

void DR_Init(void)
{
    DR_LED_Init();
    DR_SwitchInit();
    WIFI_Init();
}


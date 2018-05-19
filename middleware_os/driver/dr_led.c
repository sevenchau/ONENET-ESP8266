#include "dr_private.h"

#define  BSP_GPIOD_LED1                        DEF_BIT_12
#define  BSP_GPIOD_LED2                        DEF_BIT_13
#define  BSP_GPIOD_LED3                        DEF_BIT_14
#define  BSP_GPIOD_LED4                        DEF_BIT_15

void DR_LED_Init(void)
{
    BSP_OutPPIO_Init(IO_INIT(GPIOD, 12));
    BSP_OutPPIO_Init(IO_INIT(GPIOD, 13));
    BSP_OutPPIO_Init(IO_INIT(GPIOD, 14));
    BSP_OutPPIO_Init(IO_INIT(GPIOD, 15));
}

void  DR_LED_On (CPU_INT08U    led)
{
    switch (led) {
        case 0u:
             BSP_IO_SetBits(IO_CTR(GPIOD, 12));
             BSP_IO_SetBits(IO_CTR(GPIOD, 13));
             BSP_IO_SetBits(IO_CTR(GPIOD, 14));
             BSP_IO_SetBits(IO_CTR(GPIOD, 15));
             break;
        case 1u:
             BSP_IO_SetBits(IO_CTR(GPIOD, 12));
             break;
        case 2u:
             BSP_IO_SetBits(IO_CTR(GPIOD, 13));
             break;
        case 3u:
             BSP_IO_SetBits(IO_CTR(GPIOD, 14));
             break;
        case 4u:
             BSP_IO_SetBits(IO_CTR(GPIOD, 15));
             break;
        default:
             break;
    }
}

void  DR_LED_Off (CPU_INT08U led)
{
    switch (led) {
        case 0u:
             BSP_IO_ResetBits(IO_CTR(GPIOD, 12));
             BSP_IO_ResetBits(IO_CTR(GPIOD, 13));
             BSP_IO_ResetBits(IO_CTR(GPIOD, 14));
             BSP_IO_ResetBits(IO_CTR(GPIOD, 15));
             break;
        case 1u:
             BSP_IO_ResetBits(IO_CTR(GPIOD, 12));
             break;
        case 2u:
             BSP_IO_ResetBits(IO_CTR(GPIOD, 13));
             break;
        case 3u:
             BSP_IO_ResetBits(IO_CTR(GPIOD, 14));
             break;
        case 4u:
             BSP_IO_ResetBits(IO_CTR(GPIOD, 15));
             break;
        default:
             break;
    }
}

void  DR_LED_Toggle (CPU_INT08U  led)
{
    switch (led) {
        case 0u:
             BSP_IO_Toggle(IO_CTR(GPIOD, 12));
             BSP_IO_Toggle(IO_CTR(GPIOD, 13));
             BSP_IO_Toggle(IO_CTR(GPIOD, 14));
             BSP_IO_Toggle(IO_CTR(GPIOD, 15));
             break;
        case 1u:
             BSP_IO_Toggle(IO_CTR(GPIOD, 12));
             break;
        case 2u:
             BSP_IO_Toggle(IO_CTR(GPIOD, 13));
             break;
        case 3u:
             BSP_IO_Toggle(IO_CTR(GPIOD, 14));
             break;
        case 4u:
             BSP_IO_Toggle(IO_CTR(GPIOD, 15));
             break;
        default:
             break;
    }
}

void DR_SwitchInit(void)
{
    BSP_OutPPIO_Init(IO_INIT(GPIOE, 6));
    BSP_IO_ResetBits(IO_CTR(GPIOE, 6));
}

void  DR_SwitchOn (CPU_INT08U    pswitch)
{
    BSP_IO_SetBits(IO_CTR(GPIOE, 6));
}

void  DR_SwitchOff (CPU_INT08U    pswitch)
{
    BSP_IO_ResetBits(IO_CTR(GPIOE, 6));
}
#include "bsp_private.h"

void  BSP_INFLOATIO_Init(CPU_DATA  pwr_clk_id, GPIO_TypeDef* gpio, uint16_t pin)
{
    GPIO_InitTypeDef  gpio_init;

    BSP_PeriphEn(pwr_clk_id);
    
    gpio_init.GPIO_Pin   = pin;
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_DOWN;//GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    
    GPIO_Init(gpio, &gpio_init);
}

void  BSP_OutPPIO_Init(CPU_DATA  pwr_clk_id, GPIO_TypeDef* gpio, uint16_t pin)
{
    GPIO_InitTypeDef  gpio_init;

    BSP_PeriphEn(pwr_clk_id);
    
    gpio_init.GPIO_Pin   = pin;
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_UP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    
    GPIO_Init(gpio, &gpio_init);
}

void  BSP_IO_SetBits (GPIO_TypeDef* gpio, uint16_t pin)
{
    GPIO_SetBits(gpio, pin);
}

void  BSP_IO_ResetBits (GPIO_TypeDef* gpio, uint16_t pin)
{
    GPIO_ResetBits(gpio, pin);
}


void  BSP_IO_Toggle (GPIO_TypeDef* gpio, uint16_t pin)
{
    CPU_INT32U  pins;

    pins  = GPIO_ReadOutputData(gpio);
    pins ^= pin;
    GPIO_SetBits(  gpio,   pins   &  pin);
    GPIO_ResetBits(gpio, (~pins)  &  pin);
}

uint8_t BSP_IO_ReadInPutBit(GPIO_TypeDef* gpio, uint16_t pin)
{
    return GPIO_ReadInputDataBit(gpio, pin);
}

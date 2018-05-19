#include "dr_private.h"

#define DHT11_INIT()            IO_INIT(GPIOB,0)
#define DHT11_CTR_PARA()        IO_CTR(GPIOB,0)

void DHT11_Init(void)
{
    BSP_OutPPIO_Init(DHT11_INIT());
    
    BSP_IO_ResetBits(DHT11_CTR_PARA());
    delay_us(19000);
    BSP_IO_SetBits(DHT11_CTR_PARA());
    delay_us(50);
    
    BSP_INFLOATIO_Init(DHT11_INIT());
}

uint16_t DHT11_ReadBit(void)
{
    while (BSP_IO_ReadInPutBit(DHT11_CTR_PARA()) == RESET);
    delay_us(40);
    if (BSP_IO_ReadInPutBit(DHT11_CTR_PARA()) == SET) {
        while (BSP_IO_ReadInPutBit(DHT11_CTR_PARA()) == SET);
        return 1;
    }
    else {
        return 0;
    }
}

uint16_t DHT11_ReadByte(void)
{
    uint16_t i;
    uint16_t data = 0;
    for (i = 0; i < 8; i++) {
        data <<= 1;
        data |= DHT11_ReadBit();
    }
    return data;
}

void DHT11_Reset(void)
{
    DHT11_Init();
}

uint8_t DHT11_ReadMsg(uint8_t humidity[2] , uint8_t temperature[2])
{
    uint16_t i = 0;
    uint8_t  buffer[5] = {0};
    
    DHT11_Init();
    
    if (BSP_IO_ReadInPutBit(DHT11_CTR_PARA()) == RESET) {
        //¼ì²âµ½DHT11ÏìÓ¦
        while (BSP_IO_ReadInPutBit(DHT11_CTR_PARA()) == RESET);
        while (BSP_IO_ReadInPutBit(DHT11_CTR_PARA()) == SET);
        for (i = 0; i < 5; i++) {
            buffer[i] = DHT11_ReadByte();
        }
        
        while (BSP_IO_ReadInPutBit(DHT11_CTR_PARA()) == RESET);
        
        BSP_OutPPIO_Init(DHT11_INIT());
        BSP_IO_SetBits(DHT11_CTR_PARA());
        
        u8 checksum = buffer[0] + buffer[1] + buffer[2] + buffer[3];
        if (checksum != buffer[4]) {
            // checksum error
            return ERROR;
        }
        
        Mem_Copy(humidity, buffer, sizeof(humidity));
        Mem_Copy(temperature, buffer+2, sizeof(temperature));
        
        return SUCCESS;
    }
    
    return ERROR;
}
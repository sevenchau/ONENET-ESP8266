#ifndef __DR_INC_133ED5B_H
#define __DR_INC_133ED5B_H

#include "bsp.h"
#include "mem_malloc.h"

#define   CONNECTED                                     1
#define   UNCONNECT                                     0

#define   TX_IDLE                                       0
#define   TX_ING                                        1
#define   TX_OK                                         2
#define   TX_FAILED                                     3

extern void DR_Init(void);

extern void  DR_LED_Off    (CPU_INT08U led);
extern void  DR_LED_On     (CPU_INT08U led);
extern void  DR_LED_Toggle (CPU_INT08U led);

extern void  DHT11_Reset(void);
extern uint8_t DHT11_ReadMsg(uint8_t humidity[2] , uint8_t temperature[2]);

extern void  DR_SwitchOn (CPU_INT08U    pswitch);
extern void  DR_SwitchOff (CPU_INT08U    pswitch);

extern void     WIFI_RxMsgCallBackInit(void(*payload_cb)(uint8_t* pchar, uint32_t len));
extern FunctionalState WIFI_IsTxEnable(void);
extern uint8_t  WIFI_GetLogInServerStatus(void);
extern uint8_t  WIFI_SendMsg(uint8_t* pchar, uint32_t len);
extern void     WIFI_TmrIrqHook(void);
extern void     WIFI_SetReConnectToServer(void);

#endif
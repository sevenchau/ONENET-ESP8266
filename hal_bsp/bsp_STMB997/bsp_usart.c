#include "bsp_private.h"

struct dev_t {
    uint8_t* dev_name;
	uint32_t dev_id;
};

struct comdev_t {
	struct dev_t dev;

	uint32_t baudrate;
	CPU_INT08U irq_pro;
	void (*rx_irq_cb)(uint16_t);
};

void USART_IrqRxDefault(uint16_t value) {};

void (*USART1_IrqRxCallBack)(uint16_t value) = USART_IrqRxDefault;
void (*USART2_IrqRxCallBack)(uint16_t value) = USART_IrqRxDefault;

/*  PA9 :USART2_TX
    PA10:USART2_RX  */
void USART1_Init(uint32_t BaudRate, CPU_INT08U irq_pro, void (*rx_irq_cb)(uint16_t))
{
  	GPIO_InitTypeDef GPIO_InitStructure;
  	USART_InitTypeDef USART_InitStructure;
  
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
  	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);  
  	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);    
	
  	USART_InitStructure.USART_BaudRate = BaudRate;
  	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  	USART_InitStructure.USART_StopBits = USART_StopBits_1;
  	USART_InitStructure.USART_Parity = USART_Parity_No;
  	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; 
  	USART_Init(USART1, &USART_InitStructure);
  	USART_Cmd(USART1, ENABLE);
  	USART_ClearFlag(USART1, USART_FLAG_TC);

    USART_ITConfig(USART1, USART_IT_TC, DISABLE);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
    BSP_IntVectSet(BSP_INT_ID_USART1, BSP_IntHandlerUSART1); 
	
    // Use a lower priority number for a higher priority.
    BSP_IntPrioSet(BSP_INT_ID_USART1,irq_pro);   
    BSP_IntDis(BSP_INT_ID_USART1);

	USART1_IrqRxCallBack = rx_irq_cb;
}

/*  PA2:USART2_TX
    PA3:USART2_RX  */
void USART2_Init(uint32_t BaudRate, CPU_INT08U irq_pro, void (*rx_irq_cb)(uint16_t))
{
  	GPIO_InitTypeDef GPIO_InitStructure;
  	USART_InitTypeDef USART_InitStructure;
  
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 
  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
  	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);  
  	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);    
	
  	USART_InitStructure.USART_BaudRate = BaudRate;
  	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  	USART_InitStructure.USART_StopBits = USART_StopBits_1;
  	USART_InitStructure.USART_Parity = USART_Parity_No;
  	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; 
  	USART_Init(USART2, &USART_InitStructure);
  	USART_Cmd(USART2, ENABLE);
  	USART_ClearFlag(USART2, USART_FLAG_TC);

    USART_ITConfig(USART2, USART_IT_TC, DISABLE);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	
    BSP_IntVectSet(BSP_INT_ID_USART2, BSP_IntHandlerUSART2); 
	
    // Use a lower priority number for a higher priority.
    BSP_IntPrioSet(BSP_INT_ID_USART2,irq_pro);   
    BSP_IntDis(BSP_INT_ID_USART2);

	USART2_IrqRxCallBack = rx_irq_cb;
}

void USART1_RxIrqCtl(FunctionalState NewState)
{
    (NewState==ENABLE)?(BSP_IntEn(BSP_INT_ID_USART1)):(BSP_IntDis(BSP_INT_ID_USART1));
}

void USART2_RxIrqCtl(FunctionalState NewState)
{
    (NewState==ENABLE)?(BSP_IntEn(BSP_INT_ID_USART2)):(BSP_IntDis(BSP_INT_ID_USART2));
}

void USART1_Send(uint8_t ch)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
	USART_SendData(USART1, ch);
//	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}

void USART2_Send(uint8_t ch)
{
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
	USART_SendData(USART2, ch);
//	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
}

void USART1_IrqProcess(void)
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) { 
		USART1_IrqRxCallBack((uint8_t)USART_ReceiveData(USART1));

		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}

void USART2_IrqProcess(void)
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) { 
		USART2_IrqRxCallBack((uint8_t)USART_ReceiveData(USART2));

		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
}
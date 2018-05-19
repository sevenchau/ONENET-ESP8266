#ifndef   __MEM_LIB_133ED5A_H
#define   __MEM_LIB_133ED5A_H

#include <stddef.h>
#include <stdlib.h>
#include  "os.h"
#include "stm32f4xx.h"

#define      MEM_REGION_NUM           2
#define      MEM_MIN_NUMBER           80
#define      MEM_MIN_SIZE             128
#define      MEM_MAX_NUMBER           28
#define      MEM_MAX_SIZE             1024

uint8_t MEM_Init(void);
void* MEM_Malloc(uint32_t len);
void MEM_Free(void* paddr);

#endif
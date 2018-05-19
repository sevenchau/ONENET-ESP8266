#include "mem_malloc.h"

//OS_MEM       MEM_Pointer_Min;
//OS_MEM       MEM_Pointer_MAX;

//CPU_INT08U MEM_MinTcb[MEM_MIN_NUMBER][MEM_MIN_SIZE];
//CPU_INT08U MEM_MaxTcb[MEM_MAX_NUMBER][MEM_MAX_SIZE];

struct MEM_STR {
	OS_MEM*  pos_mem;
	uint8_t* pblk_name;
	CPU_INT08U* pblk;
	uint8_t  n_blks;	
	uint16_t blk_size;
	
	uint32_t blk_start_addr;
	uint32_t blk_end_addr;
};

struct MEM_STR MEMStr[MEM_REGION_NUM] = {
	{NULL, "mem1", NULL, MEM_MIN_NUMBER, MEM_MIN_SIZE, 0, 0},
	{NULL, "mem2", NULL, MEM_MAX_NUMBER, MEM_MAX_SIZE, 0, 0}
};

uint8_t MEM_Init(void)
{
	OS_ERR os_err;
	for (uint8_t i=0; i<MEM_REGION_NUM; i++) {
		MEMStr[i].pos_mem = (OS_MEM*)malloc(sizeof(OS_MEM));
		MEMStr[i].pblk  = malloc(MEMStr[i].n_blks*MEMStr[i].blk_size);
		MEMStr[i].blk_start_addr = (uint32_t)MEMStr[i].pblk;
		MEMStr[i].blk_end_addr   = (uint32_t)(MEMStr[i].pblk+MEMStr[i].n_blks*MEMStr[i].blk_size);
		OSMemCreate(MEMStr[i].pos_mem, (CPU_CHAR*)MEMStr[i].pblk_name, 
                    MEMStr[i].pblk, MEMStr[i].n_blks, MEMStr[i].blk_size, &os_err);
		if (OS_ERR_NONE != os_err) {
			return ERROR;
		}
	}
	return SUCCESS;
}

void* MEM_Malloc(uint32_t len)
{
	OS_ERR os_err;
	CPU_INT08U* tmp_pkt = NULL;
    
	for (uint8_t i=0; i<MEM_REGION_NUM; i++) {
		if (len <= MEMStr[i].blk_size) {
			tmp_pkt = OSMemGet(MEMStr[i].pos_mem, &os_err);
			if (OS_ERR_NONE == os_err) {
				return tmp_pkt;
			}
		}
	}
	
	return NULL;
}

void MEM_Free(void* paddr)
{
    OS_ERR os_err;
    uint32_t this_addr = (uint32_t)paddr;
    
    for (uint8_t i=0; i<MEM_REGION_NUM; i++) {
        if (this_addr >= MEMStr[i].blk_start_addr && this_addr <= MEMStr[i].blk_end_addr) {
            OSMemPut(MEMStr[i].pos_mem, paddr, &os_err);
            return;
        }
    }
}

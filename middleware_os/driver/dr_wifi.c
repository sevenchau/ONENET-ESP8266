#include "dr_private.h"

#define  DR_TASK_WIFI_PRIO           3u
#define  DR_TASK_WIFI_STK_SIZE       512

#define   RX_MAX_LEN                 MEM_MAX_SIZE
#define   RX_TMROUT                  30

#define   NET_USART_INIT             USART2_Init
#define   NET_USART_SEND             USART2_Send
#define   NET_USART_IRQ_CTL          USART2_RxIrqCtl

union word_t {
    uint32_t word;
    struct {
        uint32_t is_wifi_connected:2;
        uint32_t is_login:2;
//        uint32_t is_relogin:1;
        
        uint32_t is_send_ok:4;
    };
};

struct wifi_str_msg_t {
    char*     pchar;
    uint8_t (*fuc)(void);
};

struct wifi_dr_t {
	OS_Q     msg_q;
	
	uint8_t  irq_rx_status:1;
	OS_TICK  rx_tmr_pro;
	uint8_t* rx_pkt;
	uint32_t rx_count;

//    uint8_t  is_send_ok;
    uint8_t* tx_pkt;
    uint32_t tx_len;
    OS_TICK  tx_tmr_pro;
    
//	uint8_t  is_connected:1;
    OS_TICK  is_connected_tmr_pro;
//	uint8_t  is_relogin;
    
    union word_t status;
    
    void (*payload_cb)(uint8_t* pchar, uint32_t len);
};

uint8_t WIFI_ConnectServer(void);
uint8_t WIFI_ReConnectWifi(void);
uint8_t WIFI_SendStart(void);
uint8_t WIFI_SetSendOK(void);
uint8_t WIFI_SetSendFailed(void);

uint8_t WIFI_IsNetOK(void);
uint8_t* WIFI_PendPkt(OS_TICK pend_tmr, OS_MSG_SIZE* msg_size, OS_ERR* os_err);
static void WIFI_RxIrqCallBack(uint16_t);
void WIFI_TmrIrqHook(void);

uint8_t WIFI_ConnectDefault(void){
    return SUCCESS;
}

struct wifi_str_msg_t str_msg[] = {
    { .pchar = "WIFI DISCONNECT",.fuc = WIFI_ReConnectWifi },
    { .pchar = "ready"          ,.fuc = WIFI_ReConnectWifi },
    { .pchar = "CONNECTED"      ,.fuc = WIFI_ConnectDefault},
    
    { .pchar = "DISCONNECT"     ,.fuc = WIFI_ConnectServer },
    { .pchar = "CLOSED"         ,.fuc = WIFI_ConnectServer },
    
    { .pchar = ">"              ,.fuc = WIFI_SendStart     },
    { .pchar = "SEND OK"        ,.fuc = WIFI_SetSendOK     },
    { .pchar = "SEND FAILED"    ,.fuc = WIFI_SetSendFailed },
};
    
struct wifi_dr_t wifi_dr;
static OS_TCB    DR_WifiTaskTCB;
static CPU_STK   DR_TaskWIFStk[DR_TASK_WIFI_STK_SIZE];

static  void  DRTaskWifi          (void     *p_arg);

void WIFI_Init(void)
{
	OS_ERR os_err;

    BSP_OutPPIO_Init(IO_INIT(GPIOA, 1));
    NET_USART_INIT(115200, 11, WIFI_RxIrqCallBack);
    OSQCreate(&wifi_dr.msg_q, "dr/wifi_q", 50, &os_err);

    OSTaskCreate((OS_TCB       *)&DR_WifiTaskTCB,
                 (CPU_CHAR     *)"dr/wifi",
                 (OS_TASK_PTR   )DRTaskWifi,
                 (void         *)0u,
                 (OS_PRIO       )DR_TASK_WIFI_PRIO,
                 (CPU_STK      *)&DR_TaskWIFStk[0u],
                 (CPU_STK_SIZE  )DR_TaskWIFStk[DR_TASK_WIFI_STK_SIZE / 10u],
                 (CPU_STK_SIZE  )DR_TASK_WIFI_STK_SIZE,
                 (OS_MSG_QTY    )0u,
                 (OS_TICK       )0u,
                 (void         *)0u,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&os_err);
}

void WIFI_DataSend(uint8_t* ch, uint32_t len)
{
	for (uint32_t i=0; i<len; i++) {
	  	NET_USART_SEND(*(ch+i));
	}
}

uint8_t* WIFI_PendPkt(OS_TICK pend_tmr, OS_MSG_SIZE* msg_size, OS_ERR* os_err)
{
	CPU_TS      ts;
	return OSQPend(&wifi_dr.msg_q, pend_tmr, OS_OPT_PEND_BLOCKING, msg_size, &ts, os_err);
}

void WIFI_FlushQ(void)
{
    OS_ERR os_err;
    
    OSQFlush(&wifi_dr.msg_q, &os_err);
}

void WIFI_DataFree(void* ppkt)
{
	MEM_Free(ppkt);
	ppkt = NULL;
}

uint8_t WIFI_SetSSIDAndPswd(uint8_t* ssid, uint8_t* password)
{
    uint8_t* tmp_v = MEM_Malloc(100);
    int32_t len = sprintf((char*)tmp_v, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);
    WIFI_DataSend(tmp_v, len);
    MEM_Free(tmp_v);
    
    return SUCCESS;
}

uint8_t WIFI_ModeInit(void)
{
    OS_ERR err;
    
	//AP+STA mode : "AT+CWMODE=3\r\n";
    WIFI_DataSend("AT+CWMODE=3\r\n",strlen("AT+CWMODE=3\r\n"));
    OSTimeDly(100, OS_OPT_TIME_DLY, &err);

    WIFI_DataSend("AT+RST\r\n", strlen("AT+RST\r\n"));
    OSTimeDly(100, OS_OPT_TIME_DLY, &err);
//    WIFI_DataSend("AT+CIFSR\r\n", strlen("AT+CIFSR\r\n"));
//    OSTimeDly(500, OS_OPT_TIME_DLY, &err);

    // AT+CIPMODE=1
    WIFI_DataSend("AT+CIPMODE\r\n", strlen("AT+CIPMODE\r\n"));
    OSTimeDly(100, OS_OPT_TIME_DLY, &err);
    
    return SUCCESS;
}

void WIFI_NetStatusUpdate(uint8_t* pkt, uint32_t dlen)
{
    // net status update
    for (uint8_t i=0; i<sizeof(str_msg)/sizeof(str_msg[0]); i++) {
        CPU_CHAR* pch = Str_Str((const char*)pkt, str_msg[i].pchar);
        if (NULL != pch) {
            str_msg[i].fuc();
        }
    }
}

void WIFI_HWReset(void)
{
    OS_ERR err;
    
    BSP_IO_ResetBits(IO_CTR (GPIOA, 1));
    OSTimeDly(10, OS_OPT_TIME_DLY, &err);
    BSP_IO_SetBits(IO_CTR (GPIOA, 1));
    OSTimeDly(100, OS_OPT_TIME_DLY, &err);
}

uint8_t WIFI_IsNetOK(void)
{
    OS_ERR err;
    OS_MSG_SIZE msg_size;
    uint8_t count = 100;
    uint8_t* pkt = NULL;
    
    wifi_dr.status.is_wifi_connected = UNCONNECT;
    
    while(count--) {
        WIFI_DataSend("AT+CIPSTATUS\r\n", strlen("AT+CIPSTATUS\r\n"));
        while(NULL != (pkt = WIFI_PendPkt(100, &msg_size, &err))) {
            CPU_CHAR* pchar = Str_Str((const char*)pkt, "STATUS:2");
            WIFI_DataFree(pkt);
            pkt = NULL;
            if (NULL == pchar) {
                continue;
            }
            wifi_dr.status.is_wifi_connected = CONNECTED;
            return SUCCESS;
        }
        OSTimeDly(100, OS_OPT_TIME_DLY, &err);
    }
    
    return ERROR;
}

uint8_t WIFI_ReConnectWifi(void)
{
    OS_ERR err;
    
    wifi_dr.status.is_wifi_connected  = UNCONNECT;
    
    WIFI_HWReset();
    OSTimeDly(2000, OS_OPT_TIME_DLY, &err);
    WIFI_IsNetOK();
    
    WIFI_ConnectServer();
    
    return SUCCESS;
}

uint8_t WIFI_ConnectServer(void)
{
    OS_ERR err;
    OS_MSG_SIZE msg_size;
    uint8_t count = 10;
    uint8_t* pkt = NULL;
    //AT+CIPSTART="TCP","192.168.3.14",5000

    if (CONNECTED != wifi_dr.status.is_wifi_connected ) {
        return ERROR;
    }
    wifi_dr.status.is_login  = UNCONNECT;
    
    while(count--) {
        WIFI_DataSend("AT+CIPSTART=\"TCP\",\"183.230.40.39\",876\r\n", 
               strlen("AT+CIPSTART=\"TCP\",\"183.230.40.39\",876\r\n"));
        while(NULL != (pkt = WIFI_PendPkt(1000, &msg_size, &err))) {
            CPU_CHAR* pchar1 = Str_Str((const char*)pkt, "CONNECT");
            CPU_CHAR* pchar2 = Str_Str((const char*)pkt, "CONNECTED");
            if (NULL != Str_Str((const char*)pkt, "busy")) {
                OSTimeDly(10000, OS_OPT_TIME_DLY, &err);
            }
            
            WIFI_DataFree(pkt);
            pkt = NULL;
            if (NULL == pchar1 && NULL == pchar2) {
                continue;
            }
            wifi_dr.status.is_login  = CONNECTED;
            return SUCCESS;
        }
        OSTimeDly(5000, OS_OPT_TIME_DLY, &err);
    }
    
    wifi_dr.status.is_login  = UNCONNECT;
    
    return ERROR;
}

uint8_t WIFI_SendStart(void)
{
    OS_ERR err;
    
    if (NULL == wifi_dr.tx_pkt) {
        wifi_dr.status.is_send_ok = TX_IDLE;
        return ERROR;
    }
    
    wifi_dr.status.is_send_ok = TX_ING;
    WIFI_DataSend(wifi_dr.tx_pkt, wifi_dr.tx_len);
    MEM_Free(wifi_dr.tx_pkt);
    wifi_dr.tx_pkt = NULL;
    wifi_dr.tx_tmr_pro = OSTimeGet(&err);
    
    return SUCCESS;
}

uint8_t WIFI_SetSendOK(void)
{
    return wifi_dr.status.is_send_ok = TX_OK;
}

uint8_t WIFI_SetSendFailed(void)
{
    return wifi_dr.status.is_send_ok = TX_FAILED;
}

void WIFI_RxMsgCallBackInit(void(*payload_cb)(uint8_t* pchar, uint32_t len))
{
    wifi_dr.payload_cb = payload_cb;
}

uint8_t WIFI_SendMsg(uint8_t* pchar, uint32_t len)
{
    OS_ERR err;
    
    uint8_t tmp[20] = {0};
    //AT+CIPSEND=50
    if (CONNECTED != wifi_dr.status.is_wifi_connected || TX_IDLE != wifi_dr.status.is_send_ok) {
        return ERROR;
    }

    wifi_dr.tx_tmr_pro = OSTimeGet(&err);
    wifi_dr.tx_len = len;
    wifi_dr.tx_pkt = MEM_Malloc(len);
    if (NULL == wifi_dr.tx_pkt) {
        return ERROR;
    }
    Mem_Copy(wifi_dr.tx_pkt, pchar, wifi_dr.tx_len);
    int32_t tmp_len = sprintf((char*)tmp, "AT+CIPSEND=%d\r\n", len);
    WIFI_DataSend(tmp, tmp_len);
    wifi_dr.status.is_send_ok = TX_ING;
    
    return SUCCESS;
}

uint8_t WIFI_GetLogInServerStatus(void)
{
    return wifi_dr.status.is_login;
}

void WIFI_SetReConnectToServer(void)
{
    wifi_dr.status.is_login = UNCONNECT;
}

FunctionalState WIFI_IsTxEnable(void)
{
    if (TX_OK == wifi_dr.status.is_send_ok || TX_FAILED  == wifi_dr.status.is_send_ok ||
        TX_IDLE == wifi_dr.status.is_send_ok) {
        wifi_dr.status.is_send_ok = TX_IDLE;
        return ENABLE;
    }
    
    return DISABLE;
}

void  DRTaskWifi          (void     *p_arg)
{
	OS_ERR err;
    OS_MSG_SIZE msg_size = 0;
    uint8_t* pkt = NULL;
    uint32_t now_time = 0;
    
	wifi_dr.irq_rx_status = 0;
	wifi_dr.rx_tmr_pro    = OSTimeGet(&err);
    wifi_dr.rx_pkt        = NULL;
	wifi_dr.rx_count      = 0;

//    wifi_dr.is_send_ok    = TX_IDLE;
    wifi_dr.tx_pkt        = NULL;
    wifi_dr.tx_len        = 0;
    wifi_dr.tx_tmr_pro    = OSTimeGet(&err);

    wifi_dr.is_connected_tmr_pro = OSTimeGet(&err);;
	wifi_dr.status.word   = 0;
    
    wifi_init:
    NET_USART_IRQ_CTL(DISABLE);
    WIFI_FlushQ();
    WIFI_ModeInit();
    WIFI_HWReset();
    OSTimeDly(1000, OS_OPT_TIME_DLY, &err);
    
    WIFI_SetSSIDAndPswd("TP-LINK_2CF4_Main","zkh831121@");
    OSTimeDly(100, OS_OPT_TIME_DLY, &err);
    
    NET_USART_IRQ_CTL(ENABLE);
    if (ERROR == WIFI_IsNetOK()) {
        goto wifi_init;
    }
    
    if (ERROR == WIFI_ConnectServer()) {
        goto wifi_init;
    }

	while(1) {
        pkt = WIFI_PendPkt(100, &msg_size, &err);
        if (NULL != pkt) {
            WIFI_NetStatusUpdate(pkt, msg_size);
            // get message
            CPU_CHAR* pch = Str_Str((const char*)pkt, "+IPD");
            if (NULL != pch) {
                int32_t msg_len = atoi((const char*)(pch+7));
                uint8_t* ppayload = (uint8_t*)Str_Str((const char*)pkt, ":");
                // TODO:::Mem_Copy(IPD_BUF,ppayload+1,msg_len);消息队列将数据传出至APP层
                wifi_dr.payload_cb(ppayload+1, msg_len);
            }
            WIFI_DataFree(pkt);
            pkt = NULL;
        }
        else {
            OSTimeDly(100, OS_OPT_TIME_DLY, &err);
        }

        if (CONNECTED != wifi_dr.status.is_wifi_connected) {
            WIFI_ModeInit();
        }
        
        if (CONNECTED != wifi_dr.status.is_login) {
            WIFI_ConnectServer();
        }
        
        now_time = OSTimeGet(&err);
        if (TX_ING == wifi_dr.status.is_send_ok) {
            uint32_t value = (now_time>=wifi_dr.tx_tmr_pro)?(now_time-wifi_dr.tx_tmr_pro):(~0-wifi_dr.tx_tmr_pro+now_time);
            if (value > 20000) {
                wifi_dr.tx_tmr_pro = now_time;
                WIFI_SetSendFailed();
            }
        }
	}
}

/* called by usartx_rx_interrupt */
static void WIFI_RxIrqCallBack(uint16_t value)
{
	OS_ERR os_err;
	
	OS_TICK now_tmr = OSTimeGet(&os_err);
	if (OS_ERR_NONE != os_err) {
		return;
	}

	if (0 == wifi_dr.irq_rx_status) {
		wifi_dr.rx_pkt = MEM_Malloc(RX_MAX_LEN);
		if (NULL == wifi_dr.rx_pkt) {
			return;
		}
		wifi_dr.rx_count = 0;
		wifi_dr.irq_rx_status = 1;
	}

	wifi_dr.rx_tmr_pro  = now_tmr;
	*(wifi_dr.rx_pkt+wifi_dr.rx_count++) = value;
	if (wifi_dr.rx_count >= RX_MAX_LEN) {
		OSQPost(&wifi_dr.msg_q, wifi_dr.rx_pkt, RX_MAX_LEN, OS_OPT_POST_FIFO, &os_err);
		wifi_dr.irq_rx_status = 0;
	}
}

/* called by os_time_tick_hook */
void WIFI_TmrIrqHook(void)
{
	OS_ERR os_err;
	OS_TICK tmr_value = 0;

	if (0 == wifi_dr.irq_rx_status) {
		return;
	}
	
	OS_TICK now_tmr = OSTimeGet(&os_err);
	if (OS_ERR_NONE != os_err) {
		return;
	}

	tmr_value = (now_tmr>=wifi_dr.rx_tmr_pro)?(now_tmr-wifi_dr.rx_tmr_pro):(~0-wifi_dr.rx_tmr_pro+now_tmr);
	if (tmr_value >= RX_TMROUT) {
		OSQPost(&wifi_dr.msg_q, wifi_dr.rx_pkt, wifi_dr.rx_count, OS_OPT_POST_FIFO, &os_err);
		wifi_dr.irq_rx_status = 0;
	}
}
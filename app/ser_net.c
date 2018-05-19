#include "app_cfg.h"
#include  "dr_inc.h"
#include  <includes.h>
#include <os_app_hooks.h>
#include "Common.h"

#define  S_IDLE                                       0
#define  S_DO_ING                                     1
#define  S_DONE                                       2
#define  S_FAILED                                     3

#define  APP_NET_TASK_STK_SIZE                        256u
#define  TIME_OUT_TO_UP_MSG                           10000
#define  TIME_OUT_TO_PING                             60000

#define  MAX_WIDGET                                   12
#define  DEV_ID                                       "25986684"

static  OS_TCB       App_TaskNetTCB;
static  CPU_STK      App_TaskNetSTK[APP_NET_TASK_STK_SIZE];

static  void  App_TaskNet (void  *p_arg);
static  void GetPayloadMsg_CallBack(uint8_t* pchar, uint32_t len);

uint8_t test_tcb[1024];

struct events_t {
    uint8_t fd;
    uint8_t status;
    uint8_t (*ev_cb)(uint8_t* pchar, uint32_t* dlen);
};

struct widget_t {
    uint8_t   is_flag;
    uint8_t*  name;
    uint8_t*  bt_op_v;
    uint8_t*  show_v;
};

struct net_ser_t {
    edp_pkt* send_pkg;
    EdpPacket* send_pkg1;
    struct widget_t* pwidget;
    uint8_t is_oprate;
    
    uint32_t sensor_pro_tmr;
    
    uint8_t is_net_ser;
    uint32_t is_net_pro_tmr;
    uint32_t ping_failed_cnt;
};

//struct events_t events {
//    
//}

struct widget_t widget[] = {
    0, "Infrared_lamp1", "led1on"  , "440101",
    0, "Infrared_lamp1", "led1off" , "440100",
    0, "Infrared_lamp2", "led2on"  , "440201",
    0, "Infrared_lamp2", "led2off" , "440200",
    0, "Infrared_lamp3", "led3on"  , "440301",
    0, "Infrared_lamp4", "led3off" , "440300",
    0, "switch_lamp"   , "81"      , "81",
    0, "switch_lamp"   , "1"       , "1",
    0, "switch_lamp"   , "82"      , "82",
    0, "switch_lamp"   , "2"       , "2",
    0, "switch_lamp"   , "83"      , "83",
    0, "switch_lamp"   , "3"       , "3",
};

struct net_ser_t net_ser = {
    .send_pkg = NULL,
    .send_pkg1 = NULL,
    .is_oprate = S_IDLE,
    .is_net_ser = S_IDLE,
    .pwidget = widget,
};

void  AppObjCreate (void)
{
    OS_ERR  os_err;
    
    /* ------------- CREATE FLOATING POINT TASK ----------- */
    OSTaskCreate((OS_TCB      *)&App_TaskNetTCB,
                 (CPU_CHAR    *)"app/net",
                 (OS_TASK_PTR  ) App_TaskNet, 
                 (void        *) 0,
                 (OS_PRIO      ) APP_CFG_TASK_NET_PRIO,
                 (CPU_STK     *)&App_TaskNetSTK[0],
                 (CPU_STK_SIZE ) App_TaskNetSTK[APP_NET_TASK_STK_SIZE / 10u],
                 (CPU_STK_SIZE ) APP_NET_TASK_STK_SIZE,
                 (OS_MSG_QTY   ) 0u,
                 (OS_TICK      ) 0u,
                 (void        *) 0,
                 (OS_OPT       )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
                 (OS_ERR      *)&os_err);
}

void EV_LogInServer(void)
{
    net_ser.send_pkg1 = PacketConnect1(DEV_ID, "6yCaaQ8809p7uww4QG7X3cq0GUU=");
    WIFI_SendMsg(net_ser.send_pkg1->_data, net_ser.send_pkg1->_write_pos);
    while(ENABLE != WIFI_IsTxEnable());
    
    MEM_Free(net_ser.send_pkg1->_data);
    net_ser.send_pkg1->_data = NULL;
    MEM_Free(net_ser.send_pkg1);
    net_ser.send_pkg1 = NULL;
}

void EV_IsTmrToCheckNet(uint32_t now_tmr, uint32_t tmr_out)
{
    if (S_FAILED == net_ser.is_net_ser) {
        DR_LED_Off(4u);
        WIFI_SetReConnectToServer();
        while(CONNECTED != WIFI_GetLogInServerStatus());
        EV_LogInServer();
        DR_LED_On(4u);
        
        net_ser.is_net_ser = S_IDLE;
        net_ser.is_net_pro_tmr = now_tmr;
        net_ser.ping_failed_cnt = 0;
    }
}

uint8_t EV_IsTmrToPing(uint32_t now_tmr, uint32_t tmr_out)
{
    uint32_t tmr = 0;
    
    tmr = (now_tmr>=net_ser.is_net_pro_tmr)?(now_tmr-net_ser.is_net_pro_tmr):(~0-net_ser.is_net_pro_tmr+now_tmr);
    if (tmr<=tmr_out) {
        return ERROR;
    }
    
    net_ser.is_net_pro_tmr = now_tmr;
    
    if (0 == net_ser.ping_failed_cnt) {
        net_ser.ping_failed_cnt ++;
        net_ser.is_net_ser = S_DO_ING;
        DR_LED_Off(4u);
    }
    else {
        net_ser.ping_failed_cnt ++;
        if (net_ser.ping_failed_cnt > 10) {
            net_ser.ping_failed_cnt = 0;
            net_ser.is_net_ser = S_FAILED;
            return ERROR;
        }
    }

    net_ser.send_pkg1 = PacketPing();
    WIFI_SendMsg(net_ser.send_pkg1->_data, net_ser.send_pkg1->_write_pos);
    while(ENABLE != WIFI_IsTxEnable());
    
    MEM_Free(net_ser.send_pkg1->_data);
    net_ser.send_pkg1->_data = NULL;
    MEM_Free(net_ser.send_pkg1);
    net_ser.send_pkg1 = NULL;
    
    return SUCCESS;
}

uint8_t EV_IsTmrToUpDateMsg(uint32_t now_tmr, uint32_t tmr_out)
{
    uint32_t sensor_tmr_out = 0;
    uint8_t value1[2], value2[2];
    uint8_t v_char[10];
    
    sensor_tmr_out = (now_tmr>=net_ser.sensor_pro_tmr)?(now_tmr-net_ser.sensor_pro_tmr):(~0-net_ser.sensor_pro_tmr+now_tmr);
    if (sensor_tmr_out<=TIME_OUT_TO_UP_MSG) {
        return ERROR;
    }
    
    net_ser.sensor_pro_tmr = now_tmr;
    
    uint8_t* pvalue_pkg = JsonT1_PacketToInit(500);
    JsonT1_PacketToHeader(pvalue_pkg);

    if (SUCCESS == GetHumitureMsg(value1, value2)) {
        sprintf((char*)v_char, "%d.%d", value1[0], value1[1]);
        JsonT1_PacketAddParaValue(pvalue_pkg, "temperature", v_char,DISABLE);
        
        sprintf((char*)v_char, "%d.%d", value2[0], value2[1]);
        JsonT1_PacketAddParaValue(pvalue_pkg, "humidity", v_char,DISABLE);
    }
    
    if (SUCCESS == GetGaspMsg(value1)) {
        Mem_Set(v_char, '\0', sizeof(v_char));
        sprintf((char*)v_char, "%d.%d", value1[0], value1[1]);
        JsonT1_PacketAddParaValue(pvalue_pkg, "gasp", v_char,DISABLE);
    }
    if (SUCCESS == GetBrightnessMsg(value1)) {
        Mem_Set(v_char, '\0', sizeof(v_char));
        sprintf((char*)v_char, "%d.%d", value1[0], value1[1]);
        JsonT1_PacketAddParaValue(pvalue_pkg, "brightness", v_char, DISABLE);
    }
    JsonT1_PacketAddParaValue(pvalue_pkg, "waring_msg", "80",ENABLE);
    JsonT1_PacketToEnder(pvalue_pkg);

    net_ser.send_pkg = JsonT1_packetDataSaveTrans(pvalue_pkg, DEV_ID);
    if (NULL == net_ser.send_pkg) {
        goto failed;
    }
    WIFI_SendMsg(net_ser.send_pkg->data, net_ser.send_pkg->len);
    while(ENABLE != WIFI_IsTxEnable());

    MEM_Free(net_ser.send_pkg);
    net_ser.send_pkg = NULL;
failed:
    MEM_Free(pvalue_pkg);
    pvalue_pkg = NULL;
    
    return SUCCESS;
}

uint8_t EV_IsWidgetEvents(uint32_t now_tmr, uint32_t tmr_out)
{
    if (net_ser.is_oprate != S_DONE) {
        return ERROR;
    }
    
    net_ser.is_oprate = S_IDLE;
        
    uint8_t* pvalue_pkg = JsonT1_PacketToInit(500);
    JsonT1_PacketToHeader(pvalue_pkg);
    
    for (uint8_t i=0; i<MAX_WIDGET; i++) {
        if (S_DONE == net_ser.pwidget[i].is_flag) {
            net_ser.pwidget[i].is_flag = S_IDLE;
            
            JsonT1_PacketAddParaValue(pvalue_pkg, net_ser.pwidget[i].name, net_ser.pwidget[i].show_v,DISABLE);
        }
    }
    
    JsonT1_PacketAddParaValue(pvalue_pkg, "waring_msg", "81",ENABLE);
    
    JsonT1_PacketToEnder(pvalue_pkg);

    net_ser.send_pkg = JsonT1_packetDataSaveTrans(pvalue_pkg, DEV_ID);
    if (NULL == net_ser.send_pkg) {
        goto failed;
    }
    WIFI_SendMsg(net_ser.send_pkg->data, net_ser.send_pkg->len);
    while(ENABLE != WIFI_IsTxEnable());
    
    MEM_Free(net_ser.send_pkg);
    net_ser.send_pkg = NULL;
failed:
    MEM_Free(pvalue_pkg);
    pvalue_pkg = NULL;
    
    return SUCCESS;
}

static void  App_TaskNet (void  *p_arg)
{
    OS_ERR  os_err;
    uint32_t now_tmr = OSTimeGet(&os_err);
    
    net_ser.sensor_pro_tmr = OSTimeGet(&os_err);
    net_ser.is_net_pro_tmr = OSTimeGet(&os_err);
    
    WIFI_RxMsgCallBackInit(GetPayloadMsg_CallBack);
    
    while (CONNECTED != WIFI_GetLogInServerStatus()) {
        OSTimeDlyHMSM(0u, 0u, 0u, 100u, OS_OPT_TIME_HMSM_STRICT, &os_err);
    }
    EV_LogInServer();
    net_ser.ping_failed_cnt = 0;
    DR_LED_On(4u);
    
    while (DEF_TRUE) {
        now_tmr = OSTimeGet(&os_err);
        
        EV_IsTmrToCheckNet(now_tmr, 0);
        
        if (SUCCESS == EV_IsTmrToUpDateMsg(now_tmr, TIME_OUT_TO_UP_MSG)) {
            DR_LED_On(2u);
            OSTimeDly(10, OS_OPT_TIME_DLY, &os_err);
            DR_LED_Off(2u);
        }
        
        if (SUCCESS == EV_IsTmrToPing(now_tmr, TIME_OUT_TO_PING)) {
            DR_LED_On(2u);
            OSTimeDly(10, OS_OPT_TIME_DLY, &os_err);
            DR_LED_Off(2u);
        }
        
        if (SUCCESS == EV_IsWidgetEvents(now_tmr, 0)) {
            DR_LED_On(2u);
            OSTimeDly(10, OS_OPT_TIME_DLY, &os_err);
            DR_LED_Off(2u);
        }
        
        OSTimeDly(100, OS_OPT_TIME_DLY, &os_err);
    }
}

edp_pkt rx_pkt;
char cmd[1024];
char id[1024]; 
static void GetPayloadMsg_CallBack(uint8_t* pchar, uint32_t len)
{
    int32_t rmlen = 0 , id_len = 0, cmd_len = 0;
    
    rx_pkt.len = len;
    rx_pkt.read_p = 0;
    Mem_Copy(rx_pkt.data, pchar, rx_pkt.len);
    
    net_ser.is_net_ser = S_IDLE;
    net_ser.ping_failed_cnt = 0;
    
    if (1 != isEdpPkt(&rx_pkt)) {
        return;
    }

    edpCommandReqParse(&rx_pkt, id, cmd, &rmlen, &id_len, &cmd_len);
    for (uint8_t i=0; i<MAX_WIDGET; i++) {
        if (NULL != strstr(cmd, (const char*)net_ser.pwidget[i].bt_op_v)) {
            net_ser.is_oprate = S_DONE;
            net_ser.pwidget[i].is_flag = S_DONE;
        }
    }
}
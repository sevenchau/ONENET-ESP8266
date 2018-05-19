#include "app_cfg.h"
#include  "dr_inc.h"
#include  <includes.h>
#include  <os_app_hooks.h>
#include  "Common.h"

#define  APP_TASK_EQ_0_ITERATION_NBR              16u

struct sensor_t {
    uint8_t is_sensor_ok;
    uint8_t humidity[2];
    uint8_t temperature[2];
};

static  void  App_TaskEq0Fp         (void  *p_arg);             /* Floating Point Equation 0 task.                      */

static  OS_TCB       App_TaskEq0FpTCB;
static  CPU_STK      App_TaskEq0FpStk[APP_CFG_TASK_EQ_STK_SIZE];

struct sensor_t sensor;

uint8_t GetHumitureMsg(uint8_t temperature[2], uint8_t humidity[2])
{
    OS_ERR os_err;
    
    srand(OSTimeGet(&os_err)*5);
    humidity[0] = 50+rand()%20;
    humidity[1] = rand()%99;
    srand(OSTimeGet(&os_err)*6);
    temperature[0] = 20+rand()%10;
    temperature[1] = rand()%99;
//    if (1 == sensor.is_sensor_ok) {
//        Mem_Copy(humidity, sensor.humidity, sizeof(sensor.humidity));
//        Mem_Copy(temperature, sensor.temperature, sizeof(sensor.temperature));
//        sensor.is_sensor_ok = 0;
        
        return SUCCESS;
//    }
    
//    return ERROR;
}

uint8_t GetGaspMsg(uint8_t gasp[2])
{
    OS_ERR os_err;

    srand(OSTimeGet(&os_err));
    gasp[0] = 100+rand()%9;
    srand(OSTimeGet(&os_err)*2);
    gasp[1] = rand()%99;
    
    return SUCCESS;
}

uint8_t GetBrightnessMsg(uint8_t brightness[2])
{
    OS_ERR os_err;

    srand(OSTimeGet(&os_err)*3);
    brightness[0] = 50+rand()%50;
    srand(OSTimeGet(&os_err)*4);
    brightness[1] = rand()%99;
    
    return SUCCESS;
}

// read the sensor messages
void  App_TaskEq0Fp (void  *p_arg)
{
    CPU_FP32    a;
    CPU_FP32    b;
    CPU_FP32    c;
    CPU_FP32    eps;
    CPU_FP32    f_a;
    CPU_FP32    f_c;
    CPU_FP32    delta;
    CPU_INT08U  iteration;
    RAND_NBR    wait_cycles;

    OS_ERR  err;
    
    while (DEF_TRUE) {
        if (SUCCESS == DHT11_ReadMsg(sensor.humidity, sensor.temperature)) {
            sensor.is_sensor_ok = 1;
        }
        eps       = 0.00001;
        a         = 3.0; 
        b         = 4.0;
        delta     = a - b;
        iteration = 0u;
        if (delta < 0) {
            delta = delta * -1.0;
        }
        
        while (((2.00 * eps) < delta) || 
               (iteration    > 20u  )) {
            c   = (a + b) / 2.00;
            f_a = (exp((-1.0) * a) * (3.2 * sin(a) - 0.5 * cos(a)));
            f_c = (exp((-1.0) * c) * (3.2 * sin(c) - 0.5 * cos(c)));
            
            if (((f_a > 0.0) && (f_c < 0.0)) || 
                ((f_a < 0.0) && (f_c > 0.0))) {
                b = c;
            } else if (((f_a > 0.0) && (f_c > 0.0)) || 
                       ((f_a < 0.0) && (f_c < 0.0))) {
                a = c;           
            } else {
                break;
            }
                
            delta = a - b;
            if (delta < 0) {
               delta = delta * -1.0;
            }
            iteration++;

            wait_cycles = Math_Rand();
            wait_cycles = wait_cycles % 1000;

            while (wait_cycles > 0u) {
                wait_cycles--;
            }

            if (iteration > APP_TASK_EQ_0_ITERATION_NBR) {
                APP_TRACE_INFO(("App_TaskEq0Fp() max # iteration reached\n"));
                break;
            }            
        }

        APP_TRACE_INFO(("Eq0 Task Running ....\n"));
        
        if (iteration == APP_TASK_EQ_0_ITERATION_NBR) {
            APP_TRACE_INFO(("Root = %f; f(c) = %f; #iterations : %d\n", c, f_c, iteration));
        }
        
        OSTimeDlyHMSM(0u, 0u, 0u, 100u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }
}

void  AppDataAcqCreate (void)
{
    OS_ERR  os_err;
    
                                                                /* ------------- CREATE FLOATING POINT TASK ----------- */
    OSTaskCreate((OS_TCB      *)&App_TaskEq0FpTCB,
                 (CPU_CHAR    *)"FP  Equation 1",
                 (OS_TASK_PTR  ) App_TaskEq0Fp, 
                 (void        *) 0,
                 (OS_PRIO      ) APP_CFG_TASK_EQ_PRIO,
                 (CPU_STK     *)&App_TaskEq0FpStk[0],
                 (CPU_STK_SIZE ) App_TaskEq0FpStk[APP_CFG_TASK_EQ_STK_SIZE / 10u],
                 (CPU_STK_SIZE ) APP_CFG_TASK_EQ_STK_SIZE,
                 (OS_MSG_QTY   ) 0u,
                 (OS_TICK      ) 0u,
                 (void        *) 0,
                 (OS_OPT       )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
                 (OS_ERR      *)&os_err);
}

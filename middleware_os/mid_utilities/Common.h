#ifndef __COMMON_H__
#define __COMMON_H__
//
///*---------------------------------------------------------------------------*/
///* Type Definition Macros                                                    */
///*---------------------------------------------------------------------------*/
//#ifndef __WORDSIZE
//  /* Assume 32 */
//  #define __WORDSIZE 32
//#endif
//
//#if defined(_LINUX) || defined (WIN32)
//    typedef unsigned char   uint8;
//    typedef char            int8;
//    typedef unsigned short  uint16;
//    typedef short           int16;
//    typedef unsigned int    uint32;
//    typedef int             int32;
//#endif
//
//#ifdef WIN32
//    typedef int socklen_t;
//#endif
//
//#if defined(WIN32)
//    typedef unsigned long long int  uint64;
//    typedef long long int           int64;
//#elif (__WORDSIZE == 32)
//    __extension__
//    typedef long long int           int64;
//    __extension__
//    typedef unsigned long long int  uint64;
//#elif (__WORDSIZE == 64)
//    typedef unsigned long int       uint64;
//    typedef long int                int64;
//#endif

#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include  "os.h"
#include "stm32f4xx.h"
#include "mem_malloc.h"

typedef unsigned char   uint8;
typedef char            int8;
typedef unsigned short  uint16;
typedef short           int16;
typedef unsigned int    uint32;
typedef int             int32;

typedef long long int           int64;
typedef unsigned long long int  uint64;

#define MAX_LEN         (MEM_MAX_SIZE-8)

typedef struct
{
  uint8 data[MAX_LEN];
  int16 len;
  int16 read_p;
} edp_pkt;

edp_pkt *JsonT5_packetDataSaveTrans(const int8* destId, const int8* streamId, const int8 *val);

uint8_t* JsonT1_PacketToInit(uint32_t v_len);
uint8_t* JsonT1_PacketToHeader(uint8_t* pvalue_pkt);
uint8_t* JsonT1_PacketAddParaValue(uint8_t* pvalue_pkt, uint8_t* id, uint8_t* value, FunctionalState is_last);
uint8_t* JsonT1_PacketToEnder(uint8_t* pvalue_pkt);
edp_pkt* JsonT1_packetDataSaveTrans(uint8_t* pvalue_pkt, const int8* destId);

int16 isEdpPkt(edp_pkt* pkt);
int edpCommandReqParse(edp_pkt* pkt, char *id, char *cmd, int32 *rmlen, int32 *id_len, int32 *cmd_len);
int edpPushDataParse(edp_pkt* pkt, char *srcId, char *data);

#endif /* __COMMON_H__ */

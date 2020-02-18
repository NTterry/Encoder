

#ifndef _ENCODER_H_
#define _ENCODER_H_
#include "stdint.h"
#define UINT32_EXTEND                               //32位扩展

#define MAXCOUNT            30000                   //最大计数值
#define ENCODER_TIM_PERIOD  (MAXCOUNT * 2)          //自动重装值

#define SPTIMEOUT           600                     // 速度检测超时时间  600ms

#ifdef UINT32_EXTEND
#endif

//测速状态机
typedef enum {
    SP_INIT = 0,
    SP_CALU,
    SP_REC,
    SP_DLY = 101,         // 最长测量时间 ms
}SPEED_STATE;

/*编码长度，速度和加速度存储*/
typedef  struct
{
    SPEED_STATE Sta;
    uint16_t    LastResCnt;         //寄存器值
	int32_t     TotalCnt;           //总计数值
    int32_t     TotalCnt1;          //备份计数器值
    uint32_t    LastTimMs;          //上次记录的时间
    int32_t     PerTimMs;           //当前记录时间
    int32_t     PerTick;
    int32_t     Speed;              // 10倍  n/s
    int32_t     Acce;               // n/s^2
}ENCODER;

/*接口函数*/
void Enc_Clr_TotalCnt(void); 
void Enc_Clr_TotalCnt1(void); 
int32_t Enc_Get_Speed(void);
int32_t Enc_Get_Acce(void);


/*对于局部函数，不允许在其他文件中直接操作，必须使用函数接口的方式进行操作*/
#endif

/******************************************************************************/
/** 模块名称：增量编码器读取                                                   **/
/** 文件名称：Encoder.h                                                       **/
/** 版    本：V1.0.0                                                          **/
/** 简    介：用于获取编码器参数值                                              **/
/**           1、清除增量编码器数值                                             **/
/**           2、清除增量编码器数值1                                            **/
/**           3、得到编码器速度                                                 **/
/**           4、得到编码器加速度                                               **/
/**-------------------------------------------------------------------------- **/
/** 修改记录：                                                                 **/
/**     版本      日期              作者              说明                     **/
/**     V1.0.0  2016-04-17          Terry            创建文件                 **/
/**                                                                          **/
/******************************************************************************/
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
    uint32_t    PerTimMs;           //时间间隔值
    int32_t     PerTick;            //计数间隔值
    int32_t     Speed;              // 10倍  n/s
    int32_t     Acce;               // n/s^2
}ENCODER;

/*接口函数*/
void Enc_Clr_TotalCnt(void); 
void Enc_Clr_TotalCnt1(void); 
int32_t Enc_Get_Speed(void);
int32_t Enc_Get_Acce(void);
void Enc_Set_Dir(int32_t dir);     //设置 0不改变  1改变


/*对于局部函数，不允许在其他文件中直接操作，必须使用函数接口的方式进行操作*/
#endif
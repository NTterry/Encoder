#include "Encoder.h"

ENCODER  Encoder1Data;
static int32_t s_dir = 0;

/************************************************************
  * @brief   求出两次时间的差值
  * @param   t1:上一次时间  t2:后来的时间（新）
  * @return  时间的差值
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    NULL
  ***********************************************************/
uint32_t Enc_DiffTimes(uint32_t t1, uint32_t t2)
{
    uint32_t res;

    if(t2 < t1)
    {
        res = 0xFFFFFFFF - t1 + t2;
    }
    else
    {
        /* code */
        res = t2 - t1;
    }
    return res;
}
/************************************************************
  * @brief   两个编码器脉冲计数值解析
  * @param   none
  * @return  计数（带正负号） int32
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    间隔时间T内，计数值不能超过MAXCOUNT
  *          假设T = 0.1秒，每秒计数最大值为MAXCOUNT * 10
  ***********************************************************/
int32_t Enc_DiffCnt(uint16_t c1,uint16_t c2)
{
	int32_t Angle = c2 - c1;
	
	if(Angle > MAXCOUNT)
	{
		Angle -= ENCODER_TIM_PERIOD;
	}
	if(Angle < -MAXCOUNT)
	{
		Angle += ENCODER_TIM_PERIOD;
	}
  /*修改计数方向，同时会修改速度和加速度的方向  Terry 2020.2.18*/
  if(s_dir)
    Angle = -Angle;

	return -Angle; 
}


/* TIM4 init function  ±àÂëÆ÷½Ó¿Ú */
void MX_TIM4_Init(void)
{
    TIM_Encoder_InitTypeDef sConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 0;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = ENCODER_TIM_PERIOD - 1;;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
    sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC1Filter = 5;
    sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC2Filter = 5;
    if (HAL_TIM_Encoder_Init(&htim4, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);			/*开启编码器计数*/
}


void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef* tim_encoderHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(tim_encoderHandle->Instance==TIM4)
  {
  /* USER CODE BEGIN TIM4_MspInit 0 */

  /* USER CODE END TIM4_MspInit 0 */
    /* TIM4 clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();
  
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**TIM4 GPIO Configuration    
    PB6     ------> TIM4_CH1
    PB7     ------> TIM4_CH2 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  /* USER CODE BEGIN TIM4_MspInit 1 */
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);        //复用为外部中断引脚
  /* USER CODE END TIM4_MspInit 1 */
  }
}


/************************************************************
  * @brief   编码器CNT1外部中断
  * @param   GPIO_Pin:引脚号
  * @return  none
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    中断函数
  ***********************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{	
	uint16_t TmpCnt;
	uint32_t TmpTim;
	int32_t  TmpSp;
	
	/***********************************触发*************************************/
	if(GPIO_Pin == GPIO_PIN_7)					
	{
		switch (Encoder1Data.Sta)
        {
            case SP_INIT:
                Encoder1Data.LastTimMs = osKernelSysTick();    			    //获得当前系统时间
                Encoder1Data.LastResCnt = __HAL_TIM_GET_COUNTER(&htim4);  	//得到当前计数器值    
                HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);          
                Encoder1Data.Sta = SP_DLY;							        // 关闭中断 等待100ms
                break;

            case SP_CALU:
                TmpCnt = __HAL_TIM_GET_COUNTER(&htim4);
                TmpTim = osKernelSysTick(); 
                Encoder1Data.PerTimMs = Enc_DiffTimes(Encoder1Data.LastTimMs,TmpTim);		//间隔时间	
                Encoder1Data.PerTick = Enc_DiffCnt(Encoder1Data.LastResCnt,TmpCnt);      // 间隔脉冲数
                /*此处累计编码计数*/
                Encoder1Data.TotalCnt += Encoder1Data.PerTick;
                Encoder1Data.TotalCnt1 += Encoder1Data.PerTick;
                TmpSp = Encoder1Data.PerTick * 2500 / Encoder1Data.PerTimMs;             //速度计算 10倍的测量速度，原来4倍的脉冲数
                /*与上一次的速度比较，计算加速度的值*/
                Encoder1Data.Acce = 100 * (TmpSp - Encoder1Data.Speed)/Encoder1Data.PerTimMs;   // ticks / (s * s) 
                Encoder1Data.Speed = TmpSp;			
                /*保存当前值*/
                Encoder1Data.LastTimMs = TmpTim;
                Encoder1Data.LastResCnt = TmpCnt;  					
                HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);                      //关闭外部中断
                Encoder1Data.Sta = SP_DLY;	
                break;	
            default:
                break;	
        }
	}
}


/************************************************************
  * @brief   编码器计算,每毫秒调用一次
  * @param   none
  * @return  none
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    最好放在操作系统的系统时钟回调函数中执行
  ***********************************************************/
void EncoderCacuMs(void)
{
    uint32_t tmp;

	if(Encoder1Data.Sta == SP_REC)						
	{
		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);					/*¿ªÆô±àÂëÆ÷Òý½ÅÍâ²¿ÖÐ¶Ï*/
        Encoder1Data.Sta = SP_CALU;								/*¿ªÊ¼¼ì²â*/		
	}
    if(Encoder1Data.Sta > SP_REC)
	{
        Encoder1Data.Sta--;
	}
    
    tmp = osKernelSysTick();
    if(Def_Times(Encoder1Data.LastTimMs,tmp) >= SPTIMEOUT )    		// 如果超时，将触发错误信号
    {
        uint16_t counter;
        // 编码绝对值
		    counter = __HAL_TIM_GET_COUNTER(&htim4); 
        Encoder1Data.PerTick = Enc_DiffCnt(Encoder1Data.LastResCnt,counter);

        Encoder1Data.TotalCnt += Encoder1Data.PerTick;
        Encoder1Data.TotalCnt1 += Encoder1Data.PerTick;
        Encoder1Data.LastResCnt = counter;
        Encoder1Data.LastTimMs = tmp;
        //计算速度
        Encoder1Data.Speed = Encoder1Data.PerTick * 2500 / SPTIMEOUT;
        Encoder1Data.Acce = 0;
        Encoder1Data.Sta = SP_DELAY100MS;
		    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    }
}
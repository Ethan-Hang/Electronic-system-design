#include "walkmotor.h"
#include "delay.h"


// 定义舵机角度
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180
#define SERVO_CENTER_ANGLE 90

// SG90舵机专用PWM参数 (使用TIM2)
// 周期 = 72M/(1000*72) = 20ms
#define PWM_PERIOD 1000         // ARR值
#define PWM_MIN_PULSE 50        // 1ms对应的CCR值: 1000*1ms/20ms = 50
#define PWM_MAX_PULSE 100       // 2ms对应的CCR值: 1000*2ms/20ms = 100  
#define PWM_CENTER_PULSE 75     // 1.5ms对应的CCR值: 1000*1.5ms/20ms = 75

void Walkmotor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    
    // 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);  // TIM2时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // GPIOB时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  // AFIO时钟，重映射必需
    
    // 配置TIM2重映射 - 完全重映射
    // TIM2完全重映射后：CH1->PA15, CH2->PB3, CH3->PB10, CH4->PB11
    GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);
    
    // 配置GPIO - PB11对应重映射后的TIM2_CH4
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 定时器基本配置
    // 周期为72M/(1000*72) = 20ms
    TIM_TimeBaseStructure.TIM_Period = (1000-1);       // ARR = 999
    TIM_TimeBaseStructure.TIM_Prescaler = (72-1);      // PSC = 71, 得到1MHz计数频率
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    // PWM输出配置 - 使用通道4
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = PWM_MIN_PULSE;    // 初始化到0度
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC4Init(TIM2, &TIM_OCInitStructure);  // 使用通道4
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
    
    TIM_Cmd(TIM2, ENABLE);
    
    delay_ms(100);  // 初始化稳定延时
}

// 设置舵机角度函数
void Servo_SetAngle(uint16_t angle)
{
    uint16_t pulse_width;
    
    // 限制角度范围
    if(angle > SERVO_MAX_ANGLE) 
        angle = SERVO_MAX_ANGLE;
    
    // 计算对应的CCR值
    // 角度0度 -> CCR=50, 角度180度 -> CCR=100
    pulse_width = PWM_MIN_PULSE + (angle * (PWM_MAX_PULSE - PWM_MIN_PULSE)) / SERVO_MAX_ANGLE;
    
    // 设置PWM占空比 - 使用TIM2通道4
    TIM_SetCompare4(TIM2, pulse_width);
}

// 舵机转到开启位置 (180度)
void Walkmotor_ON(void)
{
    Servo_SetAngle(180);  // 转到180度位置
    delay_ms(1000);
}

// 舵机转到关闭位置 (0度)  
void Walkmotor_OFF(void)
{
    Servo_SetAngle(0);    // 转到0度位置
    delay_ms(1000);
}

// 舵机转到中间位置 (90度)
void Walkmotor_CENTER(void)
{
    Servo_SetAngle(90);   // 转到90度位置
    delay_ms(1000);
}


























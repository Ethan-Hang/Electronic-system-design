#include "stm32f10x.h"
#include "SR04.h"
#include "Delay.h"
#include "Timer.h"

extern uint32_t a;  // 引用Timer.c中定义的变量
uint8_t echo_state = 0;  // 0: 等待回波, 1: 正在测量
uint32_t echo_duration = 0;  // 回波时间(微秒)

void SR04_Init(void)
{
    // 使能GPIOB时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    // 配置触发引脚(PB9)为输出
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 配置回波引脚(PB8)为输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 初始状态: 触发引脚为低电平
    GPIO_ResetBits(GPIOB, GPIO_Pin_9);
}

void Exti_Init(void)
{
    // 使能AFIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    
    // 连接EXTI线到回波引脚
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8);
    
    // 配置EXTI线
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line8;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    
    // 配置NVIC中断优先级
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line8) == SET)
    {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8)) // 上升沿
        {
            a = 0;  // 重置溢出计数器
            TIM_SetCounter(TIM3, 0);  // 清零计数器
            TIM_Cmd(TIM3, ENABLE);    // 启动定时器
            echo_state = 1;
        }
        else if (echo_state == 1) // 下降沿
        {
            TIM_Cmd(TIM3, DISABLE);   // 停止定时器
            
            // 计算总时间(微秒): 每个溢出65536us + 当前计数器值
            echo_duration = a * 65536 + TIM_GetCounter(TIM3);
            echo_state = 0;
        }
        EXTI_ClearITPendingBit(EXTI_Line8);  // 清除中断标志位
    }
}

void SR04_Start(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_9);    // 触发引脚设置为高电平
    Delay_us(10);                       // 保持高电平10μs
    GPIO_ResetBits(GPIOB, GPIO_Pin_9);  // 触发引脚设置为低电平
}

uint32_t SR04_GetDistance(void)
{
    uint32_t distance;
    
    // 重置测量值
    echo_duration = 0;
    echo_state = 0;
    
    // 开始测量
    SR04_Start();
    
    // 等待测量完成(最多等待30ms)
    uint32_t timeout = 30;  
    while (echo_state == 1 || (echo_state == 0 && echo_duration == 0))
    {
        Delay_ms(1);
        timeout--;
        if (timeout == 0) break;  // 超时退出
    }
    
    // 计算距离(单位:厘米)
    // 声速约340m/s，那么声音在空气中传播1cm需要29.4us
    // 距离(cm) = 回波时间(us) * 0.017 (考虑声波往返)
    if (echo_duration > 0 && echo_duration < 23529) {  // 最大4米(约23529us)
        distance = echo_duration / 58;  // 换算成厘米 (精确到1cm)
    } else {
        distance = 0;  // 无效测量
    }
    
    return distance; // 返回距离(厘米)
}

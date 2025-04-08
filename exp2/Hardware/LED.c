#include "stm32f10x.h"

void LED_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //使能GPIOA时钟
    GPIO_InitTypeDef GPIO_InitStructure; //定义GPIO初始化结构体
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15; //选择引脚
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //设置引脚速度
    GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化GPIOB15引脚
}

void LED_Flash(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_15); //点亮LED
    Delay_ms(500); //延时500ms
    GPIO_SetBits(GPIOB, GPIO_Pin_15); //熄灭LED
    Delay_ms(500); //延时500ms
}

void LED_Set(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_15); //点亮LED
}

void LED_Clr(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_15); //熄灭LED
}
#include "stm32f10x.h"

void Buffer_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //使能GPIOB时钟
    GPIO_InitTypeDef GPIO_InitStructure; //定义GPIO初始化结构体
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //设置为推挽输出模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; //选择引脚
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //设置引脚速度
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化GPIOB0、GPIOB1、GPIOB2引脚
}

void Buffer_Set(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_7); //设置GPIOB0、GPIOB1、GPIOB2引脚为高电平
}

void Buffer_Clr(void)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_7); //设置GPIOB0、GPIOB1、GPIOB2引脚为低电平
}
#include "stm32f10x.h" // Device header
#include "stm32f10x_rcc.h"
#include "Delay.h"

void GPIO_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //使能GPIOA时钟
    GPIO_InitTypeDef GPIO_InitStructure; //定义GPIO初始化结构体
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15; //选择引脚
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //设置引脚速度
    GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化GPIOB15引脚
}

void GPIO_Register(void)
{
    // 1. 使能 GPIOB 时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    // 2. 配置 PB15 为推挽输出，50MHz
    GPIOB->CRH &= ~(GPIO_CRH_CNF15 | GPIO_CRH_MODE15);  // 清除原有配置
    GPIOB->CRH |= GPIO_CRH_MODE15;  // 输出模式，50MHz (0b11)
    // CNF15 已被清零，对应推挽输出（0b00）
}

int main()
{
    // GPIO_init(); //初始化GPIOB15引脚
    GPIO_Register(); //注册GPIOB15引脚
    while (1)
    {
        // GPIO_ResetBits(GPIOB, GPIO_Pin_15); //设置GPIOB15引脚为高电平
        
        // GPIOB->BSRR = GPIO_BSRR_BR15;  // PB15 = 0
        // GPIOB->BSRR = GPIO_BSRR_BS15;  // PB15 = 1
    } 
}

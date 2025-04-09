#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "OLED.h"
#include "Timer.h"
#include "stm32f10x_it.h"

void SysTick_Init(void)
{
    // SysTick配置为1ms中断
    SysTick_Config(SystemCoreClock);
}



int main()
{
    OLED_Init();
    // Timer_Init();
    SysTick_Init();
    while (1)
    {
        OLED_ShowNum(1, 1, a, 5);
    } 
}


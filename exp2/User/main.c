#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Buffer.h"
#include <stdbool.h>
#include "Keyboard.h"

bool ledflash = false; //LED闪烁状态

int main()
{
    // Key_Init();
    // LED_Init();
    // Buffer_Init();
    Keyboard_Init(); // 初始化键盘
    OLED_Init(); // 初始化OLED
    while (1)
    {
        uint8_t keyValue = Keyboard_Scan(); // 扫描键盘
        // OLED_ShowNum(1, 1, keyValue, 2); // 显示按键值
        // Delay_ms(100); // 延时100ms
        // if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12) == 0) //按键按下
        // {
        //     Delay_ms(2);
        //     while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12) == 0); //等待按键释放
        //     Delay_ms(2);
        //     ledflash = !ledflash; //切换LED闪烁状态
        // }
        // if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) == 0) //按键按下
        // {
        //     Delay_ms(2);
        //     while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) == 0); //等待按键释放
        //     Delay_ms(2);
        //     ledflash = !ledflash; //切换LED闪烁状态
        // }

        // if (ledflash) //LED闪烁状态为真
        // {
        //     LED_Flash(); //调用LED闪烁函数
        // }
        // else //LED闪烁状态为假
        // {
        //     LED_Set(); //点亮LED
        // }
        // // Buffer_Set(); //设置GPIOB0、GPIOB1、GPIOB2引脚为高电平
        // // Buffer_Clr(); //设置GPIOB0、GPIOB1、GPIOB2引脚为低电平
    } 
}

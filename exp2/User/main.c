#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Buffer.h"
#include <stdbool.h>
#include "Keyboard.h"
#include "Key.h"

bool ledflash = false; // LED闪烁状态

void Task1(void);
void Task2(void);

int main()
{
    Key_Init();
    LED_Init();
    Buffer_Init();
    Keyboard_Init(); // 初始化键盘
    OLED_Init();     // 初始化OLED
    while (1)
    {
        Task1(); // 执行任务1
        // Task2(); // 执行任务2
    }
}

void Task1(void)
{
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12) == 0) // 按键按下
    {
        Delay_ms(2);
        while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12) == 0)
            ; // 等待按键释放
        Delay_ms(2);
        ledflash = !ledflash; // 切换LED闪烁状态
    }
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) == 0) // 按键按下
    {
        Delay_ms(2);
        while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) == 0)
            ; // 等待按键释放
        Delay_ms(2);
        ledflash = !ledflash; // 切换LED闪烁状态
    }

    if (ledflash) // LED闪烁状态为真
    {
        LED_Flash(); // 调用LED闪烁函数
    }
    else // LED闪烁状态为假
    {
        LED_Set(); // 点亮LED
    }
}

void Task2(void)
{
    int8_t keyValue = Keyboard_Scan(); // 扫描键盘
    switch (keyValue)
    {
    case -1:
        break;
    case 0x0A:
        OLED_ShowString(1, 1, "A"); // 显示键值
        break;
    case 0x0B:
        OLED_ShowString(1, 1, "B"); // 显示键值
        break;
    case 0x0C:
        OLED_ShowString(1, 1, "C"); // 显示键值
        break;
    case 0x0D:
        OLED_ShowString(1, 1, "D"); // 显示键值
        break;
    case 0x2A:
        OLED_ShowString(1, 1, "*"); // 显示键值
        break;
    case 0x23:
        OLED_ShowString(1, 1, "#"); // 显示键值
        break;
    default:
        OLED_ShowNum(1, 1, keyValue, 1); // 显示键值
        break;
    }
}

#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "OLED.h"

void OLED_ShowBasicInfo(void);
void OLED_ShowTemHum(void);

int main()
{
    OLED_Init();
    while (1)
    {
        //显示班级名字学号
        // OLED_ShowBasicInfo();
        OLED_ShowTemHum();

    }
}

void OLED_ShowBasicInfo(void)
{
    for (uint8_t i = 0; i <= 2; i++)
    {
        OLED_ShowCN(1, i + 1, i);
    }
    OLED_ShowNum(1, 7, 233, 3);
    
    for (uint8_t i = 3; i <= 5; i++)
    {
        OLED_ShowCN(2, i - 2, i);
    }
    OLED_ShowNum(2, 7, 3230515068, 10);
    for (uint8_t i = 6; i <= 7; i++)
    {
        OLED_ShowCN(3, i - 5, i);
    }
    OLED_ShowNum(3, 5, 3230621101, 10);
}

void OLED_ShowTemHum(void)
{
    for (uint8_t i = 8; i <= 11; i++)
    {
        OLED_ShowCN(1, i - 7, i);
    }
    OLED_ShowChar(1, 9, ':'); 
    OLED_ShowNum(1, 10, 10, 3);
    OLED_ShowCN(1, 7, 12);

    for (uint8_t i = 13; i <= 16; i++)
    {
        OLED_ShowCN(2, i - 12, i);
    }
    OLED_ShowChar(2, 9, ':'); 
    OLED_ShowNum(2, 10, 10, 3);
    OLED_ShowChar(2, 13, '%');
}

#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "OLED.h"
#include "DHT11.h"

void OLED_ShowBasicInfo(void);
void OLED_ShowTemHum(void);

int main()
{
    OLED_Init();
    while (1)
    {
        // 显示班级名字学号
        //  OLED_ShowBasicInfo();
        // 显示温湿度
        OLED_ShowTemHum();
    }
}

void OLED_ShowBasicInfo(void)
{
    // 自动化
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
    DHT11_REC_Data(); // 获取数据

    for (uint8_t i = 8; i <= 11; i++)
    {
        OLED_ShowCN(1, i - 7, i);
    }
    OLED_ShowChar(1, 9, ':');
    OLED_ShowNum(1, 10, rec_data[2], 2); // 温度整数部分
    OLED_ShowChar(1, 12, '.');
    OLED_ShowNum(1, 13, rec_data[3], 2); // 温度小数部分
    OLED_ShowCN(1, 8, 12);               // 显示摄氏度符号

    for (uint8_t i = 13; i <= 16; i++)
    {
        OLED_ShowCN(2, i - 12, i);
    }
    OLED_ShowChar(2, 9, ':');
    OLED_ShowNum(2, 10, rec_data[0], 2); // 湿度整数部分
    OLED_ShowChar(2, 12, '.');
    OLED_ShowNum(2, 13, rec_data[1], 2); // 湿度小数部分
    OLED_ShowChar(2, 15, '%');

    Delay_s(1); // 延时1秒
}

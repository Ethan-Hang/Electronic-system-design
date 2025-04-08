#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "DH11.h"
#include "OLED.h"

int main()
{
    OLED_Init();
    DHT11_Start(); // 发送开始信号
    Delay_ms(1000); // 等待DHT11准备好
    while (1)
    {  
        OLED_ShowNum(1, 1, rec_data[0], 2); // 显示温度整数部分
        OLED_ShowNum(1, 2, rec_data[1], 2); // 显示温度小数部分 
        OLED_ShowNum(1, 3, rec_data[2], 2); // 显示湿度整数部分
        OLED_ShowNum(1, 4, rec_data[3], 2); // 显示湿度小数部分
        Delay_ms(1000); // 延时1秒
    } 
}

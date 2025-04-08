#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "OLED.h"

int main()
{
    OLED_Init();
    while (1)
    {
        OLED_ShowChar(1, 1, 'a');
        OLED_ShowString(1, 3, "Hello World!");
        OLED_ShowNum(2, 1, 12345, 4);
        OLED_ShowSignedNum(2, 7, -66, 2);
        OLED_ShowHexNum(3, 1, 0xAA55, 4);
        OLED_ShowBinNum(4, 1, 0xAA55, 16);
    } 
}

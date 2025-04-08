#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "OLED.h"

int main()
{
    OLED_Init();
    while (1)
    {
        // OLED_ShowChar(1, 1, 'a');
        // OLED_ShowString(1, 3, "Hello World!");
        // OLED_ShowNum(2, 1, 12345, 4);
        // OLED_ShowSignedNum(2, 7, -66, 2);
        // OLED_ShowHexNum(3, 1, 0xAA55, 4);
        // OLED_ShowBinNum(4, 1, 0xAA55, 16);

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
}

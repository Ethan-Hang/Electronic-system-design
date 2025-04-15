#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Timer.h"
#include "SR04.h"

uint32_t distance_cm; // 存储厘米单位的距离

void SysTick_Init(void)
{
    SysTick_Config(SystemCoreClock / 1000); // 1ms 中断
}

int main(void)
{
    OLED_Init();
    SR04_Init();
    Exti_Init();
    Timer_Init();
    SysTick_Init(); // 启用 SysTick 用于延时

    // 显示标题和单位
    OLED_ShowString(1, 1, "Distance:");
    OLED_ShowString(2, 4, "cm");
    
    while (1)
    {
        // 进行多次测量并取平均值
        uint32_t total_distance = 0;
        uint8_t valid_count = 0;
        
        for (int i = 0; i < 5; i++)
        {
            uint32_t temp = SR04_GetDistance(); // 获取单次距离(厘米)
            if (temp > 0 && temp <= 400) // 有效距离0-400cm
            {
                total_distance += temp;
                valid_count++;
            }
            Delay_ms(20); // 测量间隔
        }
        
        // 计算平均值
        if (valid_count > 0)
        {
            distance_cm = total_distance / valid_count;
        }
        else
        {
            distance_cm = 0; // 无有效测量
        }
        
        // 清除原显示区域
        OLED_ShowString(2, 1, "   ");
        
        // 显示新测量结果(精确到厘米)
        OLED_ShowNum(2, 1, distance_cm, 3); // 显示距离(厘米)，最多3位数
        
        // 等待刷新间隔，确保每秒显示一次
        Delay_ms(1000 - 5*20); // 1秒减去前面已用的时间
    }
}

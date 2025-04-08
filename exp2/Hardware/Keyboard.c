#include "stm32f10x.h"
#include "delay.h"
#include "oled.h"

// 定义行引脚 (输出)
#define ROW1_PIN GPIO_Pin_0   // PB0
#define ROW2_PIN GPIO_Pin_1   // PB1
#define ROW3_PIN GPIO_Pin_11  // PA11
#define ROW4_PIN GPIO_Pin_3   // PB3

// 定义列引脚 (输入)
#define COL1_PIN GPIO_Pin_4   // PB4
#define COL2_PIN GPIO_Pin_5   // PB5
#define COL3_PIN GPIO_Pin_6   // PB6
#define COL4_PIN GPIO_Pin_7   // PB7
#define COL_PORT GPIOB
#define COL_PINS (COL1_PIN | COL2_PIN | COL3_PIN | COL4_PIN)

// 键值映射表
const uint8_t keymap[4][4] = {
    {1, 2, 3, 4},    // 第一行
    {5, 6, 7, 8},    // 第二行
    {9, 10, 11, 12}, // 第三行
    {13, 14, 15, 16} // 第四行
};

// 键盘初始化函数
void Keyboard_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能GPIOA、GPIOB和AFIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    
    // 禁用JTAG，保留SWD，释放PB3、PB4、PA15引脚
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    
    // 配置PB0、PB1、PB3为推挽输出
    GPIO_InitStructure.GPIO_Pin = ROW1_PIN | ROW2_PIN | ROW4_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 配置PA11为推挽输出
    GPIO_InitStructure.GPIO_Pin = ROW3_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 配置列引脚为上拉输入
    GPIO_InitStructure.GPIO_Pin = COL_PINS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // 上拉输入
    GPIO_Init(COL_PORT, &GPIO_InitStructure);
    
    // 初始状态，所有行输出高电平
    GPIO_SetBits(GPIOB, ROW1_PIN | ROW2_PIN | ROW4_PIN);
    GPIO_SetBits(GPIOA, ROW3_PIN);
}

// 键盘扫描函数
uint8_t Keyboard_Scan(void)
{
    uint8_t keyValue = 0;
    uint16_t colStatus;
    
    // 先检查是否有按键按下（所有列是否有低电平）
    colStatus = GPIO_ReadInputData(COL_PORT) & COL_PINS;
    
    // 在OLED上显示列状态以进行调试
    OLED_ShowHexNum(1, 1, colStatus, 4);
    
    if (colStatus != COL_PINS) // 如果有低电平，表示有按键按下
    {
        // 去抖
        Delay_ms(10);
        colStatus = GPIO_ReadInputData(COL_PORT) & COL_PINS;
        if (colStatus != COL_PINS) // 再次确认
        {
            // 第一行扫描 (PB0)
            GPIO_SetBits(GPIOB, ROW1_PIN | ROW2_PIN | ROW4_PIN);
            GPIO_SetBits(GPIOA, ROW3_PIN);
            GPIO_ResetBits(GPIOB, ROW1_PIN); // 第一行置低
            Delay_ms(1); // 等待电平稳定
            
            colStatus = GPIO_ReadInputData(COL_PORT) & COL_PINS;
            
            // 显示第一行扫描结果
            OLED_ShowString(2, 1, "R1:");
            OLED_ShowHexNum(2, 4, colStatus, 4);
            
            if (colStatus != COL_PINS) // 第一行有按键按下
            {
                if ((colStatus & COL1_PIN) == 0) keyValue = keymap[0][0];
                else if ((colStatus & COL2_PIN) == 0) keyValue = keymap[0][1];
                else if ((colStatus & COL3_PIN) == 0) keyValue = keymap[0][2];
                else if ((colStatus & COL4_PIN) == 0) keyValue = keymap[0][3];
                
                // 等待按键释放
                while ((GPIO_ReadInputData(COL_PORT) & COL_PINS) != COL_PINS);
                Delay_ms(10); // 去抖
                
                // 恢复所有行为高电平
                GPIO_SetBits(GPIOB, ROW1_PIN | ROW2_PIN | ROW4_PIN);
                GPIO_SetBits(GPIOA, ROW3_PIN);
                
                return keyValue;
            }
            
            // 第二行扫描 (PB1)
            GPIO_SetBits(GPIOB, ROW1_PIN | ROW2_PIN | ROW4_PIN);
            GPIO_SetBits(GPIOA, ROW3_PIN);
            GPIO_ResetBits(GPIOB, ROW2_PIN); // 第二行置低
            Delay_ms(1); // 等待电平稳定
            
            colStatus = GPIO_ReadInputData(COL_PORT) & COL_PINS;
            
            // 显示第二行扫描结果
            OLED_ShowString(3, 1, "R2:");
            OLED_ShowHexNum(3, 4, colStatus, 4);
            
            if (colStatus != COL_PINS) // 第二行有按键按下
            {
                if ((colStatus & COL1_PIN) == 0) keyValue = keymap[1][0];
                else if ((colStatus & COL2_PIN) == 0) keyValue = keymap[1][1];
                else if ((colStatus & COL3_PIN) == 0) keyValue = keymap[1][2];
                else if ((colStatus & COL4_PIN) == 0) keyValue = keymap[1][3];
                
                // 等待按键释放
                while ((GPIO_ReadInputData(COL_PORT) & COL_PINS) != COL_PINS);
                Delay_ms(10); // 去抖
                
                // 恢复所有行为高电平
                GPIO_SetBits(GPIOB, ROW1_PIN | ROW2_PIN | ROW4_PIN);
                GPIO_SetBits(GPIOA, ROW3_PIN);
                
                return keyValue;
            }
            
            // 第三行扫描 (PA11)
            GPIO_SetBits(GPIOB, ROW1_PIN | ROW2_PIN | ROW4_PIN);
            GPIO_SetBits(GPIOA, ROW3_PIN);
            GPIO_ResetBits(GPIOA, ROW3_PIN); // 第三行置低
            Delay_ms(1); // 等待电平稳定
            
            colStatus = GPIO_ReadInputData(COL_PORT) & COL_PINS;
            
            // 显示第三行扫描结果
            OLED_ShowString(4, 1, "R3:");
            OLED_ShowHexNum(4, 4, colStatus, 4);
            
            if (colStatus != COL_PINS) // 第三行有按键按下
            {
                if ((colStatus & COL1_PIN) == 0) keyValue = keymap[2][0];
                else if ((colStatus & COL2_PIN) == 0) keyValue = keymap[2][1];
                else if ((colStatus & COL3_PIN) == 0) keyValue = keymap[2][2];
                else if ((colStatus & COL4_PIN) == 0) keyValue = keymap[2][3];
                
                // 等待按键释放
                while ((GPIO_ReadInputData(COL_PORT) & COL_PINS) != COL_PINS);
                Delay_ms(10); // 去抖
                
                // 恢复所有行为高电平
                GPIO_SetBits(GPIOB, ROW1_PIN | ROW2_PIN | ROW4_PIN);
                GPIO_SetBits(GPIOA, ROW3_PIN);
                
                return keyValue;
            }
            
            // 第四行扫描 (PB3)
            GPIO_SetBits(GPIOB, ROW1_PIN | ROW2_PIN | ROW4_PIN);
            GPIO_SetBits(GPIOA, ROW3_PIN);
            GPIO_ResetBits(GPIOB, ROW4_PIN); // 第四行置低
            Delay_ms(1); // 等待电平稳定
            
            colStatus = GPIO_ReadInputData(COL_PORT) & COL_PINS;
            
            // 显示第四行扫描结果
            OLED_ShowString(4, 8, "R4:");
            OLED_ShowHexNum(4, 11, colStatus, 4);
            
            if (colStatus != COL_PINS) // 第四行有按键按下
            {
                if ((colStatus & COL1_PIN) == 0) keyValue = keymap[3][0];
                else if ((colStatus & COL2_PIN) == 0) keyValue = keymap[3][1];
                else if ((colStatus & COL3_PIN) == 0) keyValue = keymap[3][2];
                else if ((colStatus & COL4_PIN) == 0) keyValue = keymap[3][3];
                
                // 等待按键释放
                while ((GPIO_ReadInputData(COL_PORT) & COL_PINS) != COL_PINS);
                Delay_ms(10); // 去抖
                
                // 恢复所有行为高电平
                GPIO_SetBits(GPIOB, ROW1_PIN | ROW2_PIN | ROW4_PIN);
                GPIO_SetBits(GPIOA, ROW3_PIN);
                
                return keyValue;
            }
        }
    }
    
    // 恢复所有行为高电平
    GPIO_SetBits(GPIOB, ROW1_PIN | ROW2_PIN | ROW4_PIN);
    GPIO_SetBits(GPIOA, ROW3_PIN);
    
    return 0; // 没有按键按下，返回0
}

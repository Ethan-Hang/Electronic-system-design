#include "stm32f10x.h"
#include "oled.h"
#include "delay.h"
#include "buffer.h"

// 定义行引脚（输出）
#define ROW1_PIN GPIO_Pin_0   // PB0
#define ROW2_PIN GPIO_Pin_1   // PB1
#define ROW3_PIN GPIO_Pin_11  // PA11
#define ROW4_PIN GPIO_Pin_3   // PB3

// 定义列引脚（输入）
#define COL1_PIN GPIO_Pin_4   // PB4
#define COL2_PIN GPIO_Pin_5   // PB5
#define COL3_PIN GPIO_Pin_6   // PB6
#define COL4_PIN GPIO_Pin_7   // PB7
#define COL_PORT GPIOB
#define COL_PINS (COL1_PIN | COL2_PIN | COL3_PIN | COL4_PIN)

// 键值映射表
const uint8_t keymap[4][4] = {
    {1, 2, 3, 0x0A},    // 第1行
    {4, 5, 6, 0x0B},    // 第2行
    {7, 8, 9, 0x0C}, // 第3行
    {0x2A, 0, 0x23, 0x0D} // 第4行
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
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 配置PA11为推挽输出
    GPIO_InitStructure.GPIO_Pin = ROW3_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 配置列引脚为上拉输入
    GPIO_InitStructure.GPIO_Pin = COL_PINS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
    GPIO_Init(COL_PORT, &GPIO_InitStructure);
    
    // 初始状态：所有行高电平
    GPIO_SetBits(GPIOB, ROW1_PIN | ROW2_PIN | ROW4_PIN);
    GPIO_SetBits(GPIOA, ROW3_PIN);
    
    // 初始化OLED并清屏
    OLED_Init();
    OLED_Clear();
}

// 键盘扫描函数
int8_t Keyboard_Scan(void)
{
    uint8_t keyValue = 0;
    uint16_t colStatus;
    
    // 扫描第1行（PB0）
    GPIO_SetBits(GPIOB, ROW2_PIN | ROW4_PIN);
    GPIO_SetBits(GPIOA, ROW3_PIN);
    GPIO_ResetBits(GPIOB, ROW1_PIN); // 第1行置低
    Delay_ms(1);
    colStatus = GPIO_ReadInputData(COL_PORT) & COL_PINS;
    if (colStatus != COL_PINS)
    {
        if ((colStatus & COL1_PIN) == 0) keyValue = keymap[0][0];
        else if ((colStatus & COL2_PIN) == 0) keyValue = keymap[0][1];
        else if ((colStatus & COL3_PIN) == 0) keyValue = keymap[0][2];
        else if ((colStatus & COL4_PIN) == 0) keyValue = keymap[0][3];
        while ((GPIO_ReadInputData(COL_PORT) & COL_PINS) != COL_PINS); // 等待释放
        Delay_ms(10); // 去抖
        GPIO_SetBits(GPIOB, ROW1_PIN);
        // OLED_Clear(); // 清屏
        // OLED_ShowNum(1, 1, keyValue, 2); // 显示键值
        Buffer_Once(); // 触发一次缓冲器
        return keyValue;
    }
    
    // 扫描第2行（PB1）
    GPIO_SetBits(GPIOB, ROW1_PIN | ROW4_PIN);
    GPIO_SetBits(GPIOA, ROW3_PIN);
    GPIO_ResetBits(GPIOB, ROW2_PIN); // 第2行置低
    Delay_ms(1);
    colStatus = GPIO_ReadInputData(COL_PORT) & COL_PINS;
    if (colStatus != COL_PINS)
    {
        if ((colStatus & COL1_PIN) == 0) keyValue = keymap[1][0];
        else if ((colStatus & COL2_PIN) == 0) keyValue = keymap[1][1];
        else if ((colStatus & COL3_PIN) == 0) keyValue = keymap[1][2];
        else if ((colStatus & COL4_PIN) == 0) keyValue = keymap[1][3];
        while ((GPIO_ReadInputData(COL_PORT) & COL_PINS) != COL_PINS);
        Delay_ms(10);
        GPIO_SetBits(GPIOB, ROW2_PIN);
        // OLED_Clear(); // 清屏
        // OLED_ShowNum(1, 1, keyValue, 2); // 显示键值
        Buffer_Once(); // 触发一次缓冲器
        return keyValue;
    }
    
    // 扫描第3行（PA11）
    GPIO_SetBits(GPIOB, ROW1_PIN | ROW2_PIN | ROW4_PIN);
    GPIO_ResetBits(GPIOA, ROW3_PIN); // 第3行置低
    Delay_ms(1);
    colStatus = GPIO_ReadInputData(COL_PORT) & COL_PINS;
    if (colStatus != COL_PINS)
    {
        if ((colStatus & COL1_PIN) == 0) keyValue = keymap[2][0];
        else if ((colStatus & COL2_PIN) == 0) keyValue = keymap[2][1];
        else if ((colStatus & COL3_PIN) == 0) keyValue = keymap[2][2];
        else if ((colStatus & COL4_PIN) == 0) keyValue = keymap[2][3];
        while ((GPIO_ReadInputData(COL_PORT) & COL_PINS) != COL_PINS);
        Delay_ms(10);
        GPIO_SetBits(GPIOA, ROW3_PIN);
        // OLED_Clear(); // 清屏
        // OLED_ShowNum(1, 1, keyValue, 2); // 显示键值
        Buffer_Once(); // 触发一次缓冲器
        return keyValue;
    }
    
    // 扫描第4行（PB3）
    GPIO_SetBits(GPIOB, ROW1_PIN | ROW2_PIN);
    GPIO_SetBits(GPIOA, ROW3_PIN);
    GPIO_ResetBits(GPIOB, ROW4_PIN); // 第4行置低
    Delay_ms(1);
    colStatus = GPIO_ReadInputData(COL_PORT) & COL_PINS;
    if (colStatus != COL_PINS)
    {
        if ((colStatus & COL1_PIN) == 0) keyValue = keymap[3][0];
        else if ((colStatus & COL2_PIN) == 0) keyValue = keymap[3][1];
        else if ((colStatus & COL3_PIN) == 0) keyValue = keymap[3][2];
        else if ((colStatus & COL4_PIN) == 0) keyValue = keymap[3][3];
        while ((GPIO_ReadInputData(COL_PORT) & COL_PINS) != COL_PINS);
        Delay_ms(10);
        GPIO_SetBits(GPIOB, ROW4_PIN);
        // OLED_Clear(); // 清屏
        // OLED_ShowNum(1, 1, keyValue, 2); // 显示键值
        Buffer_Once(); // 触发一次缓冲器
        return keyValue;
    }
    
    // 恢复所有行高电平
    GPIO_SetBits(GPIOB, ROW1_PIN | ROW2_PIN | ROW4_PIN);
    GPIO_SetBits(GPIOA, ROW3_PIN);
    return -1; // 无按键按下
}

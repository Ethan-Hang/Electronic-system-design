#include "walkmotor.h"
#include "delay.h"


// �������Ƕ�
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180
#define SERVO_CENTER_ANGLE 90

// SG90���ר��PWM���� (ʹ��TIM2)
// ���� = 72M/(1000*72) = 20ms
#define PWM_PERIOD 1000         // ARRֵ
#define PWM_MIN_PULSE 50        // 1ms��Ӧ��CCRֵ: 1000*1ms/20ms = 50
#define PWM_MAX_PULSE 100       // 2ms��Ӧ��CCRֵ: 1000*2ms/20ms = 100  
#define PWM_CENTER_PULSE 75     // 1.5ms��Ӧ��CCRֵ: 1000*1.5ms/20ms = 75

void Walkmotor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    
    // ʹ��ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);  // TIM2ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // GPIOBʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  // AFIOʱ�ӣ���ӳ�����
    
    // ����TIM2��ӳ�� - ��ȫ��ӳ��
    // TIM2��ȫ��ӳ���CH1->PA15, CH2->PB3, CH3->PB10, CH4->PB11
    GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);
    
    // ����GPIO - PB11��Ӧ��ӳ����TIM2_CH4
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // ��ʱ����������
    // ����Ϊ72M/(1000*72) = 20ms
    TIM_TimeBaseStructure.TIM_Period = (1000-1);       // ARR = 999
    TIM_TimeBaseStructure.TIM_Prescaler = (72-1);      // PSC = 71, �õ�1MHz����Ƶ��
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    // PWM������� - ʹ��ͨ��4
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = PWM_MIN_PULSE;    // ��ʼ����0��
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC4Init(TIM2, &TIM_OCInitStructure);  // ʹ��ͨ��4
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
    
    TIM_Cmd(TIM2, ENABLE);
    
    delay_ms(100);  // ��ʼ���ȶ���ʱ
}

// ���ö���ǶȺ���
void Servo_SetAngle(uint16_t angle)
{
    uint16_t pulse_width;
    
    // ���ƽǶȷ�Χ
    if(angle > SERVO_MAX_ANGLE) 
        angle = SERVO_MAX_ANGLE;
    
    // �����Ӧ��CCRֵ
    // �Ƕ�0�� -> CCR=50, �Ƕ�180�� -> CCR=100
    pulse_width = PWM_MIN_PULSE + (angle * (PWM_MAX_PULSE - PWM_MIN_PULSE)) / SERVO_MAX_ANGLE;
    
    // ����PWMռ�ձ� - ʹ��TIM2ͨ��4
    TIM_SetCompare4(TIM2, pulse_width);
}

// ���ת������λ�� (180��)
void Walkmotor_ON(void)
{
    Servo_SetAngle(180);  // ת��180��λ��
    delay_ms(1000);
}

// ���ת���ر�λ�� (0��)  
void Walkmotor_OFF(void)
{
    Servo_SetAngle(0);    // ת��0��λ��
    delay_ms(1000);
}

// ���ת���м�λ�� (90��)
void Walkmotor_CENTER(void)
{
    Servo_SetAngle(90);   // ת��90��λ��
    delay_ms(1000);
}


























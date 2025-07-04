#include "beep.h"
#include "delay.h"
//初始化PB8为输出口.并使能这个口的时钟		    
//蜂鸣器初始化
void BEEP_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能GPIOB端口时钟
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;				 //BEEP-->PB.8 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);	 //根据参数初始化GPIOB.8
 
  GPIO_SetBits(GPIOB,GPIO_Pin_8);//输出0，关闭蜂鸣器输出
 
    GPIO_ResetBits(GPIOB,GPIO_Pin_9);//BEEP OFF
}



void beep_on_mode1(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_8);//BEEP OFF GPIO_SetBits
	delay_ms(600);
	GPIO_SetBits(GPIOB,GPIO_Pin_8);//BEEP ON
}


void beep_on_mode2(void)
{
	
	int i;
	
	for(i=0; i<5; i++)
	{
	
	GPIO_ResetBits(GPIOB,GPIO_Pin_8);//BEEP ON
	delay_ms(200);
	GPIO_SetBits(GPIOB,GPIO_Pin_8);//BEEP OFF
	delay_ms(200);
	}
	
	
}

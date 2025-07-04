#include "sys.h"
#include "usart.h"	  
#include "string.h"	  
#include "oled.h"	   
#include "delay.h"	  
#include "stdio.h"     // 添加stdio.h支持sprintf
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif


//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序

u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  
  
void uart_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1 

}

void USART1_IRQHandler(void)                	//串口1中断服务程序
	{
	u8 Res;
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
		Res =USART_ReceiveData(USART1);	//读取接收到的数据
		
		if((USART_RX_STA&0x8000)==0)//接收未完成
			{
			if(USART_RX_STA&0x4000)//接收到了0x0d
				{
					if(Res!=':')USART_RX_STA=0;//接收错误,重新开始0x0a
				else USART_RX_STA|=0x8000;	//接收完成了 
				}
			else //还没收到0X0D
				{	
				if(Res=='!')USART_RX_STA|=0x4000;//0x0d
				else
					{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
					}		 
				}
			}   		 
     } 
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
} 
#endif

//检查字符串是否与接收数据匹配 (保留原有蓝牙功能)
u8 usart1_cherk(char *buf)
{
	static u16 ErrCnt = 0;
	if(memcmp(USART_RX_BUF,buf,6)==0)	{
		USART_RX_STA=0;
		memset(USART_RX_BUF,0,strlen((const char *)USART_RX_BUF));
		ErrCnt = 0;
		return 0;
	}else if(memcmp(USART_RX_BUF,"LOCK",4)==0)
	{
		USART_RX_STA=0;
		memset(USART_RX_BUF,0,USART_REC_LEN);
		ErrCnt = 0;
		return 1;
	}
	else 
	{
		if((USART_RX_STA&0xff) > 0)
		{
			int i;
			printf("接收数据：");
			for(i =0; i<(USART_RX_STA & 0xff); i++)
			printf("%c",USART_RX_BUF[i]);
			printf("\r\n");
			ErrCnt++;
			if(USART_RX_STA & 0x8000)//接收完成
			{
				USART_RX_STA = 0;
				OLED_Clear_NOupdate();
				Show_Str(12,13,128,12,"蓝牙密码：错误！",12,0); 
				OLED_Refresh_Gram();//更新显示
				delay_ms(800);
			}
		}
		if(ErrCnt > 5)//超时
		{
			USART_RX_STA=0;
			memset(USART_RX_BUF,0,USART_REC_LEN);
			ErrCnt = 0;
		}
		return 1;
	}
}

void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
	USART_SendData(pUSARTx,ch);        /*   发送数据到USART   */        
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);        /*   等待发送完成   */
}

// 修复原有函数的bug
void Usart_SendString( USART_TypeDef * pUSARTx, char *str)
{
	unsigned int k=0;
	do 
	{
		Usart_SendByte( pUSARTx, *(str + k) );  // 修复：使用传入的pUSARTx参数
		k++;
	} while(*(str + k)!='\0');
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET); // 修复：使用正确的USART              
}

// 发送AT指令（带超时等待响应）
// 参数：cmd - AT指令字符串，timeout_ms - 超时时间（毫秒）
// 返回值：0-成功，1-超时，2-响应错误
u8 Send_AT_Command(char *cmd, u16 timeout_ms)
{
    u16 wait_time = 0;
    
    // 清空接收缓冲区
    USART_RX_STA = 0;
    memset(USART_RX_BUF, 0, USART_REC_LEN);
    
    // 发送AT指令
    Usart_SendString(USART1, cmd);
    Usart_SendString(USART1, "\r\n");  // AT指令通常以\r\n结尾
    
    // 等待响应
    while(wait_time < timeout_ms)
    {
        if(USART_RX_STA & 0x8000)  // 接收完成
        {
            // 检查是否收到OK响应
            if(strstr((char*)USART_RX_BUF, "OK") != NULL)
            {
                USART_RX_STA = 0;
                return 0;  // 成功
            }
            // 检查是否收到ERROR响应
            else if(strstr((char*)USART_RX_BUF, "ERROR") != NULL || 
                    strstr((char*)USART_RX_BUF, "FAIL") != NULL)
            {
                USART_RX_STA = 0;
                return 2;  // 响应错误
            }
            else
            {
                USART_RX_STA = 0;  // 清除标志，继续等待
            }
        }
        delay_ms(10);
        wait_time += 10;
    }
    
    return 1;  // 超时
}

// 简单发送AT指令（不等待响应）
void Send_AT_Command_Simple(char *cmd)
{
    Usart_SendString(USART1, cmd);
    Usart_SendString(USART1, "\r\n");
}

// 发送AT指令并获取响应内容
u8 Send_AT_Command_GetResponse(char *cmd, char *response, u16 timeout_ms)
{
    u16 wait_time = 0;
    
    // 清空接收缓冲区
    USART_RX_STA = 0;
    memset(USART_RX_BUF, 0, USART_REC_LEN);
    
    // 发送AT指令
    Usart_SendString(USART1, cmd);
    Usart_SendString(USART1, "\r\n");
    
    // 等待响应
    while(wait_time < timeout_ms)
    {
        if(USART_RX_STA & 0x8000)  // 接收完成
        {
            // 复制响应内容
            strcpy(response, (char*)USART_RX_BUF);
            USART_RX_STA = 0;
            return 0;  // 成功
        }
        delay_ms(10);
        wait_time += 10;
    }
    
    return 1;  // 超时
}


u8 AliyunIoT_Connect(void)
{
    u8 result;
    
    printf("开始连接阿里云IoT平台...\r\n");
    OLED_Clear_NOupdate();
    OLED_Refresh_Gram();
    
    // 1. 重启ESP8266
    printf("步骤1: 重启ESP8266\r\n");
    result = Send_AT_Command("AT+RST", 100);

    
    // 2. 设置WiFi模式为AP+Station
    printf("步骤2: 设置WiFi模式\r\n");
    result = Send_AT_Command("AT+CWMODE=3",150);

    
    // 3. 连接WiFi
    printf("步骤3: 连接WiFi\r\n");
    OLED_Clear_NOupdate();
    Show_Str(12,13,128,12,"接WiFi...",12,0); 
    OLED_Refresh_Gram();
    
    result = Send_AT_Command("AT+CWJAP=\"llux\",\"wslc2005!\"", 5000);
    // result = Send_AT_Command("AT+CWJAP=\"1\",\"1234567890!\"", 5000);


    printf("WiFi连接成功\r\n");
    
    // 4. 配置MQTT用户信息
    printf("步骤4: 配置MQTT用户信息\r\n");
    OLED_Clear_NOupdate();
    Show_Str(12,13,128,12,"配置MQTT用户...",12,0); 
    OLED_Refresh_Gram();
    
    result = Send_AT_Command("AT+MQTTUSERCFG=0,1,\"k24prW5pSG6.mode002|securemode=2\\,signmethod=hmacsha256\\,timestamp=1748567989595|\",\"mode002&k24prW5pSG6\",\"de6f0363c731e7c09332eaeda76795736211651d5229211afc61bb47f48a7b81\",0,0,\"\"", 100);

    
    // 5. 连接MQTT服务器
    printf("步骤5: 连接MQTT服务器\r\n");
    OLED_Clear_NOupdate();
    Show_Str(12,13,128,12,"连接MQTT服务器",12,0); 
    OLED_Refresh_Gram();
    
    result = Send_AT_Command("AT+MQTTCONN=0,\"iot-06z00d27tdma064.mqtt.iothub.aliyuncs.com\",1883,1", 500);

    printf("MQTT服务器连接成功\r\n");
    
    // 6. 订阅属性设置主题
    printf("步骤6: 订阅属性设置主题\r\n");
    result = Send_AT_Command("AT+MQTTSUB=0,\"/sys/k24prW5pSG6/mode002/thing/service/property/set\",0", 500);

    result =Send_AT_Command("AT+MQTTPUB=0,\"/sys/k24prW5pSG6/mode002/thing/event/property/post\",\"{\\\"method\\\":\\\"thing.service.property.set\\\"\\,\\\"id\\\":\\\"1649634152\\\"\\,\\\"params\\\":{\\\"LockState\\\":0}\\,\\\"version\\\":\\\"1.0.0\\\"}\",0,0",100);

    return 0; // 成功
}

/**
 * @brief 发布锁状态到阿里云IoT平台
 * @param lock_state: 锁状态 (0-解锁, 1-上锁)
 * @return 0-成功，其他-失败
 */
u8 AliyunIoT_PublishLockState(u8 lock_state)
{
    char json_data[256];
    char at_command[512];
    u8 result;
    
    // 构造JSON数据
    sprintf(json_data, "{\\\"method\\\":\\\"thing.service.property.set\\\"\\,\\\"id\\\":\\\"1649634152\\\"\\,\\\"params\\\":{\\\"LockState\\\":%d}\\,\\\"version\\\":\\\"1.0.0\\\"}", lock_state);
    
    // 构造完整的AT指令
    sprintf(at_command, "AT+MQTTPUB=0,\"/sys/k24prW5pSG6/mode002/thing/event/property/post\",\"%s\",0,0", json_data);
    
    printf("发布锁状态: %s\r\n", lock_state ? "上锁" : "解锁");
    
    // 发送MQTT发布指令
    result = Send_AT_Command(at_command, 5000);
    if(result == 0) {
        printf("锁状态发布成功\r\n");
        OLED_Clear_NOupdate();
        Show_Str(12,13,128,12,lock_state ? "状态：已上锁" : "状态：已解锁",12,0); 
        OLED_Refresh_Gram();
    } else {
        printf("锁状态发布失败\r\n");
        OLED_Clear_NOupdate();
        Show_Str(12,13,128,12,"状态发布失败",12,0); 
        OLED_Refresh_Gram();
    }
    
    return result;
}


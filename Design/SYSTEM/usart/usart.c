#include "sys.h"
#include "usart.h"	  
#include "string.h"	  
#include "oled.h"	   
#include "delay.h"	  
#include "stdio.h"     // ���stdio.h֧��sprintf
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif


//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������

u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	  
  
void uart_init(u32 bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
  //USART1_RX	  GPIOA.10��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

  //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 

}

void USART1_IRQHandler(void)                	//����1�жϷ������
	{
	u8 Res;
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
		Res =USART_ReceiveData(USART1);	//��ȡ���յ�������
		
		if((USART_RX_STA&0x8000)==0)//����δ���
			{
			if(USART_RX_STA&0x4000)//���յ���0x0d
				{
					if(Res!=':')USART_RX_STA=0;//���մ���,���¿�ʼ0x0a
				else USART_RX_STA|=0x8000;	//��������� 
				}
			else //��û�յ�0X0D
				{	
				if(Res=='!')USART_RX_STA|=0x4000;//0x0d
				else
					{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
					}		 
				}
			}   		 
     } 
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
} 
#endif

//����ַ����Ƿ����������ƥ�� (����ԭ����������)
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
			printf("�������ݣ�");
			for(i =0; i<(USART_RX_STA & 0xff); i++)
			printf("%c",USART_RX_BUF[i]);
			printf("\r\n");
			ErrCnt++;
			if(USART_RX_STA & 0x8000)//�������
			{
				USART_RX_STA = 0;
				OLED_Clear_NOupdate();
				Show_Str(12,13,128,12,"�������룺����",12,0); 
				OLED_Refresh_Gram();//������ʾ
				delay_ms(800);
			}
		}
		if(ErrCnt > 5)//��ʱ
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
	USART_SendData(pUSARTx,ch);        /*   �������ݵ�USART   */        
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);        /*   �ȴ��������   */
}

// �޸�ԭ�к�����bug
void Usart_SendString( USART_TypeDef * pUSARTx, char *str)
{
	unsigned int k=0;
	do 
	{
		Usart_SendByte( pUSARTx, *(str + k) );  // �޸���ʹ�ô����pUSARTx����
		k++;
	} while(*(str + k)!='\0');
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET); // �޸���ʹ����ȷ��USART              
}

// ����ATָ�����ʱ�ȴ���Ӧ��
// ������cmd - ATָ���ַ�����timeout_ms - ��ʱʱ�䣨���룩
// ����ֵ��0-�ɹ���1-��ʱ��2-��Ӧ����
u8 Send_AT_Command(char *cmd, u16 timeout_ms)
{
    u16 wait_time = 0;
    
    // ��ս��ջ�����
    USART_RX_STA = 0;
    memset(USART_RX_BUF, 0, USART_REC_LEN);
    
    // ����ATָ��
    Usart_SendString(USART1, cmd);
    Usart_SendString(USART1, "\r\n");  // ATָ��ͨ����\r\n��β
    
    // �ȴ���Ӧ
    while(wait_time < timeout_ms)
    {
        if(USART_RX_STA & 0x8000)  // �������
        {
            // ����Ƿ��յ�OK��Ӧ
            if(strstr((char*)USART_RX_BUF, "OK") != NULL)
            {
                USART_RX_STA = 0;
                return 0;  // �ɹ�
            }
            // ����Ƿ��յ�ERROR��Ӧ
            else if(strstr((char*)USART_RX_BUF, "ERROR") != NULL || 
                    strstr((char*)USART_RX_BUF, "FAIL") != NULL)
            {
                USART_RX_STA = 0;
                return 2;  // ��Ӧ����
            }
            else
            {
                USART_RX_STA = 0;  // �����־�������ȴ�
            }
        }
        delay_ms(10);
        wait_time += 10;
    }
    
    return 1;  // ��ʱ
}

// �򵥷���ATָ����ȴ���Ӧ��
void Send_AT_Command_Simple(char *cmd)
{
    Usart_SendString(USART1, cmd);
    Usart_SendString(USART1, "\r\n");
}

// ����ATָ���ȡ��Ӧ����
u8 Send_AT_Command_GetResponse(char *cmd, char *response, u16 timeout_ms)
{
    u16 wait_time = 0;
    
    // ��ս��ջ�����
    USART_RX_STA = 0;
    memset(USART_RX_BUF, 0, USART_REC_LEN);
    
    // ����ATָ��
    Usart_SendString(USART1, cmd);
    Usart_SendString(USART1, "\r\n");
    
    // �ȴ���Ӧ
    while(wait_time < timeout_ms)
    {
        if(USART_RX_STA & 0x8000)  // �������
        {
            // ������Ӧ����
            strcpy(response, (char*)USART_RX_BUF);
            USART_RX_STA = 0;
            return 0;  // �ɹ�
        }
        delay_ms(10);
        wait_time += 10;
    }
    
    return 1;  // ��ʱ
}


u8 AliyunIoT_Connect(void)
{
    u8 result;
    
    printf("��ʼ���Ӱ�����IoTƽ̨...\r\n");
    OLED_Clear_NOupdate();
    OLED_Refresh_Gram();
    
    // 1. ����ESP8266
    printf("����1: ����ESP8266\r\n");
    result = Send_AT_Command("AT+RST", 100);

    
    // 2. ����WiFiģʽΪAP+Station
    printf("����2: ����WiFiģʽ\r\n");
    result = Send_AT_Command("AT+CWMODE=3",150);

    
    // 3. ����WiFi
    printf("����3: ����WiFi\r\n");
    OLED_Clear_NOupdate();
    Show_Str(12,13,128,12,"��WiFi...",12,0); 
    OLED_Refresh_Gram();
    
    result = Send_AT_Command("AT+CWJAP=\"llux\",\"wslc2005!\"", 5000);
    // result = Send_AT_Command("AT+CWJAP=\"1\",\"1234567890!\"", 5000);


    printf("WiFi���ӳɹ�\r\n");
    
    // 4. ����MQTT�û���Ϣ
    printf("����4: ����MQTT�û���Ϣ\r\n");
    OLED_Clear_NOupdate();
    Show_Str(12,13,128,12,"����MQTT�û�...",12,0); 
    OLED_Refresh_Gram();
    
    result = Send_AT_Command("AT+MQTTUSERCFG=0,1,\"k24prW5pSG6.mode002|securemode=2\\,signmethod=hmacsha256\\,timestamp=1748567989595|\",\"mode002&k24prW5pSG6\",\"de6f0363c731e7c09332eaeda76795736211651d5229211afc61bb47f48a7b81\",0,0,\"\"", 100);

    
    // 5. ����MQTT������
    printf("����5: ����MQTT������\r\n");
    OLED_Clear_NOupdate();
    Show_Str(12,13,128,12,"����MQTT������",12,0); 
    OLED_Refresh_Gram();
    
    result = Send_AT_Command("AT+MQTTCONN=0,\"iot-06z00d27tdma064.mqtt.iothub.aliyuncs.com\",1883,1", 500);

    printf("MQTT���������ӳɹ�\r\n");
    
    // 6. ����������������
    printf("����6: ����������������\r\n");
    result = Send_AT_Command("AT+MQTTSUB=0,\"/sys/k24prW5pSG6/mode002/thing/service/property/set\",0", 500);

    result =Send_AT_Command("AT+MQTTPUB=0,\"/sys/k24prW5pSG6/mode002/thing/event/property/post\",\"{\\\"method\\\":\\\"thing.service.property.set\\\"\\,\\\"id\\\":\\\"1649634152\\\"\\,\\\"params\\\":{\\\"LockState\\\":0}\\,\\\"version\\\":\\\"1.0.0\\\"}\",0,0",100);

    return 0; // �ɹ�
}

/**
 * @brief ������״̬��������IoTƽ̨
 * @param lock_state: ��״̬ (0-����, 1-����)
 * @return 0-�ɹ�������-ʧ��
 */
u8 AliyunIoT_PublishLockState(u8 lock_state)
{
    char json_data[256];
    char at_command[512];
    u8 result;
    
    // ����JSON����
    sprintf(json_data, "{\\\"method\\\":\\\"thing.service.property.set\\\"\\,\\\"id\\\":\\\"1649634152\\\"\\,\\\"params\\\":{\\\"LockState\\\":%d}\\,\\\"version\\\":\\\"1.0.0\\\"}", lock_state);
    
    // ����������ATָ��
    sprintf(at_command, "AT+MQTTPUB=0,\"/sys/k24prW5pSG6/mode002/thing/event/property/post\",\"%s\",0,0", json_data);
    
    printf("������״̬: %s\r\n", lock_state ? "����" : "����");
    
    // ����MQTT����ָ��
    result = Send_AT_Command(at_command, 5000);
    if(result == 0) {
        printf("��״̬�����ɹ�\r\n");
        OLED_Clear_NOupdate();
        Show_Str(12,13,128,12,lock_state ? "״̬��������" : "״̬���ѽ���",12,0); 
        OLED_Refresh_Gram();
    } else {
        printf("��״̬����ʧ��\r\n");
        OLED_Clear_NOupdate();
        Show_Str(12,13,128,12,"״̬����ʧ��",12,0); 
        OLED_Refresh_Gram();
    }
    
    return result;
}


#ifndef __USART_H
#define __USART_H

#include "stdio.h"	
#include "sys.h" 


#define USART_REC_LEN  			200  //�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	

//////////////////////////////////////////////////////////////////////////////////
// �������ں�������
//////////////////////////////////////////////////////////////////////////////////

void uart_init(u32 bound);              // ���ڳ�ʼ������
u8 usart1_cherk(char *buf);             // ����ַ����Ƿ����������ƥ�䣨�������ܣ�
void Usart_SendByte(USART_TypeDef * pUSARTx, uint8_t ch);    // ���͵����ֽ�
void Usart_SendString(USART_TypeDef * pUSARTx, char *str);   // �����ַ���

//////////////////////////////////////////////////////////////////////////////////
// ESP8266 WiFi ATָ����غ�������
//////////////////////////////////////////////////////////////////////////////////

// ATָ�����غ���
u8 Send_AT_Command(char *cmd, u16 timeout_ms);                     // ����ATָ��ȴ���Ӧ
void Send_AT_Command_Simple(char *cmd);                            // �򵥷���ATָ����ȴ���Ӧ��
u8 Send_AT_Command_GetResponse(char *cmd, char *response, u16 timeout_ms); // ����ATָ���ȡ��Ӧ����

// ESP8266�������ƺ���
u8 ESP8266_Test(void);                                             // ����ATָ��
u8 ESP8266_Reset(void);                                            // ����ESP8266
u8 ESP8266_GetVersion(char *version_buf);                          // ��ѯ�汾��Ϣ

// WiFi��غ���
u8 ESP8266_SetMode(u8 mode);                                       // ����WiFiģʽ (1=Station, 2=AP, 3=Station+AP)
u8 ESP8266_JoinAP(char *ssid, char *password);                     // ����WiFi
u8 ESP8266_QuitAP(void);                                           // �Ͽ�WiFi����
u8 ESP8266_GetIP(char *ip_info);                                   // ��ѯ����IP��ַ





u8 AliyunIoT_Connect(void);
#endif




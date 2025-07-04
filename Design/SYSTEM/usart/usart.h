#ifndef __USART_H
#define __USART_H

#include "stdio.h"	
#include "sys.h" 


#define USART_REC_LEN  			200  //定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	

//////////////////////////////////////////////////////////////////////////////////
// 基础串口函数声明
//////////////////////////////////////////////////////////////////////////////////

void uart_init(u32 bound);              // 串口初始化函数
u8 usart1_cherk(char *buf);             // 检查字符串是否与接收数据匹配（蓝牙功能）
void Usart_SendByte(USART_TypeDef * pUSARTx, uint8_t ch);    // 发送单个字节
void Usart_SendString(USART_TypeDef * pUSARTx, char *str);   // 发送字符串

//////////////////////////////////////////////////////////////////////////////////
// ESP8266 WiFi AT指令相关函数声明
//////////////////////////////////////////////////////////////////////////////////

// AT指令发送相关函数
u8 Send_AT_Command(char *cmd, u16 timeout_ms);                     // 发送AT指令并等待响应
void Send_AT_Command_Simple(char *cmd);                            // 简单发送AT指令（不等待响应）
u8 Send_AT_Command_GetResponse(char *cmd, char *response, u16 timeout_ms); // 发送AT指令并获取响应内容

// ESP8266基础控制函数
u8 ESP8266_Test(void);                                             // 测试AT指令
u8 ESP8266_Reset(void);                                            // 重启ESP8266
u8 ESP8266_GetVersion(char *version_buf);                          // 查询版本信息

// WiFi相关函数
u8 ESP8266_SetMode(u8 mode);                                       // 设置WiFi模式 (1=Station, 2=AP, 3=Station+AP)
u8 ESP8266_JoinAP(char *ssid, char *password);                     // 连接WiFi
u8 ESP8266_QuitAP(void);                                           // 断开WiFi连接
u8 ESP8266_GetIP(char *ip_info);                                   // 查询本机IP地址





u8 AliyunIoT_Connect(void);
#endif




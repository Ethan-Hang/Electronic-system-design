#ifndef __BEEP_H
#define __BEEP_H	 
#include "sys.h"

//�������˿ڶ���
#define BEEP PBout(8)	// BEEP,�������ӿ�		   

void BEEP_Init(void);	//��ʼ��
		 				    
								

void beep_on_mode1(void);
void beep_on_mode2(void);
#endif


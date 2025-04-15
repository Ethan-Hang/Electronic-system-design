#ifndef __SR04_H
#define __SR04_H

#include "stm32f10x.h"

extern uint32_t times;

void SR04_Init(void);
void Exti_Init(void);
void SR04_Start(void);
uint32_t SR04_GetDistance(void); // 返回毫米单位距离

#endif

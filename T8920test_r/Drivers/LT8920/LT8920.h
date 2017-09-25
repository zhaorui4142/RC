/******************** (C) COPYRIGHT  ���׵��ӹ����� ****************************
 * �� �� ��   ��LT8920.h
 * �� ��      ��LT8920��������
 * ʵ �� ƽ̨ ��STM32
 * �Ĵ����汾 ��V1.0.0
 * ����       ��rocfan        
 * �޸�ʱ��   ��2017-09-04 zhaorui
*******************************************************************************/
#ifndef __LT8920_H
#define __LT8920_H


//ͷ�ļ�����
#include "stm32f0xx_hal.h"
#include "tim.h"
#include "spi.h"
#include <stdbool.h>

//������
#define LT8920_CS_LOW()         HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET)
#define LT8920_RST_LOW()        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET)
#define LT8920_CS_HIGH()        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET)
#define LT8920_RST_HIGH()       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET)
#define LT8920_PKT_READ()       HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1)

#define Dummy_Byte 0xFF       /* д����ֽڲ��� */ 

//ȫ�ֺ�������
extern void WriteReg(uint8_t addr, uint8_t H, uint8_t L);
extern uint16_t ReadReg(uint8_t addr);
void LT8920_Init(void);
bool LT8920_Transmit(uint8_t bytes, uint8_t* data, uint32_t timeout);
bool LT8920_Receive(uint8_t bytes, uint8_t* data, uint32_t timeout);
bool LT8920_ScanRSSI(uint16_t* rssi, uint16_t* rssi_avg);
void LT8920_SetChannel(uint8_t ch);



#endif

/***************************END OF FILE********************************************/











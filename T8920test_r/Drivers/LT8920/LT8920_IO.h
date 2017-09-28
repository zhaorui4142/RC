/******************** (C) COPYRIGHT  ���׵��ӹ����� ****************************
 * �� �� ��   ��LT8920.h
 * �� ��      ��LT8920��������
 * ʵ �� ƽ̨ ��STM32
 * �Ĵ����汾 ��V1.0.0
 * ����       ��rocfan        
 * �޸�ʱ��   ��2017-09-04 zhaorui
*******************************************************************************/
#ifndef __LT8920_IO_H
#define __LT8920_IO_H


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

//������


//ȫ�ֺ�������

//��ʼ��
void LT8920_Init(void);
void LT8920_ConfigSPI(void);

//ѡ��һ��δ��ռ�õ�ͨ��
int  LT8920_SelectIdleChannel(uint8_t ch_start, uint8_t ch_end, uint32_t listen_ms, uint8_t max_rssi);   

//�շ�����
bool LT8920_Transmit(uint8_t deviceID, uint8_t fun, uint8_t* data, uint8_t len, uint32_t timeout);
bool LT8920_Receive(uint8_t* deviceID, uint8_t* fun, uint8_t* data, uint8_t *len, uint32_t timeout);

//����ź�
void LT8920_ScanRSSI(uint8_t* rssi_max, uint8_t* rssi_avg, int num);
void LT8920_SetChannel(uint8_t ch);
uint8_t LT8920_GetChannel(void);



#endif

/***************************END OF FILE********************************************/











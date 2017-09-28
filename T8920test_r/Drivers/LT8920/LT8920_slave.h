/******************** (C) COPYRIGHT  ���׵��ӹ����� ****************************
 * �� �� ��   ��LT8920.h
 * �� ��      ��LT8920��������
 * ʵ �� ƽ̨ ��STM32
 * �Ĵ����汾 ��V1.0.0
 * ����       ��rocfan        
 * �޸�ʱ��   ��2017-09-04 zhaorui
*******************************************************************************/
#ifndef __LT8920_SLAVE_H
#define __LT8920_SLAVE_H


//ͷ�ļ�����
#include "stm32f0xx_hal.h"
#include "tim.h"
#include "spi.h"
#include <stdbool.h>

//������
#define FUN_PAIR_REQUEST 0x11
#define FUN_CTRL_REQUEST 0x12
#define FUN_CTRL_FORCE   0x13
#define FUN_FIND_SLAVE   0x14
#define FUN_PAIR_RESPONSE 0x21
#define FUN_CTRL_RESPONSE 0x22
#define FUN_FIND_RESPONSE  0x23

//ȫ�ֺ�������
void LT8920_SlaveInit(uint8_t packet_length);                                                   //��ʼ��
bool LT8920_WaitPairing(uint32_t waiting_ms);                                                   //�ӻ��ȴ����
bool LT8920_WaitConnect(uint32_t waiting_ms);                                                   //�ӻ��ȴ�����
bool LT8920_WaitCommand(uint8_t* rx, uint8_t* feedback, uint32_t timeout, uint8_t* lost_count); //�ӻ��ȴ�����



#endif

/***************************END OF FILE********************************************/











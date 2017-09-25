/******************** (C) COPYRIGHT  ���׵��ӹ����� ****************************
 * �� �� ��   ��LT8920.h
 * �� ��      ��LT8920��������
 * ʵ �� ƽ̨ ��STM32
 * �Ĵ����汾 ��V1.0.0
 * ����       ��rocfan        
 * �޸�ʱ��   ��2017-09-04 zhaorui
*******************************************************************************/
#ifndef __LT8920_MASTER_H
#define __LT8920_MASTER_H


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

//��ʼ��
void LT8920_MasterInit(uint8_t packet_length);
bool LT8920_PairingRequest(uint32_t waiting_ms);//���������������
bool LT8920_FindSlave(void);
bool LT8920_ChangeSlave(void);
bool LT8920_CommunicateToSlaveWithFeedback(uint8_t* tx, uint8_t* rx, uint8_t* lost_count);//����һ��һ�������ݣ�������
bool LT8920_CommunicateToSlaveWithoutFeedback(uint8_t* tx);//�����㲥�������ݣ���������




#endif

/***************************END OF FILE********************************************/











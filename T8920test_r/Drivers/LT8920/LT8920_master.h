/******************** (C) COPYRIGHT  欣易电子工作室 ****************************
 * 文 件 名   ：LT8920.h
 * 描 述      ：LT8920驱动函数
 * 实 验 平台 ：STM32
 * 寄存器版本 ：V1.0.0
 * 作者       ：rocfan        
 * 修改时间   ：2017-09-04 zhaorui
*******************************************************************************/
#ifndef __LT8920_MASTER_H
#define __LT8920_MASTER_H


//头文件包含
#include "stm32f0xx_hal.h"
#include "tim.h"
#include "spi.h"
#include <stdbool.h>

//常数宏
#define FUN_PAIR_REQUEST 0x11
#define FUN_CTRL_REQUEST 0x12
#define FUN_CTRL_FORCE   0x13
#define FUN_FIND_SLAVE   0x14
#define FUN_PAIR_RESPONSE 0x21
#define FUN_CTRL_RESPONSE 0x22
#define FUN_FIND_RESPONSE  0x23

//全局函数声明

//初始化
void LT8920_MasterInit(uint8_t packet_length);
bool LT8920_PairingRequest(uint32_t waiting_ms);//主机发送配对请求
bool LT8920_FindSlave(void);
bool LT8920_ChangeSlave(void);
bool LT8920_CommunicateToSlaveWithFeedback(uint8_t* tx, uint8_t* rx, uint8_t* lost_count);//主机一对一发送数据，带反馈
bool LT8920_CommunicateToSlaveWithoutFeedback(uint8_t* tx);//主机广播发送数据，不带反馈




#endif

/***************************END OF FILE********************************************/











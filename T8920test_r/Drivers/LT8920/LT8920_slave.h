/******************** (C) COPYRIGHT  欣易电子工作室 ****************************
 * 文 件 名   ：LT8920.h
 * 描 述      ：LT8920驱动函数
 * 实 验 平台 ：STM32
 * 寄存器版本 ：V1.0.0
 * 作者       ：rocfan        
 * 修改时间   ：2017-09-04 zhaorui
*******************************************************************************/
#ifndef __LT8920_SLAVE_H
#define __LT8920_SLAVE_H


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
void LT8920_SlaveInit(uint8_t packet_length);                                                   //初始化
bool LT8920_WaitPairing(uint32_t waiting_ms);                                                   //从机等待配对
bool LT8920_WaitConnect(uint32_t waiting_ms);                                                   //从机等待连接
bool LT8920_WaitCommand(uint8_t* rx, uint8_t* feedback, uint32_t timeout, uint8_t* lost_count); //从机等待接收



#endif

/***************************END OF FILE********************************************/











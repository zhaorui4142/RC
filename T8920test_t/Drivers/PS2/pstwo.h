
/*********************************************************
PS2手柄stm32驱动
**********************************************************/	 
#ifndef __PSTWO_H
#define __PSTWO_H


#include "stm32f0xx_hal.h"
#include "tim.h"
#include "spi.h"
#include "gpio.h"
#include <stdbool.h>


//操作宏定义
#define PS2X_ATT_CLR()         HAL_GPIO_WritePin(PS2PAD_CS_GPIO_Port, PS2PAD_CS_Pin, GPIO_PIN_RESET)
#define PS2X_ATT_SET()        HAL_GPIO_WritePin(PS2PAD_CS_GPIO_Port, PS2PAD_CS_Pin, GPIO_PIN_SET)


//常数宏定义
//These are our button constants
#define PSB_SELECT      0x0001
#define PSB_L3          0x0002
#define PSB_R3          0x0004
#define PSB_START       0x0008
#define PSB_PAD_UP      0x0010
#define PSB_PAD_RIGHT   0x0020
#define PSB_PAD_DOWN    0x0040
#define PSB_PAD_LEFT    0x0080
#define PSB_L2          0x0100
#define PSB_R2          0x0200
#define PSB_L1          0x0400
#define PSB_R1          0x0800
#define PSB_GREEN       0x1000
#define PSB_RED         0x2000
#define PSB_BLUE        0x4000
#define PSB_PINK        0x8000
#define PSB_TRIANGLE    0x1000
#define PSB_CIRCLE      0x2000
#define PSB_CROSS       0x4000
#define PSB_SQUARE      0x8000
#define PSB_ANY         0xFFFF

//Guitar  button constants
#define UP_STRUM		0x0010
#define DOWN_STRUM		0x0040
#define STAR_POWER		0x0100
#define GREEN_FRET		0x0200
#define YELLOW_FRET		0x1000
#define RED_FRET		0x2000
#define BLUE_FRET		0x4000
#define ORANGE_FRET		0x8000
#define WHAMMY_BAR		8

//These are stick values
#define PSS_RX 5
#define PSS_RY 6
#define PSS_LX 7
#define PSS_LY 8

//These are analog buttons
#define PSAB_PAD_RIGHT    9
#define PSAB_PAD_UP      11
#define PSAB_PAD_DOWN    12
#define PSAB_PAD_LEFT    10
#define PSAB_L2          19
#define PSAB_R2          20
#define PSAB_L1          17
#define PSAB_R1          18
#define PSAB_GREEN       13
#define PSAB_RED         14
#define PSAB_BLUE        15
#define PSAB_PINK        16
#define PSAB_TRIANGLE    13
#define PSAB_CIRCLE      14
#define PSAB_CROSS       15
#define PSAB_SQUARE      16

//全局函数声明区
void     PS2X_ConfigSPI(void);								//设置SPI接口
uint8_t  PS2X_ConfigGamepad(bool pressures, bool rumble);   //配置手柄
bool     PS2X_ReadGamepad(bool motor1, uint8_t motor2);     //读取手柄数据

bool     PS2X_IsButtonOnToggle(uint16_t button);            //按钮状态改变
bool     PS2X_IsButtonOnPress(unsigned int button);         //按键被按下的一瞬间
bool     PS2X_IsButtonOnRelease(unsigned int button);       //按键被松开的一瞬间

bool     PS2X_GetButtonState(uint16_t button);              //查询某个按键状态
uint16_t PS2X_GetButtonData(void);                          //查询全部按钮的状态
uint8_t  PS2X_GetAnalogValue(uint8_t button);               //查询模拟量输出


#endif






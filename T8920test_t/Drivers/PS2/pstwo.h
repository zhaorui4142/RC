
/*********************************************************
PS2手柄stm32驱动
**********************************************************/	 
#ifndef __PSTWO_H
#define __PSTWO_H


#include "stm32f0xx_hal.h"
#include "tim.h"
#include "spi.h"
#include <stdbool.h>


//操作宏定义
#define PS2_CS_LOW()         HAL_GPIO_WritePin(PS2PAD_CS_GPIO_Port, PS2PAD_CS_Pin, GPIO_PIN_RESET)
#define PS2_CS_HIGH()        HAL_GPIO_WritePin(PS2PAD_CS_GPIO_Port, PS2PAD_CS_Pin, GPIO_PIN_SET)

//函数定义
void PS2_Init(void);
void PS2_SetSPIPort(void);
void PS2_ReadData(uint8_t* keys);
bool PS2_CheckButtonPress(uint16_t keys);


//These are our button constants
#define PSB_SELECT      0x8000
#define PSB_L3          0x4000
#define PSB_R3          0x2000
#define PSB_START       0x1000
#define PSB_PAD_UP      0x0800
#define PSB_PAD_RIGHT   0x0400
#define PSB_PAD_DOWN    0x0200
#define PSB_PAD_LEFT    0x0100
#define PSB_L2          0x0080
#define PSB_R2          0x0040
#define PSB_L1          0x0020
#define PSB_R1          0x0010
#define PSB_TRIANGLE    0x0008
#define PSB_CIRCLE      0x0004
#define PSB_CROSS       0x0002
#define PSB_SQUARE      0x0001

#define PSB_GREEN       0x0008
#define PSB_RED         0x0004
#define PSB_BLUE        0x0002
#define PSB_PINK        0x0001



//#define WHAMMY_BAR		8

//These are stick values
#define PSS_RX 5                //右摇杆X轴数据
#define PSS_RY 6
#define PSS_LX 7
#define PSS_LY 8

extern uint8_t Data[9];
extern uint16_t MASK[16];
extern uint16_t Handkey;

void PS2_Init(void);
uint8_t PS2_RedLight(void);   //判断是否为红灯模式
void PS2_ReadData(void); //读手柄数据
void PS2_Cmd(uint8_t CMD);		  //向手柄发送命令
uint8_t PS2_DataKey(void);		  //按键值读取
uint8_t PS2_AnologData(uint8_t button); //得到一个摇杆的模拟量
void PS2_ClearData(void);	  //清除数据缓冲区
void PS2_Vibration(uint8_t motor1, uint8_t motor2);//振动设置motor1  0xFF开，其他关，motor2  0x40~0xFF

void PS2_EnterConfing(void);	 //进入配置
void PS2_TurnOnAnalogMode(void); //发送模拟量
void PS2_VibrationMode(void);    //振动设置
void PS2_ExitConfing(void);	     //完成配置
void PS2_SetInit(void);		     //配置初始化

#endif






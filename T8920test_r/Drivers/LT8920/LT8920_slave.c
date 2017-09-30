/******************** (C) COPYRIGHT  欣易电子工作室 ****************************
 * 文 件 名   ：LT8920.c
 * 描 述      ：LT8920驱动函数
 * 实 验 平台 ：STM32
 * 寄存器版本 ：V1.0.0
 * 作者       ：rocfan        
 * 淘宝       ：https://shop64537643.taobao.com/
 * 修改时间   ：2017-09-04 zhaorui
*******************************************************************************/

//头文件包含
#include "LT8920_IO.h"
#include "LT8920_slave.h"
#include "stdio.h"

//常数宏定义
#define LISTEN_MS 100
#define MAX_RSSI 20

//函数声明
void WriteReg(uint8_t addr, uint8_t H, uint8_t L);
uint16_t ReadReg(uint8_t addr);
bool SaveRemoterID(uint8_t remoterID);
uint8_t LoadRemoterID(void);
uint8_t CalcSlaveDeviceID(void);

//全局变量
static uint8_t TxRxBytes;
static uint8_t DeviceID;
static uint8_t MasterID;


/*******************************************************************************
*功能说明: //初始化
*******************************************************************************/
bool LT8920_SlaveInit(uint8_t packet_length)
{
    //初始化寄存器和芯片
    for(int i=0; i<3; i++)
    {
        if(LT8920_Init())
        {
            //自动选择通道
            LT8920_SelectIdleChannel(0, 80, 100, 20);

            //初始化内部变量
            TxRxBytes = packet_length;
            DeviceID = CalcSlaveDeviceID();
            MasterID = LoadRemoterID();
            return true;
        }
    }
    return false;
}

/*******************************************************************************
*功能说明: //从机等待配对
*******************************************************************************/
bool LT8920_WaitPairing(uint32_t waiting_ms)
{
    
    uint32_t startTime = HAL_GetTick();
    uint8_t buf[TxRxBytes], ch=0;
    uint8_t remoterID,remoterCMD,remoterBytes;
    
    //检测超时
    while((HAL_GetTick() - startTime) < waiting_ms)
    {
        //设定频道
        LT8920_SetChannel(ch);
        
         //接收广播数据包
        if(LT8920_Receive(&remoterID, &remoterCMD, buf, &remoterBytes, LISTEN_MS))
        {printf("received ch = %d \n",ch);
            //配对请求
            if(remoterCMD == FUN_PAIR_REQUEST)
            {
                //回应广播数据包
                WriteReg(35, 0x07, 0x00 );//重发次数临时改为7次
                HAL_Delay(1);
                LT8920_Transmit(DeviceID, FUN_PAIR_RESPONSE, buf, TxRxBytes, 100);
                WriteReg(35, 0x03, 0x00 );//重发次数改回3次
                HAL_Delay(1);
            
                //设定频道
                LT8920_SetChannel(buf[1]);            
                //记录主机ID
                SaveRemoterID(remoterID);
                MasterID = remoterID;
                return true;
            }
            //连接请求
            if((remoterCMD == FUN_FIND_SLAVE) && (remoterID == MasterID))
            {
                //回应请求
                LT8920_Transmit(DeviceID, FUN_FIND_RESPONSE, buf, TxRxBytes, 100);
                //设定频道
                LT8920_SetChannel(buf[1]); 
                return true;
            }
        }
        
        //切换频道
        //if(ch >= 80)    ch  = 0;
        //else            ch += 8;
    }

    return false;
}



/*******************************************************************************
*功能说明: //从机等待连接
*******************************************************************************/
bool LT8920_WaitConnect(uint32_t waiting_ms)
{
    uint32_t startTime = HAL_GetTick();
    uint8_t buf[TxRxBytes], ch=0;
    uint8_t remoterID,remoterCMD,remoterBytes;
    
    //检测超时
    while((HAL_GetTick() - startTime) < waiting_ms)
    {
        //设定频道
        LT8920_SetChannel(ch);
        
         //接收广播数据包
        if(LT8920_Receive(&remoterID, &remoterCMD, buf, &remoterBytes, LISTEN_MS))
        {
            //连接请求
            if((remoterCMD == FUN_FIND_SLAVE) && (remoterID == MasterID))
            {
                //回应请求
                LT8920_Transmit(DeviceID, FUN_FIND_RESPONSE, buf, TxRxBytes, 100);
                //设定频道
                LT8920_SetChannel(buf[1]); 
                return true;
            }
        }
        
        //切换频道
        if(ch >= 80)    ch  = 0;
        else            ch += 8;
    }

    return false;
}

/*******************************************************************************
*功能说明: //从机等待接收
*******************************************************************************/
bool LT8920_WaitCommand(uint8_t* rx, uint8_t* feedback, uint32_t timeout, uint8_t* lost_count)
{
    //启动接收
    uint32_t startTime = HAL_GetTick();
    uint8_t remoterID,remoterCMD,remoterBytes;
    
    if(LT8920_Receive(&remoterID, &remoterCMD, rx, &remoterBytes, timeout))
    {
        switch(remoterCMD)
        {
            //配对请求
            case FUN_PAIR_REQUEST:
            {
                if((remoterID== MasterID) && (rx[0] == DeviceID))
                {
                    //补发配对回应
                    LT8920_Transmit(DeviceID, FUN_PAIR_RESPONSE, feedback, TxRxBytes, 100);
                    LT8920_SetChannel(rx[1]);
                    lost_count = 0;
                    return true;
                }
            }break;
            
            //查找从机请求
            case FUN_FIND_SLAVE:
            {
                if((remoterID== MasterID) && (rx[0] == DeviceID))
                {
                    //补发配对回应
                    LT8920_Transmit(DeviceID, FUN_FIND_RESPONSE, feedback, TxRxBytes, 100);
                    LT8920_SetChannel(rx[1]);
                    lost_count = 0;
                    return true;
                }
            }break;
            
            //控制请求
            case FUN_CTRL_REQUEST:
            {
                LT8920_Transmit(DeviceID, FUN_CTRL_RESPONSE, feedback, TxRxBytes, 100);
                lost_count = 0;
                return true;
            };
            
            //直接控制请求
            case FUN_CTRL_FORCE:
            {
                lost_count = 0;
                return true;
            };
            
            default:
            {
                lost_count++;
                return false;
            };
        }
    }

    lost_count++;
    return false;
}

/*******************************************************************************
*内部函数
*功能说明: 将ID写入flash
*写入新数据返回1，原有数据和写入数据相同返回0
*******************************************************************************/
bool SaveRemoterID(uint8_t remoterID)
{

    uint32_t addr = 0x08000000 + (15*1024);//写入第15页的位置

    //读取数据
    uint32_t flashData = *(__IO uint32_t*)(addr);
    
    if((flashData & 0x000000FF) != remoterID)
    {
        //解锁flash
        HAL_FLASH_Unlock();

        //擦除页
        FLASH_EraseInitTypeDef f;
        f.TypeErase = FLASH_TYPEERASE_PAGES;
        f.PageAddress = addr;
        f.NbPages = 1;

        uint32_t PageError = 0;
        HAL_FLASHEx_Erase(&f, &PageError);

        //编程flash
        HAL_FLASH_Program(TYPEPROGRAM_WORD, addr, (uint32_t)remoterID);

        //重新上锁
        HAL_FLASH_Lock();
        
        return true;
    }
    else
    {
        return false;
    }
}


uint8_t LoadRemoterID(void)
{
    uint32_t addr = 0x08000000 + (15*1024);

    //读取数据
    uint32_t flashData = *(__IO uint32_t*)(addr);
    
    return (flashData & 0x000000FF);
}

/*******************************************************************************
*内部函数
*功能说明: //计算本机ID
*******************************************************************************/
uint8_t CalcSlaveDeviceID(void)
{
    //利用stm32f030的cpuid计算出8位的id，用以标识器件
    uint32_t CPU_ID= *(__IO uint32_t *)(0x1FFFF7AC);
    uint8_t crc = 0;
    for(int j=0; j<4; j++)
    {
        crc=crc^((uint8_t)CPU_ID);
        for(int i = 8; i > 0; i--)
        {
            if(crc & 0x80)  crc = (crc<< 1)^0x31;
            else            crc = crc<< 1;
        }
        CPU_ID >>= 8;
    }
    
    return crc;
}







/*************************END OF FILE*****************************************/


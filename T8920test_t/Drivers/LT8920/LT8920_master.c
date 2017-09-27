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
#include "LT8920_master.h"
#include "stdio.h"

//常数宏定义
#define LISTEN_MS 30
#define WAIT_SLAVE_RESPOND_MS 100
#define MAX_SAVED_ID 8

//函数声明
uint8_t CalcMasterDeviceID(void);
uint8_t SaveSlaveID(uint8_t ID);
bool LoadSavedID(uint8_t index, uint8_t* id);

//全局变量
static uint8_t TxRxBytes;
static uint8_t DeviceID;
static uint8_t LockedSlaveID;
static uint8_t LockedSlaveIndex;
//static bool    LockedFlag;

/*******************************************************************************
*功能说明: 初始化为主机
*******************************************************************************/
void LT8920_MasterInit(uint8_t packet_length)
{
    //初始化寄存器和芯片
    LT8920_Init();
    
    //自动选择通道
    LT8920_SelectIdleChannel(8, 80, 100, 20);
    
    //初始化内部变量
    TxRxBytes = packet_length;
    DeviceID = CalcMasterDeviceID();
    //LockedFlag = false;
}

/*******************************************************************************
*功能说明: //主机发送配对请求
*******************************************************************************/
bool LT8920_PairingRequest(uint32_t waiting_ms)
{
    //发送广播数据包
    uint8_t buf[TxRxBytes];
    buf[0] = DeviceID;
    buf[1] = LT8920_GetChannel();//发送使用的频道
    LT8920_Transmit(DeviceID, FUN_PAIR_REQUEST, buf, TxRxBytes, 100);
    
    //等待从机回应
    uint8_t respones, receiverID, receivedBytes;
    if(LT8920_Receive(&receiverID, &respones, buf, &receivedBytes, waiting_ms))
    {
        if(respones == FUN_PAIR_RESPONSE)
        {
            //将从机的ID写入flash
            LockedSlaveID = receiverID;
            LockedSlaveIndex = SaveSlaveID(receiverID);
            
            return true;
        }  
    }
    
    return false;
}

/*******************************************************************************
*功能说明: //主机查找从机,一旦查找到从机，就一直往这个从机发数据
*******************************************************************************/
bool LT8920_FindSlave(void)
{
    uint8_t id, buf[TxRxBytes];
    uint8_t report_id,report_cmd, report_bytes;
    for(int i=0; i<MAX_SAVED_ID; i++)
    {
        //调出从机ID
        if(LoadSavedID(i, &id))
        {
            //往这个从机发数据
            buf[0] = id;                    //id
            buf[1] = LT8920_GetChannel();   //ch
            LT8920_Transmit(DeviceID, FUN_FIND_SLAVE, buf, TxRxBytes, 100);
        
            //等待从机返回数据
            if(LT8920_Receive(&report_id, &report_cmd, buf, &report_bytes, WAIT_SLAVE_RESPOND_MS))
            {
                if(report_cmd == FUN_FIND_RESPONSE)
                {
                    LockedSlaveID = id;
					LockedSlaveIndex = i;
                    //LockedFlag = true;
                    return true;
                }
            }
        }
    }
    return false;
}

/*******************************************************************************
*功能说明: //主机切换从机
*******************************************************************************/
bool LT8920_ChangeSlave(void)
{
    uint8_t id, buf[TxRxBytes];
    uint8_t report_id,report_cmd,report_bytes;
    int idx = LockedSlaveIndex, i = 0;
    
    while(i++ < MAX_SAVED_ID)
    {
        //调出下一个从机的ID
		if(LoadSavedID(idx, &id))
		{
			//往这个从机发数据
			buf[0] = id;        			//id
			buf[1] = LT8920_GetChannel();   //ch
			LT8920_Transmit(DeviceID, FUN_FIND_SLAVE, buf, TxRxBytes, 100);
        
			//等待从机返回数据
			if(LT8920_Receive(&report_id, &report_cmd, buf, &report_bytes, WAIT_SLAVE_RESPOND_MS))
			{
				if(report_cmd == FUN_FIND_RESPONSE)
				{
                    LockedSlaveID = id;
					LockedSlaveIndex = idx;
					//LockedFlag = true;
					return true;
				}
			}
		}
        
        if(++idx >= MAX_SAVED_ID) 
            idx -= MAX_SAVED_ID;     
    }

    return false;
}

/*******************************************************************************
*功能说明: //主机发送，带反馈，从机收到要发送反馈信号
*******************************************************************************/
bool LT8920_CommunicateToSlaveWithFeedback(uint8_t* tx, uint8_t* rx, uint8_t* lost_count)
{
    uint8_t report_id,report_cmd,report_bytes;

    //发送数据
	LT8920_Transmit(DeviceID, FUN_CTRL_REQUEST, tx, TxRxBytes, 100);
        
	//等待从机返回数据
	if(LT8920_Receive(&report_id, &report_cmd, rx, &report_bytes, WAIT_SLAVE_RESPOND_MS))
	{
		if((report_cmd == FUN_CTRL_RESPONSE) && (report_id == LockedSlaveID))
		{
            lost_count = 0;
			//LockedFlag = true;
			return true;
		}
	}
    
    lost_count ++;
    return false;
}



/*******************************************************************************
*功能说明: //主机发送，不带反馈，所有从机都能收到，但是不响应
*******************************************************************************/
bool LT8920_CommunicateToSlaveWithoutFeedback(uint8_t* tx)
{
    //发送数据
	LT8920_Transmit(DeviceID, FUN_CTRL_FORCE, tx, TxRxBytes, 100);
    return true;
}    




/*******************************************************************************
*内部函数
*功能说明: 将ID写入flash
*返回 index
*******************************************************************************/
uint8_t SaveSlaveID(uint8_t ID)
{
    //最多纪录8个，FIFO，按32位整数写入
    uint32_t addr = 0x08000000 + (15*1024);//第15页的位置
    uint32_t flashData = *(__IO uint32_t*)(addr);
    
    //检查当前ID是否存在,顺便读出原先的ID
    uint8_t dat[MAX_SAVED_ID];
    for(int i=0; i<MAX_SAVED_ID; i++)
    {
        flashData = *(__IO uint32_t*)(addr + (i*4));
        dat[i] = flashData & 0x000000ff;
        
        if(dat[i] == ID)  return i;
    }
    
    //如果flash中没有此ID，则写入FIFO队列
    HAL_FLASH_Unlock();
    //擦除页
    FLASH_EraseInitTypeDef f;
    f.TypeErase = FLASH_TYPEERASE_PAGES;
    f.PageAddress = addr;
    f.NbPages = 1;
    uint32_t PageError = 0;
    HAL_FLASHEx_Erase(&f, &PageError);
    //编程flash
    HAL_FLASH_Program(TYPEPROGRAM_WORD, addr, (uint32_t)ID);
    for(int i=0; i<MAX_SAVED_ID; i++)
    {
        HAL_FLASH_Program(TYPEPROGRAM_WORD, (addr + i*4), (uint32_t)dat[i]);
    }
    HAL_FLASH_Lock();
        
    return 0;
}


/*******************************************************************************
*内部函数
*功能说明: 读取flash中的ID
*******************************************************************************/
bool LoadSavedID(uint8_t index, uint8_t* id)
{
    uint32_t addr = 0x08000000 + (15*1024);//第15页的位置
    uint32_t flashData = *(__IO uint32_t*)(addr + (index*4));
    
    uint8_t ret = flashData & 0x000000ff;
    if(ret != 0xff)
    {
        *id = ret;
        return true;
    }
    else
    {
        return false;
    }
}


/*******************************************************************************
*内部函数
*功能说明: //计算本机ID
*******************************************************************************/
uint8_t CalcMasterDeviceID(void)
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


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
#define MAX_RSSI 20
#define MAX_SAVED_ID 8

//函数声明
uint8_t CalcDeviceID(void);
bool SaveRemoterID(uint8_t remoterID);
uint8_t LoadSavedID(uint8_t index);

//全局变量
static uint8_t TxRxBytes;
static uint8_t DeviceID;
static uint8_t LockedSlaveID;
//static uint8_t LockedMasterID;
static bool    isConnected;

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
    DeviceID = CalcDeviceID();
    isConnected = false;
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
    uint8_t respones, receiverID;
    bool return_value = false;
    if(LT8920_Receive(&receiverID, &respones, buf, TxRxBytes, waiting_ms))
    {
        if(respones == FUN_PAIR_RESPONSE)
        {
            //将从机的ID写入flash
            SaveReceiverID(receiverID);
            return_value = true;
        }  
    }
    
    return return_value;
}

/*******************************************************************************
*功能说明: //主机查找从机,一旦查找到从机，就一直往这个从机发数据
*******************************************************************************/
bool LT8920_FindSlave(void)
{
    //flash第15页
    uint8_t id, buf[TxRxBytes];
    uint8_t report_id,report_cmd;
    for(int i=0; i<8; i++)
    {
        //调出从机ID
        id = LoadSavedID(i);
        
        //往这个从机发数据
        buf[0] = id;                    //id
        buf[1] = LT8920_GetChannel();   //ch
        LT8920_Transmit(DeviceID, FUN_FIND_SLAVE, buf, TxRxBytes, 100);
        
        //等待从机返回数据
        if(LT8920_Receive(&report_id, &report_cmd, buf, 100))
        {
            //从机有回应
            if(report_cmd == FUN_FIND_RESPONSE)
            {
                LockedSlaveID = id;
                return true;
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
    //flash第15页
    uint8_t id, buf[TxRxBytes];
    uint8_t report_id,report_cmd;
    
    for(int i=0; i<MAX_SAVED_ID; i++)
    {
        //调出从机ID
        id = LoadSavedID(i);
        
        //往这个从机发数据
        buf[0] = id;        //id
        buf[1] = TxRx_CH;   //ch
        LT8920_Transmit(DeviceID, FUN_FIND_SLAVE, buf, 100);
        
        //等待从机返回数据
        if(LT8920_Receive(&report_id, &report_cmd, buf, 100))
        {
            //从机有回应
            if(report_cmd == FUN_FIND_RESPONSE)
            {
                if(LockedSlaveID != id)
                {
                    LockedSlaveID = id;
                    return true;
                }
            }
        }
    }
    return false;
}

/*******************************************************************************
*功能说明: //主机发送，带反馈，从机收到要发送反馈信号
*******************************************************************************/
bool LT8920_CommunicateToSlaveWithFeedback(uint8_t* tx, uint8_t* rx, uint8_t* lost_count)
{
    //ID=receiverID^remoterID
    //FUN_CTRL_REQUEST
    LT8920_Transmit(DeviceID, FUN_PAIR_REQUEST, tx, 100);
}



/*******************************************************************************
*功能说明: //主机发送，不带反馈，所有从机都能收到，但是不响应
*******************************************************************************/
bool LT8920_CommunicateToSlaveWithoutFeedback(uint8_t* tx)
{
    //FUN_CTRL_FORCE
}    

/*******************************************************************************
*功能说明: //从机等待连接
*******************************************************************************/
bool LT8920_WaitMastersConnect(void)
{
    isConnected
}

/*******************************************************************************
*功能说明: //从机等待接收
*******************************************************************************/
bool LT8920_WaitMastersCommuncation(uint8_t* rx, uint8_t* feedback, uint32_t timeout, uint8_t* lost_count)
{
    //启动接收
    uint8_t id,fun,buf[TxRxBytes];
    if(LT8920_Receive(&id, &fun, buf, 100))
    {
        switch(fun)
        {
            //配对请求
            case FUN_PAIR_REQUEST:
            {
                if(id == LockedMasterID)
                {
                    //回应广播数据包
                    LT8920_Transmit(DeviceID, FUN_PAIR_RESPONSE, buf, 100);
                    HAL_Delay(1);
            
                    //记录使用的频道
                    LT8920_SetChannel(buf[1]);
                }
                #define FUN_PAIR_RESPONSE 0x21
            }break;
            //控制请求
            case FUN_CTRL_REQUEST:
            {
                //ID=receiverID^remoterID
                #define FUN_CTRL_RESPONSE 0x22
            }break;
            //直接控制请求
            case FUN_CTRL_FORCE:
            {
                
            }break;
            //查找从机请求
            case FUN_FIND_SLAVE:
            {
                #define FUN_FIND_RESPONSE  0x23
            }break;
            default:
            {
                return false;
            }
        }
    }
    else
    {
        return false;
    }
    //如果是配对码，检查是否已配对，如果已配对，直接回配对响应码
    

}


/*******************************************************************************
*函 数 名: LT8900_Init
*功能说明: LT8920芯片初始化函数
*形    参：无
*返 回 值: 无
*******************************************************************************/
void LT8920_Init(uint8_t packet_length)
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
    DeviceID = crc;

	//初始化变量
	TxRx_CH = 0;
    TxRxBytes = packet_length;
    LockedSlaveID = 0;
    LockedMasterID = *(__IO uint8_t*)0x08003C00;
    isConnected = false;

    LT8920_CS_HIGH();
    LT8920_RST_LOW();
    HAL_Delay(2);
	LT8920_RST_HIGH();
	HAL_Delay(5);

    //初始化寄存器
	WriteReg( 0, 0x6f, 0xef );
	WriteReg( 1, 0x56, 0x81 );
	WriteReg( 2, 0x66, 0x17 );
	WriteReg( 4, 0x9c, 0xc9 );
	WriteReg( 5, 0x66, 0x37 );
	WriteReg( 7, 0x00, 0x00 );//TX_EN[8] RX_EN[7] RF_PLL_CH_NO[6:0]  channel is 2402Mhz
	WriteReg( 8, 0x6c, 0x90 );
	WriteReg( 9, 0x48, 0x00 );//PA最大电流和增益
	WriteReg(10, 0x7f, 0xfd );//XTAL_OSC_EN
	WriteReg(11, 0x00, 0x08 );//RSSI[11] 开启
	WriteReg(12, 0x00, 0x00 );
	WriteReg(13, 0x48, 0xbd );
	WriteReg(22, 0x00, 0xff );
	WriteReg(23, 0x80, 0x05 );//TxRx_VCO_CAL_EN
	WriteReg(24, 0x00, 0x67 );
	WriteReg(25, 0x16, 0x59 );
	WriteReg(26, 0x19, 0xe0 );
	WriteReg(27, 0x13, 0x00 );
	WriteReg(28, 0x18, 0x00 );
	WriteReg(32, 0x48, 0x00 );//8920在62.5kbps下同步头只能是32或16bit
	WriteReg(33, 0x3f, 0xc7 );
	WriteReg(34, 0x20, 0x00 );
	WriteReg(35, 0x03, 0x00 );//重发次数3次
	WriteReg(36, 0x03, 0x80 );
	WriteReg(37, 0x03, 0x80 );
	WriteReg(38, 0x5a, 0x5a );
	WriteReg(39, 0x03, 0x80 );
	WriteReg(40, 0x44, 0x02 );
	WriteReg(41, 0xb0, 0x00 );//CRC is ON; scramble is OFF; 关闭AUTO_ACK
	WriteReg(42, 0xfd, 0xb0 );//等待RX_ACK时间 176us
	WriteReg(43, 0x00, 0x0f );					
	WriteReg(44, 0x10, 0x00 );
	WriteReg(45, 0x05, 0x52 );		 //62.5k
	WriteReg(50, 0x00, 0x00 );
    HAL_Delay(100);
}

/*******************************************************************************
*功能说明: //自动选择一个空闲通道
*形    参：无
*返 回 值: 无
*******************************************************************************/
int LT8920_SelectIdleChannel(uint8_t ch_start, uint8_t ch_end, uint32_t listen_ms, uint8_t max_rssi) 
{
    bool selected = true;
    uint8_t ch;
    //从第一个通道开始，检查通道占用
    for(ch=ch_start; ch<=ch_end; ch+=8)
    {
        //TX_EN=0 RX_EN=1
        WriteReg(7, 0x00, (0x80|ch));
        Delay_us(2);
        
        //每个通道检测ms_ch
        uint32_t start = HAL_GetTick();
        while((HAL_GetTick()-start) < listen_ms)
        {
            //在这个通道接收到数据，或者RSSI超限，弃用
            if((LT8920_PKT_READ() != GPIO_PIN_RESET) || ((ReadReg(6)>>10) > max_rssi))
            {
                selected = false;
                break;
            }
            Delay_us(2);
        }
        WriteReg(7, 0x00, ch);
        Delay_us(2);
        
        //选定通道
        if(selected) 
        {
            LT8920_SetChannel(ch);
            break;
        }
    }  
    
    //返回
    if(selected) return ch;
    else         return -1;
}


/*******************************************************************************
*函 数 名: WriteReg
*功能说明: 内部函数，SPI写16位寄存器
*形    参：addr地址 H高字节 L低字节
*返 回 值: 无
*******************************************************************************/
void WriteReg(uint8_t addr, uint8_t H, uint8_t L)
{
	LT8920_CS_LOW();
    
    Delay_us(1);
    HAL_SPI_Transmit(&hspi1, &addr, 1,500);
    Delay_us(1);
    HAL_SPI_Transmit(&hspi1, &H, 1,500);
    Delay_us(1);
    HAL_SPI_Transmit(&hspi1, &L, 1,500);
    Delay_us(1);
    
	LT8920_CS_HIGH();
    Delay_us(1);
}

/*******************************************************************************
*函 数 名: ReadReg
*功能说明: 内部函数，SPI读8位寄存器
*形    参：addr要读取的寄存器地址值
*返 回 值: 无
*******************************************************************************/
uint16_t ReadReg(uint8_t addr)
{	
    uint8_t RegH;
    uint8_t RegL;  
    
	LT8920_CS_LOW();
    
    Delay_us(1);
    addr |= 0x80;//when reading a Register,the Address should be added with 0x80.
    HAL_SPI_Transmit(&hspi1, &addr, 1, 500);
    Delay_us(1);
    HAL_SPI_Receive(&hspi1, &RegH, 1, 500);
    Delay_us(1);
    HAL_SPI_Receive(&hspi1, &RegL, 1, 500);
    Delay_us(1);
    
	LT8920_CS_HIGH();
    Delay_us(1);
    
    return ((((uint16_t)RegH)<<8) | ((uint16_t)RegL));
}

/*******************************************************************************
*全局函数
*功能说明: 通过LT8900往外发数据
*******************************************************************************/
bool LT8920_Transmit(uint8_t deviceID, uint8_t fun, uint8_t* data, uint32_t timeout)
{
	//计时
	uint32_t start = HAL_GetTick();
    uint8_t fifo = 50;
	
    //发射时启动AUTO_ACK
    WriteReg(41, 0xb8, 0x00 );
    //TX_EN=0 RX_EN=0
	WriteReg(7, 0x00, TxRx_CH); 
    //清fifo的读写指针
	WriteReg(52, 0x80, 0x80);
	
    //填入需要的数据到fifo
	LT8920_CS_LOW();
    Delay_us(1);
	//写入地址
	HAL_SPI_Transmit(&hspi1, &fifo, 1, 500);
    Delay_us(1);
	//写入数据长度
    uint8_t len = TxRxBytes+2;
    HAL_SPI_Transmit(&hspi1, &len, 1, 500);
    Delay_us(1);
    //写入ID
    HAL_SPI_Transmit(&hspi1, &deviceID, 1, 500);
    Delay_us(1);
    //写入功能码
    HAL_SPI_Transmit(&hspi1, &fun, 1, 500);
    Delay_us(1);
	//写入其他字节
    for(int i=0; i<TxRxBytes; i++)
	{
		HAL_SPI_Transmit(&hspi1, data++, 1,500);
		Delay_us(1);
	}
	LT8920_CS_HIGH();
    Delay_us(1);
	
    //TX_EN=1 RX_EN=0
	WriteReg(7, 0x01, TxRx_CH); 
	
    //等待pkt标志位拉高
	while((HAL_GetTick() - start) < timeout)
	{
		if(LT8920_PKT_READ() != GPIO_PIN_RESET)
			return true;	
	}
    return false;
}

/*******************************************************************************
*全局函数
*功能说明: 通过LT8900接收数据
*******************************************************************************/
bool LT8920_Receive(uint8_t* deviceID, uint8_t* fun, uint8_t* data, uint32_t timeout)
{
	//计时
	uint32_t start = HAL_GetTick();
	uint8_t len;
    uint8_t addr = 0xB2;  //(50 | 0x80).
    
    //接收时关闭AUTO_ACK
    WriteReg(41, 0xb0, 0x00 );
    //TX_EN=0 RX_EN=0
	WriteReg(7, 0x00, TxRx_CH); 
    //清fifo指针
	WriteReg(52, 0x80, 0x80);
	
    //TX_EN=0 RX_EN=1
	WriteReg(7, 0x00, (0x80 | TxRx_CH)); 
	//等待数据接收完成
    while(HAL_GetTick() - start < timeout)
    {
        if((LT8920_PKT_READ() != GPIO_PIN_RESET) && (((ReadReg(52) & 0x3F00) >> 8) >= TxRxBytes+2))
        {
            //CRC校验
            if((ReadReg(48) & 0x8000) == 0)
            {   
                //读出fifo中数据
                LT8920_CS_LOW();
                Delay_us(1);
                //写入地址
                HAL_SPI_Transmit(&hspi1, &addr, 1, 500);
                Delay_us(1);
                //接收数据长度
                HAL_SPI_Receive(&hspi1, &len, 1, 500);
                Delay_us(1);
                //接收ID
                HAL_SPI_Receive(&hspi1, deviceID, 1, 500);
                //接收功能码
                HAL_SPI_Receive(&hspi1, fun, 1, 500);
                Delay_us(1);
                //读取剩下的数据
                for(int i=0; i<TxRxBytes; i++)
                {
                    HAL_SPI_Receive(&hspi1, data++, 1, 500);
                    Delay_us(1);
                }
                LT8920_CS_HIGH();
                Delay_us(1);
                
                //TX_EN=0 RX_EN=0
                WriteReg(7, 0x00, TxRx_CH); 
                return true;       
            }
            else
            {
                break;
            }
        }
    }
    //接收超时
    WriteReg(7, 0x00, TxRx_CH); 
    return false;
}

/*******************************************************************************
*全局函数
*功能说明: 扫描各个信道的信号强度，共80个信道
*******************************************************************************/
void LT8920_ScanRSSI(uint8_t* rssi_max, uint8_t* rssi_avg, int num)
{
    uint16_t sum;
    uint16_t tmp;
    for(int i=0; i<80; i++)
    {
        //选择频道
        WriteReg(7, 0x00, 0x80+i);
        Delay_us(2);
        sum = 0;
        rssi_max[i] = 0;
        for(int j=0; j<num; j++)
        {
            //读出RSSI值
            tmp = ReadReg(6)>>10;
            //选取最大值
            if(rssi_max[i]<tmp)
                rssi_max[i] = tmp;
            //计算平均值
            sum += tmp;
            Delay_us(1);
        }
        rssi_avg[i] = sum/num;
        WriteReg(7, 0x00, i);
    }
}


/*******************************************************************************
*全局函数
*功能说明: 设置使用的通道
*******************************************************************************/
void LT8920_SetChannel(uint8_t ch)
{
    TxRx_CH = ch;
    WriteReg(7, 0x00, ch);
    Delay_us(1);
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

/*******************************************************************************
*内部函数
*功能说明: 将ID写入flash
*写入新数据返回1，原有数据和写入数据相同返回0
*******************************************************************************/
bool SaveReceiverID(uint8_t receiverID)
{
    //最多纪录8个，FIFO
    uint32_t addr = 0x08000000 + (15*1024);//写入第15页的位置
    uint32_t flashData = *(__IO uint32_t*)(addr);
    
    //检查当前ID是否存在,顺便读出原先的ID
    bool appear = false;
    uint8_t dat[8];
    for(int i=0; i<8; i++)
    {
        flashData = *(__IO uint32_t*)(addr + (i*4));
        dat[i] = flashData & 0x000000ff;
        
        if(dat[i] == receiverID)  appear = true;
    }
    
    //如果flash中没有此ID，则写入FIFO队列
    if(appear)
    {
        return false;
    }
    else
    {
        HAL_FLASH_Unlock();
        //擦除页
        FLASH_EraseInitTypeDef f;
        f.TypeErase = FLASH_TYPEERASE_PAGES;
        f.PageAddress = addr;
        f.NbPages = 1;
        uint32_t PageError = 0;
        HAL_FLASHEx_Erase(&f, &PageError);

        //编程flash
        HAL_FLASH_Program(TYPEPROGRAM_WORD, addr, (uint32_t)receiverID);
        for(int i=0; i<7; i++)
        {
            HAL_FLASH_Program(TYPEPROGRAM_WORD, addr + i*4, (uint32_t)dat[i]);
        }
        HAL_FLASH_Lock();
        
        return true;
    }
}


/*******************************************************************************
*内部函数
*功能说明: 读取flash中的ID
*******************************************************************************/
uint8_t LoadSavedID(uint8_t index)
{
    
}


//计算本机ID
uint8_t CalcDeviceID(void)
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
    
    rerurn crc;
}







/*************************END OF FILE*****************************************/


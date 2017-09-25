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

//常数宏定义


//函数声明
void WriteReg(uint8_t addr, uint8_t H, uint8_t L);
uint16_t ReadReg(uint8_t addr);

//全局变量
static uint8_t TxRxChannel;


/*******************************************************************************
*函 数 名: LT8900_Init
*功能说明: LT8920芯片初始化函数
*形    参：无
*返 回 值: 无
*******************************************************************************/
void LT8920_Init(void)
{
	//初始化变量
	TxRxChannel = 0;

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
bool LT8920_Transmit(uint8_t deviceID, uint8_t fun, uint8_t* data, uint8_t len, uint32_t timeout)
{
	//计时
	uint32_t start = HAL_GetTick();
    uint8_t fifo = 50;
	
    //发射时启动AUTO_ACK
    WriteReg(41, 0xb8, 0x00 );
    //TX_EN=0 RX_EN=0
	WriteReg(7, 0x00, TxRxChannel); 
    //清fifo的读写指针
	WriteReg(52, 0x80, 0x80);
	
    //填入需要的数据到fifo
	LT8920_CS_LOW();
    Delay_us(1);
	//写入地址
	HAL_SPI_Transmit(&hspi1, &fifo, 1, 500);
    Delay_us(1);
	//写入数据长度
    len += 2;
    HAL_SPI_Transmit(&hspi1, &len, 1, 500);
    Delay_us(1);
    //写入ID
    HAL_SPI_Transmit(&hspi1, &deviceID, 1, 500);
    Delay_us(1);
    //写入功能码
    HAL_SPI_Transmit(&hspi1, &fun, 1, 500);
    Delay_us(1);
	//写入其他字节
    for(int i=0; i<(len-2); i++)
	{
		HAL_SPI_Transmit(&hspi1, data++, 1,500);
		Delay_us(1);
	}
	LT8920_CS_HIGH();
    Delay_us(1);
	
    //TX_EN=1 RX_EN=0
	WriteReg(7, 0x01, TxRxChannel); 
	
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
bool LT8920_Receive(uint8_t* deviceID, uint8_t* fun, uint8_t* data, uint8_t *len, uint32_t timeout)
{
	//计时
	uint32_t start = HAL_GetTick();
    uint8_t addr = 0xB2;  //(50 | 0x80).
    
    //接收时关闭AUTO_ACK
    WriteReg(41, 0xb0, 0x00 );
    //TX_EN=0 RX_EN=0
	WriteReg(7, 0x00, TxRxChannel); 
    //清fifo指针
	WriteReg(52, 0x80, 0x80);
	
    //TX_EN=0 RX_EN=1
	WriteReg(7, 0x00, (0x80 | TxRxChannel)); 
    
    //等待接收到第一个字节
     while(HAL_GetTick() - start < timeout)
    {
        if(LT8920_PKT_READ() != GPIO_PIN_RESET)
        {
            //读出fifo中数据
            LT8920_CS_LOW();
            Delay_us(1);
            //写入地址
            HAL_SPI_Transmit(&hspi1, &addr, 1, 500);
            Delay_us(1);
            //接收数据长度,
            HAL_SPI_Receive(&hspi1, len, 1, 500);
            Delay_us(1);
            break;
        }
    }
    
	//等待数据接收完成
    while(HAL_GetTick() - start < timeout)
    {
        if((LT8920_PKT_READ() != GPIO_PIN_RESET) && (((ReadReg(52) & 0x3F00) >> 8) >= *len))
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
                //接收ID
                HAL_SPI_Receive(&hspi1, deviceID, 1, 500);
                //接收功能码
                HAL_SPI_Receive(&hspi1, fun, 1, 500);
                Delay_us(1);
                //读取剩下的数据
                for(int i=0; i < (*len)-3; i++)
                {
                    HAL_SPI_Receive(&hspi1, data++, 1, 500);
                    Delay_us(1);
                }
                LT8920_CS_HIGH();
                Delay_us(1);
                
                //TX_EN=0 RX_EN=0
                WriteReg(7, 0x00, TxRxChannel); 
                
                //计算实际的数据长度
                (*len) -= 3;
                return true;       
            }
            else
            {
                break;
            }
        }
    }
    //接收超时
    WriteReg(7, 0x00, TxRxChannel); 
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
    TxRxChannel = ch;
    WriteReg(7, 0x00, ch);
    Delay_us(1);
}

/*******************************************************************************
*全局函数
*功能说明: 读取当前使用的通道
*******************************************************************************/
uint8_t LT8920_GetChannel(void)
{
    return TxRxChannel;
}




/*************************END OF FILE*****************************************/


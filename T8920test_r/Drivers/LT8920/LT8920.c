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
#include "LT8920.h"
#include "stdio.h"

//函数声明
void WriteReg(uint8_t addr, uint8_t H, uint8_t L);
uint16_t ReadReg(uint8_t addr);
void WriteFIFO(uint8_t bytes, uint8_t* data);
void ReadFIFO(uint8_t bytes, uint8_t *data);	

//全局变量
static uint8_t TxRx_CH;


/*******************************************************************************
*函 数 名: LT8900_Init
*功能说明: LT8920芯片初始化函数
*形    参：无
*返 回 值: 无
*******************************************************************************/
void LT8920_Init(void)
{
	//初始化变量
	TxRx_CH = 0;
	
    //上电操作
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
	WriteReg(41, 0xb0, 0x00 );//CRC is ON; scramble is OFF; AUTO_ACK is OFF
	WriteReg(42, 0xfd, 0xb0 );//等待RX_ACK时间 176us
	WriteReg(43, 0x00, 0x0f );					
	WriteReg(44, 0x10, 0x00 );
	WriteReg(45, 0x05, 0x52 );		 //62.5k
	WriteReg(50, 0x00, 0x00 );
    HAL_Delay(100);
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

//写入fifo
void WriteFIFO(uint8_t bytes, uint8_t* data)
{
	uint8_t addr = 50;
	//拉低总线
	LT8920_CS_LOW();
    Delay_us(1);
	
	//写入地址
	HAL_SPI_Transmit(&hspi1, &addr, 1, 500);
	
	//第一个字节表示数据长度,
    HAL_SPI_Transmit(&hspi1, &bytes, 1, 500);
    Delay_us(1);
	
	//写入其他字节
	while(bytes > 0)
	{
		HAL_SPI_Transmit(&hspi1, data, 1,500);
		Delay_us(1);
		++data;
		--bytes;
	}
    
	//释放总线
	LT8920_CS_HIGH();
    Delay_us(1);
}

//从fifo读取
void ReadFIFO(uint8_t bytes, uint8_t *data)
{
	uint8_t reg0;
	uint8_t addr = 0xB2;//when reading a Register,the Address should be added with 0x80.
	
	//拉低总线
	LT8920_CS_LOW();
    Delay_us(1);
	
    //写入地址
    HAL_SPI_Transmit(&hspi1, &addr, 1, 500);
    Delay_us(1);
	
	//第一个字节表示数据长度
    HAL_SPI_Receive(&hspi1, &reg0, 1, 500);
    Delay_us(1);
	
	//读取剩下的数据
	while(bytes > 0)
	{
		HAL_SPI_Receive(&hspi1, data, 1, 500);
		Delay_us(1);
		data++;
		bytes--;
	}
    
    //释放总线
	LT8920_CS_HIGH();
    Delay_us(1);
}

/*******************************************************************************
*全局函数
*功能说明: 通过LT8900往外发数据
*******************************************************************************/
bool LT8920_Transmit(uint8_t bytes, uint8_t* data, uint32_t timeout)
{
	//计时
	uint32_t start = HAL_GetTick();
	
    //写reg52->0x8000 清fifo
	WriteReg(7, 0x00, TxRx_CH); 
	WriteReg(52, 0x80, 0x80);
	
    //写reg50-> 填入需要的数据到fifo
	WriteFIFO(bytes, data);
	
    //写reg7->0x10xxxxxxx 7位选择通道号
	WriteReg(7, 0x01, TxRx_CH); 
	
    //等待pkt标志位拉高或超时
	while(1)
	{
		//发送成功
		if(LT8920_PKT_READ() != GPIO_PIN_RESET)
			return true;
		//发送超时(这里没做错误处理)
		if(HAL_GetTick() - start >= timeout)
			return false;
	}
}

/*******************************************************************************
*全局函数
*功能说明: 通过LT8900接收数据
*******************************************************************************/
bool LT8920_Receive(uint8_t bytes, uint8_t* data, uint32_t timeout)
{
	//计时
	uint32_t start = HAL_GetTick();
	
    //写reg52->0x8080 清fifo
	WriteReg(7, 0x00, TxRx_CH); 
	WriteReg(52, 0x80, 0x80);
	
    //写reg7->0x1xxxxxxx 7位选择通道号
	WriteReg(7, 0x00, (0x80 | TxRx_CH)); 
	
    //等待pkt标志位拉高
	while(LT8920_PKT_READ() == GPIO_PIN_RESET)
	{
		//接收超时(这里没做错误处理)
		if(HAL_GetTick() - start >= timeout)
        {
            WriteReg(7, 0x00, TxRx_CH); 
            printf("pkt timeout!");
            return false;
        }
			
	}
	
    //等待接收完成
	while(((ReadReg(52) & 0x3F00) >> 8) < bytes)
	{
		//接收超时(这里没做错误处理)
		if(HAL_GetTick() - start >= timeout)
        {
            WriteReg(7, 0x00, TxRx_CH); 
            printf("bytes timeout!");
            return false;
        }
			
	}
	
	//CRC校验
    if((ReadReg(48) & 0x8000) == 0)
    {   
        //读reg50，读出相应数据
		ReadFIFO(bytes, data);
		return true;       
    }
	else
	{
		return false;
	}
	
	
	//停止读数据
	
	
	
}

/*******************************************************************************
*全局函数
*功能说明: 扫描各个信道的信号强度，共80个信道
*******************************************************************************/
bool LT8920_ScanRSSI(uint16_t* rssi_max, uint16_t* rssi_avg)
{
    uint32_t sum;
    uint16_t tmp;
    for(int i=0; i<80; i++)
    {
        WriteReg(7, 0x00, 0x80+i);
        Delay_us(2);
        sum = 0;
        rssi_max[i] = 0;
        for(int j=0; j<1000; j++)
        {
            tmp = (uint32_t)ReadReg(6)>>10;
            if(rssi_max[i]<tmp)
                rssi_max[i] = tmp;
            sum += tmp;
            Delay_us(1);
        }
        rssi_avg[i] = sum/1000;
        
    }
   /* WriteReg(42, 0xFD, 0xFF);
    WriteReg(43, 0x80, 0x0F);
    while(LT8920_PKT_READ() != GPIO_PIN_RESET);
    uint16_t wr_ptr_cnt;
	while(1)
    {
        wr_ptr_cnt = ReadReg(52);
        wr_ptr_cnt >>= 8;
        wr_ptr_cnt &= 0x003F;
        if(wr_ptr_cnt == 0x3F)
            break;
    }
    
    for(int i=0; i<wr_ptr_cnt; i++)
    {
        rssi[i] = ReadReg(50);
    }*/
	return true;
}





/*************************END OF FILE*****************************************/


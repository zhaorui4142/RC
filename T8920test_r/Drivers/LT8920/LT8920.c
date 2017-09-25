/******************** (C) COPYRIGHT  ���׵��ӹ����� ****************************
 * �� �� ��   ��LT8920.c
 * �� ��      ��LT8920��������
 * ʵ �� ƽ̨ ��STM32
 * �Ĵ����汾 ��V1.0.0
 * ����       ��rocfan        
 * �Ա�       ��https://shop64537643.taobao.com/
 * �޸�ʱ��   ��2017-09-04 zhaorui
*******************************************************************************/

//ͷ�ļ�����
#include "LT8920.h"
#include "stdio.h"

//��������
void WriteReg(uint8_t addr, uint8_t H, uint8_t L);
uint16_t ReadReg(uint8_t addr);
void WriteFIFO(uint8_t bytes, uint8_t* data);
void ReadFIFO(uint8_t bytes, uint8_t *data);	

//ȫ�ֱ���
static uint8_t TxRx_CH;


/*******************************************************************************
*�� �� ��: LT8900_Init
*����˵��: LT8920оƬ��ʼ������
*��    �Σ���
*�� �� ֵ: ��
*******************************************************************************/
void LT8920_Init(void)
{
	//��ʼ������
	TxRx_CH = 0;
	
    //�ϵ����
    LT8920_CS_HIGH();
    LT8920_RST_LOW();
    HAL_Delay(2);
	LT8920_RST_HIGH();
	HAL_Delay(5);

    //��ʼ���Ĵ���
	WriteReg( 0, 0x6f, 0xef );
	WriteReg( 1, 0x56, 0x81 );
	WriteReg( 2, 0x66, 0x17 );
	WriteReg( 4, 0x9c, 0xc9 );
	WriteReg( 5, 0x66, 0x37 );
	WriteReg( 7, 0x00, 0x00 );//TX_EN[8] RX_EN[7] RF_PLL_CH_NO[6:0]  channel is 2402Mhz
	WriteReg( 8, 0x6c, 0x90 );
	WriteReg( 9, 0x48, 0x00 );//PA������������
	WriteReg(10, 0x7f, 0xfd );//XTAL_OSC_EN
	WriteReg(11, 0x00, 0x08 );//RSSI[11] ����
	WriteReg(12, 0x00, 0x00 );
	WriteReg(13, 0x48, 0xbd );
	WriteReg(22, 0x00, 0xff );
	WriteReg(23, 0x80, 0x05 );//TxRx_VCO_CAL_EN
	WriteReg(24, 0x00, 0x67 );
	WriteReg(25, 0x16, 0x59 );
	WriteReg(26, 0x19, 0xe0 );
	WriteReg(27, 0x13, 0x00 );
	WriteReg(28, 0x18, 0x00 );
	WriteReg(32, 0x48, 0x00 );//8920��62.5kbps��ͬ��ͷֻ����32��16bit
	WriteReg(33, 0x3f, 0xc7 );
	WriteReg(34, 0x20, 0x00 );
	WriteReg(35, 0x03, 0x00 );//�ط�����3��
	WriteReg(36, 0x03, 0x80 );
	WriteReg(37, 0x03, 0x80 );
	WriteReg(38, 0x5a, 0x5a );
	WriteReg(39, 0x03, 0x80 );
	WriteReg(40, 0x44, 0x02 );
	WriteReg(41, 0xb0, 0x00 );//CRC is ON; scramble is OFF; AUTO_ACK is OFF
	WriteReg(42, 0xfd, 0xb0 );//�ȴ�RX_ACKʱ�� 176us
	WriteReg(43, 0x00, 0x0f );					
	WriteReg(44, 0x10, 0x00 );
	WriteReg(45, 0x05, 0x52 );		 //62.5k
	WriteReg(50, 0x00, 0x00 );
    HAL_Delay(100);
}
/*******************************************************************************
*�� �� ��: WriteReg
*����˵��: �ڲ�������SPIд16λ�Ĵ���
*��    �Σ�addr��ַ H���ֽ� L���ֽ�
*�� �� ֵ: ��
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
*�� �� ��: ReadReg
*����˵��: �ڲ�������SPI��8λ�Ĵ���
*��    �Σ�addrҪ��ȡ�ļĴ�����ֵַ
*�� �� ֵ: ��
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

//д��fifo
void WriteFIFO(uint8_t bytes, uint8_t* data)
{
	uint8_t addr = 50;
	//��������
	LT8920_CS_LOW();
    Delay_us(1);
	
	//д���ַ
	HAL_SPI_Transmit(&hspi1, &addr, 1, 500);
	
	//��һ���ֽڱ�ʾ���ݳ���,
    HAL_SPI_Transmit(&hspi1, &bytes, 1, 500);
    Delay_us(1);
	
	//д�������ֽ�
	while(bytes > 0)
	{
		HAL_SPI_Transmit(&hspi1, data, 1,500);
		Delay_us(1);
		++data;
		--bytes;
	}
    
	//�ͷ�����
	LT8920_CS_HIGH();
    Delay_us(1);
}

//��fifo��ȡ
void ReadFIFO(uint8_t bytes, uint8_t *data)
{
	uint8_t reg0;
	uint8_t addr = 0xB2;//when reading a Register,the Address should be added with 0x80.
	
	//��������
	LT8920_CS_LOW();
    Delay_us(1);
	
    //д���ַ
    HAL_SPI_Transmit(&hspi1, &addr, 1, 500);
    Delay_us(1);
	
	//��һ���ֽڱ�ʾ���ݳ���
    HAL_SPI_Receive(&hspi1, &reg0, 1, 500);
    Delay_us(1);
	
	//��ȡʣ�µ�����
	while(bytes > 0)
	{
		HAL_SPI_Receive(&hspi1, data, 1, 500);
		Delay_us(1);
		data++;
		bytes--;
	}
    
    //�ͷ�����
	LT8920_CS_HIGH();
    Delay_us(1);
}

/*******************************************************************************
*ȫ�ֺ���
*����˵��: ͨ��LT8900���ⷢ����
*******************************************************************************/
bool LT8920_Transmit(uint8_t bytes, uint8_t* data, uint32_t timeout)
{
	//��ʱ
	uint32_t start = HAL_GetTick();
	
    //дreg52->0x8000 ��fifo
	WriteReg(7, 0x00, TxRx_CH); 
	WriteReg(52, 0x80, 0x80);
	
    //дreg50-> ������Ҫ�����ݵ�fifo
	WriteFIFO(bytes, data);
	
    //дreg7->0x10xxxxxxx 7λѡ��ͨ����
	WriteReg(7, 0x01, TxRx_CH); 
	
    //�ȴ�pkt��־λ���߻�ʱ
	while(1)
	{
		//���ͳɹ�
		if(LT8920_PKT_READ() != GPIO_PIN_RESET)
			return true;
		//���ͳ�ʱ(����û��������)
		if(HAL_GetTick() - start >= timeout)
			return false;
	}
}

/*******************************************************************************
*ȫ�ֺ���
*����˵��: ͨ��LT8900��������
*******************************************************************************/
bool LT8920_Receive(uint8_t bytes, uint8_t* data, uint32_t timeout)
{
	//��ʱ
	uint32_t start = HAL_GetTick();
	
    //дreg52->0x8080 ��fifo
	WriteReg(7, 0x00, TxRx_CH); 
	WriteReg(52, 0x80, 0x80);
	
    //дreg7->0x1xxxxxxx 7λѡ��ͨ����
	WriteReg(7, 0x00, (0x80 | TxRx_CH)); 
	
    //�ȴ�pkt��־λ����
	while(LT8920_PKT_READ() == GPIO_PIN_RESET)
	{
		//���ճ�ʱ(����û��������)
		if(HAL_GetTick() - start >= timeout)
        {
            WriteReg(7, 0x00, TxRx_CH); 
            printf("pkt timeout!");
            return false;
        }
			
	}
	
    //�ȴ��������
	while(((ReadReg(52) & 0x3F00) >> 8) < bytes)
	{
		//���ճ�ʱ(����û��������)
		if(HAL_GetTick() - start >= timeout)
        {
            WriteReg(7, 0x00, TxRx_CH); 
            printf("bytes timeout!");
            return false;
        }
			
	}
	
	//CRCУ��
    if((ReadReg(48) & 0x8000) == 0)
    {   
        //��reg50��������Ӧ����
		ReadFIFO(bytes, data);
		return true;       
    }
	else
	{
		return false;
	}
	
	
	//ֹͣ������
	
	
	
}

/*******************************************************************************
*ȫ�ֺ���
*����˵��: ɨ������ŵ����ź�ǿ�ȣ���80���ŵ�
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


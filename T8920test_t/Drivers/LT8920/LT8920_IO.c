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
#include "LT8920_IO.h"

//�����궨��


//��������
void WriteReg(uint8_t addr, uint8_t H, uint8_t L);
uint16_t ReadReg(uint8_t addr);

//ȫ�ֱ���
static uint8_t TxRxChannel;


/*******************************************************************************
*�� �� ��: LT8900_Init
*����˵��: LT8920оƬ��ʼ������
*��    �Σ���
*�� �� ֵ: ��
*******************************************************************************/
void LT8920_Init(void)
{
	//��ʼ������
	TxRxChannel = 0;

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
	WriteReg(41, 0xb0, 0x00 );//CRC is ON; scramble is OFF; �ر�AUTO_ACK
	WriteReg(42, 0xfd, 0xb0 );//�ȴ�RX_ACKʱ�� 176us
	WriteReg(43, 0x00, 0x0f );					
	WriteReg(44, 0x10, 0x00 );
	WriteReg(45, 0x05, 0x52 );		 //62.5k
	WriteReg(50, 0x00, 0x00 );
    HAL_Delay(100);
}

/*******************************************************************************
*����˵��: //�Զ�ѡ��һ������ͨ��
*��    �Σ���
*�� �� ֵ: ��
*******************************************************************************/
int LT8920_SelectIdleChannel(uint8_t ch_start, uint8_t ch_end, uint32_t listen_ms, uint8_t max_rssi) 
{
    bool selected = true;
    uint8_t ch;
    //�ӵ�һ��ͨ����ʼ�����ͨ��ռ��
    for(ch=ch_start; ch<=ch_end; ch+=8)
    {
        //TX_EN=0 RX_EN=1
        WriteReg(7, 0x00, (0x80|ch));
        Delay_us(2);
        
        //ÿ��ͨ�����ms_ch
        uint32_t start = HAL_GetTick();
        while((HAL_GetTick()-start) < listen_ms)
        {
            //�����ͨ�����յ����ݣ�����RSSI���ޣ�����
            if((LT8920_PKT_READ() != GPIO_PIN_RESET) || ((ReadReg(6)>>10) > max_rssi))
            {
                selected = false;
                break;
            }
            Delay_us(2);
        }
        WriteReg(7, 0x00, ch);
        Delay_us(2);
        
        //ѡ��ͨ��
        if(selected) 
        {
            LT8920_SetChannel(ch);
            break;
        }
    }  
    
    //����
    if(selected) return ch;
    else         return -1;
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

/*******************************************************************************
*ȫ�ֺ���
*����˵��: ͨ��LT8900���ⷢ����
*******************************************************************************/
bool LT8920_Transmit(uint8_t deviceID, uint8_t fun, uint8_t* data, uint8_t len, uint32_t timeout)
{
	//��ʱ
	uint32_t start = HAL_GetTick();
    uint8_t fifo = 50;
	
    //����ʱ����AUTO_ACK
    WriteReg(41, 0xb8, 0x00 );
    //TX_EN=0 RX_EN=0
	WriteReg(7, 0x00, TxRxChannel); 
    //��fifo�Ķ�дָ��
	WriteReg(52, 0x80, 0x80);
	
    //������Ҫ�����ݵ�fifo
	LT8920_CS_LOW();
    Delay_us(1);
	//д���ַ
	HAL_SPI_Transmit(&hspi1, &fifo, 1, 500);
    Delay_us(1);
	//д�����ݳ���
    len += 2;
    HAL_SPI_Transmit(&hspi1, &len, 1, 500);
    Delay_us(1);
    //д��ID
    HAL_SPI_Transmit(&hspi1, &deviceID, 1, 500);
    Delay_us(1);
    //д�빦����
    HAL_SPI_Transmit(&hspi1, &fun, 1, 500);
    Delay_us(1);
	//д�������ֽ�
    for(int i=0; i<(len-2); i++)
	{
		HAL_SPI_Transmit(&hspi1, data++, 1,500);
		Delay_us(1);
	}
	LT8920_CS_HIGH();
    Delay_us(1);
	
    //TX_EN=1 RX_EN=0
	WriteReg(7, 0x01, TxRxChannel); 
	
    //�ȴ�pkt��־λ����
	while((HAL_GetTick() - start) < timeout)
	{
		if(LT8920_PKT_READ() != GPIO_PIN_RESET)
			return true;	
	}
    return false;
}

/*******************************************************************************
*ȫ�ֺ���
*����˵��: ͨ��LT8900��������
*******************************************************************************/
bool LT8920_Receive(uint8_t* deviceID, uint8_t* fun, uint8_t* data, uint8_t *len, uint32_t timeout)
{
	//��ʱ
	uint32_t start = HAL_GetTick();
    uint8_t addr = 0xB2;  //(50 | 0x80).
    
    //����ʱ�ر�AUTO_ACK
    WriteReg(41, 0xb0, 0x00 );
    //TX_EN=0 RX_EN=0
	WriteReg(7, 0x00, TxRxChannel); 
    //��fifoָ��
	WriteReg(52, 0x80, 0x80);
	
    //TX_EN=0 RX_EN=1
	WriteReg(7, 0x00, (0x80 | TxRxChannel)); 
    
    //�ȴ����յ���һ���ֽ�
     while(HAL_GetTick() - start < timeout)
    {
        if(LT8920_PKT_READ() != GPIO_PIN_RESET)
        {
            //����fifo������
            LT8920_CS_LOW();
            Delay_us(1);
            //д���ַ
            HAL_SPI_Transmit(&hspi1, &addr, 1, 500);
            Delay_us(1);
            //�������ݳ���,
            HAL_SPI_Receive(&hspi1, len, 1, 500);
            Delay_us(1);
            break;
        }
    }
    
	//�ȴ����ݽ������
    while(HAL_GetTick() - start < timeout)
    {
        if((LT8920_PKT_READ() != GPIO_PIN_RESET) && (((ReadReg(52) & 0x3F00) >> 8) >= *len))
        {
            //CRCУ��
            if((ReadReg(48) & 0x8000) == 0)
            {   
                //����fifo������
                LT8920_CS_LOW();
                Delay_us(1);
                //д���ַ
                HAL_SPI_Transmit(&hspi1, &addr, 1, 500);
                Delay_us(1);
                //����ID
                HAL_SPI_Receive(&hspi1, deviceID, 1, 500);
                //���չ�����
                HAL_SPI_Receive(&hspi1, fun, 1, 500);
                Delay_us(1);
                //��ȡʣ�µ�����
                for(int i=0; i < (*len)-3; i++)
                {
                    HAL_SPI_Receive(&hspi1, data++, 1, 500);
                    Delay_us(1);
                }
                LT8920_CS_HIGH();
                Delay_us(1);
                
                //TX_EN=0 RX_EN=0
                WriteReg(7, 0x00, TxRxChannel); 
                
                //����ʵ�ʵ����ݳ���
                (*len) -= 3;
                return true;       
            }
            else
            {
                break;
            }
        }
    }
    //���ճ�ʱ
    WriteReg(7, 0x00, TxRxChannel); 
    return false;
}

/*******************************************************************************
*ȫ�ֺ���
*����˵��: ɨ������ŵ����ź�ǿ�ȣ���80���ŵ�
*******************************************************************************/
void LT8920_ScanRSSI(uint8_t* rssi_max, uint8_t* rssi_avg, int num)
{
    uint16_t sum;
    uint16_t tmp;
    for(int i=0; i<80; i++)
    {
        //ѡ��Ƶ��
        WriteReg(7, 0x00, 0x80+i);
        Delay_us(2);
        sum = 0;
        rssi_max[i] = 0;
        for(int j=0; j<num; j++)
        {
            //����RSSIֵ
            tmp = ReadReg(6)>>10;
            //ѡȡ���ֵ
            if(rssi_max[i]<tmp)
                rssi_max[i] = tmp;
            //����ƽ��ֵ
            sum += tmp;
            Delay_us(1);
        }
        rssi_avg[i] = sum/num;
        WriteReg(7, 0x00, i);
    }
}


/*******************************************************************************
*ȫ�ֺ���
*����˵��: ����ʹ�õ�ͨ��
*******************************************************************************/
void LT8920_SetChannel(uint8_t ch)
{
    TxRxChannel = ch;
    WriteReg(7, 0x00, ch);
    Delay_us(1);
}

/*******************************************************************************
*ȫ�ֺ���
*����˵��: ��ȡ��ǰʹ�õ�ͨ��
*******************************************************************************/
uint8_t LT8920_GetChannel(void)
{
    return TxRxChannel;
}




/*************************END OF FILE*****************************************/


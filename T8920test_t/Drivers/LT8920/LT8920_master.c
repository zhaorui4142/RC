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
#include "LT8920_master.h"
#include "stdio.h"

//�����궨��
#define LISTEN_MS 30
#define MAX_RSSI 20
#define MAX_SAVED_ID 8

//��������
uint8_t CalcDeviceID(void);
bool SaveRemoterID(uint8_t remoterID);
uint8_t LoadSavedID(uint8_t index);

//ȫ�ֱ���
static uint8_t TxRxBytes;
static uint8_t DeviceID;
static uint8_t LockedSlaveID;
//static uint8_t LockedMasterID;
static bool    isConnected;

/*******************************************************************************
*����˵��: ��ʼ��Ϊ����
*******************************************************************************/
void LT8920_MasterInit(uint8_t packet_length)
{
    //��ʼ���Ĵ�����оƬ
    LT8920_Init();
    
    //�Զ�ѡ��ͨ��
    LT8920_SelectIdleChannel(8, 80, 100, 20);
    
    //��ʼ���ڲ�����
    TxRxBytes = packet_length;
    DeviceID = CalcDeviceID();
    isConnected = false;
}

/*******************************************************************************
*����˵��: //���������������
*******************************************************************************/
bool LT8920_PairingRequest(uint32_t waiting_ms)
{
    //���͹㲥���ݰ�
    uint8_t buf[TxRxBytes];
    buf[0] = DeviceID;
    buf[1] = LT8920_GetChannel();//����ʹ�õ�Ƶ��
    LT8920_Transmit(DeviceID, FUN_PAIR_REQUEST, buf, TxRxBytes, 100);
    
    //�ȴ��ӻ���Ӧ
    uint8_t respones, receiverID;
    bool return_value = false;
    if(LT8920_Receive(&receiverID, &respones, buf, TxRxBytes, waiting_ms))
    {
        if(respones == FUN_PAIR_RESPONSE)
        {
            //���ӻ���IDд��flash
            SaveReceiverID(receiverID);
            return_value = true;
        }  
    }
    
    return return_value;
}

/*******************************************************************************
*����˵��: //�������Ҵӻ�,һ�����ҵ��ӻ�����һֱ������ӻ�������
*******************************************************************************/
bool LT8920_FindSlave(void)
{
    //flash��15ҳ
    uint8_t id, buf[TxRxBytes];
    uint8_t report_id,report_cmd;
    for(int i=0; i<8; i++)
    {
        //�����ӻ�ID
        id = LoadSavedID(i);
        
        //������ӻ�������
        buf[0] = id;                    //id
        buf[1] = LT8920_GetChannel();   //ch
        LT8920_Transmit(DeviceID, FUN_FIND_SLAVE, buf, TxRxBytes, 100);
        
        //�ȴ��ӻ���������
        if(LT8920_Receive(&report_id, &report_cmd, buf, 100))
        {
            //�ӻ��л�Ӧ
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
*����˵��: //�����л��ӻ�
*******************************************************************************/
bool LT8920_ChangeSlave(void)
{
    //flash��15ҳ
    uint8_t id, buf[TxRxBytes];
    uint8_t report_id,report_cmd;
    
    for(int i=0; i<MAX_SAVED_ID; i++)
    {
        //�����ӻ�ID
        id = LoadSavedID(i);
        
        //������ӻ�������
        buf[0] = id;        //id
        buf[1] = TxRx_CH;   //ch
        LT8920_Transmit(DeviceID, FUN_FIND_SLAVE, buf, 100);
        
        //�ȴ��ӻ���������
        if(LT8920_Receive(&report_id, &report_cmd, buf, 100))
        {
            //�ӻ��л�Ӧ
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
*����˵��: //�������ͣ����������ӻ��յ�Ҫ���ͷ����ź�
*******************************************************************************/
bool LT8920_CommunicateToSlaveWithFeedback(uint8_t* tx, uint8_t* rx, uint8_t* lost_count)
{
    //ID=receiverID^remoterID
    //FUN_CTRL_REQUEST
    LT8920_Transmit(DeviceID, FUN_PAIR_REQUEST, tx, 100);
}



/*******************************************************************************
*����˵��: //�������ͣ��������������дӻ������յ������ǲ���Ӧ
*******************************************************************************/
bool LT8920_CommunicateToSlaveWithoutFeedback(uint8_t* tx)
{
    //FUN_CTRL_FORCE
}    

/*******************************************************************************
*����˵��: //�ӻ��ȴ�����
*******************************************************************************/
bool LT8920_WaitMastersConnect(void)
{
    isConnected
}

/*******************************************************************************
*����˵��: //�ӻ��ȴ�����
*******************************************************************************/
bool LT8920_WaitMastersCommuncation(uint8_t* rx, uint8_t* feedback, uint32_t timeout, uint8_t* lost_count)
{
    //��������
    uint8_t id,fun,buf[TxRxBytes];
    if(LT8920_Receive(&id, &fun, buf, 100))
    {
        switch(fun)
        {
            //�������
            case FUN_PAIR_REQUEST:
            {
                if(id == LockedMasterID)
                {
                    //��Ӧ�㲥���ݰ�
                    LT8920_Transmit(DeviceID, FUN_PAIR_RESPONSE, buf, 100);
                    HAL_Delay(1);
            
                    //��¼ʹ�õ�Ƶ��
                    LT8920_SetChannel(buf[1]);
                }
                #define FUN_PAIR_RESPONSE 0x21
            }break;
            //��������
            case FUN_CTRL_REQUEST:
            {
                //ID=receiverID^remoterID
                #define FUN_CTRL_RESPONSE 0x22
            }break;
            //ֱ�ӿ�������
            case FUN_CTRL_FORCE:
            {
                
            }break;
            //���Ҵӻ�����
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
    //���������룬����Ƿ�����ԣ��������ԣ�ֱ�ӻ������Ӧ��
    

}


/*******************************************************************************
*�� �� ��: LT8900_Init
*����˵��: LT8920оƬ��ʼ������
*��    �Σ���
*�� �� ֵ: ��
*******************************************************************************/
void LT8920_Init(uint8_t packet_length)
{
    //����stm32f030��cpuid�����8λ��id�����Ա�ʶ����
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

	//��ʼ������
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
bool LT8920_Transmit(uint8_t deviceID, uint8_t fun, uint8_t* data, uint32_t timeout)
{
	//��ʱ
	uint32_t start = HAL_GetTick();
    uint8_t fifo = 50;
	
    //����ʱ����AUTO_ACK
    WriteReg(41, 0xb8, 0x00 );
    //TX_EN=0 RX_EN=0
	WriteReg(7, 0x00, TxRx_CH); 
    //��fifo�Ķ�дָ��
	WriteReg(52, 0x80, 0x80);
	
    //������Ҫ�����ݵ�fifo
	LT8920_CS_LOW();
    Delay_us(1);
	//д���ַ
	HAL_SPI_Transmit(&hspi1, &fifo, 1, 500);
    Delay_us(1);
	//д�����ݳ���
    uint8_t len = TxRxBytes+2;
    HAL_SPI_Transmit(&hspi1, &len, 1, 500);
    Delay_us(1);
    //д��ID
    HAL_SPI_Transmit(&hspi1, &deviceID, 1, 500);
    Delay_us(1);
    //д�빦����
    HAL_SPI_Transmit(&hspi1, &fun, 1, 500);
    Delay_us(1);
	//д�������ֽ�
    for(int i=0; i<TxRxBytes; i++)
	{
		HAL_SPI_Transmit(&hspi1, data++, 1,500);
		Delay_us(1);
	}
	LT8920_CS_HIGH();
    Delay_us(1);
	
    //TX_EN=1 RX_EN=0
	WriteReg(7, 0x01, TxRx_CH); 
	
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
bool LT8920_Receive(uint8_t* deviceID, uint8_t* fun, uint8_t* data, uint32_t timeout)
{
	//��ʱ
	uint32_t start = HAL_GetTick();
	uint8_t len;
    uint8_t addr = 0xB2;  //(50 | 0x80).
    
    //����ʱ�ر�AUTO_ACK
    WriteReg(41, 0xb0, 0x00 );
    //TX_EN=0 RX_EN=0
	WriteReg(7, 0x00, TxRx_CH); 
    //��fifoָ��
	WriteReg(52, 0x80, 0x80);
	
    //TX_EN=0 RX_EN=1
	WriteReg(7, 0x00, (0x80 | TxRx_CH)); 
	//�ȴ����ݽ������
    while(HAL_GetTick() - start < timeout)
    {
        if((LT8920_PKT_READ() != GPIO_PIN_RESET) && (((ReadReg(52) & 0x3F00) >> 8) >= TxRxBytes+2))
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
                //�������ݳ���
                HAL_SPI_Receive(&hspi1, &len, 1, 500);
                Delay_us(1);
                //����ID
                HAL_SPI_Receive(&hspi1, deviceID, 1, 500);
                //���չ�����
                HAL_SPI_Receive(&hspi1, fun, 1, 500);
                Delay_us(1);
                //��ȡʣ�µ�����
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
    //���ճ�ʱ
    WriteReg(7, 0x00, TxRx_CH); 
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
    TxRx_CH = ch;
    WriteReg(7, 0x00, ch);
    Delay_us(1);
}


/*******************************************************************************
*�ڲ�����
*����˵��: ��IDд��flash
*д�������ݷ���1��ԭ�����ݺ�д��������ͬ����0
*******************************************************************************/
bool SaveRemoterID(uint8_t remoterID)
{

    uint32_t addr = 0x08000000 + (15*1024);//д���15ҳ��λ��

    //��ȡ����
    uint32_t flashData = *(__IO uint32_t*)(addr);
    
    if((flashData & 0x000000FF) != remoterID)
    {
        //����flash
        HAL_FLASH_Unlock();

        //����ҳ
        FLASH_EraseInitTypeDef f;
        f.TypeErase = FLASH_TYPEERASE_PAGES;
        f.PageAddress = addr;
        f.NbPages = 1;

        uint32_t PageError = 0;
        HAL_FLASHEx_Erase(&f, &PageError);

        //���flash
        HAL_FLASH_Program(TYPEPROGRAM_WORD, addr, (uint32_t)remoterID);

        //��������
        HAL_FLASH_Lock();
        
        return true;
    }
    else
    {
        return false;
    }
}

/*******************************************************************************
*�ڲ�����
*����˵��: ��IDд��flash
*д�������ݷ���1��ԭ�����ݺ�д��������ͬ����0
*******************************************************************************/
bool SaveReceiverID(uint8_t receiverID)
{
    //����¼8����FIFO
    uint32_t addr = 0x08000000 + (15*1024);//д���15ҳ��λ��
    uint32_t flashData = *(__IO uint32_t*)(addr);
    
    //��鵱ǰID�Ƿ����,˳�����ԭ�ȵ�ID
    bool appear = false;
    uint8_t dat[8];
    for(int i=0; i<8; i++)
    {
        flashData = *(__IO uint32_t*)(addr + (i*4));
        dat[i] = flashData & 0x000000ff;
        
        if(dat[i] == receiverID)  appear = true;
    }
    
    //���flash��û�д�ID����д��FIFO����
    if(appear)
    {
        return false;
    }
    else
    {
        HAL_FLASH_Unlock();
        //����ҳ
        FLASH_EraseInitTypeDef f;
        f.TypeErase = FLASH_TYPEERASE_PAGES;
        f.PageAddress = addr;
        f.NbPages = 1;
        uint32_t PageError = 0;
        HAL_FLASHEx_Erase(&f, &PageError);

        //���flash
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
*�ڲ�����
*����˵��: ��ȡflash�е�ID
*******************************************************************************/
uint8_t LoadSavedID(uint8_t index)
{
    
}


//���㱾��ID
uint8_t CalcDeviceID(void)
{
    //����stm32f030��cpuid�����8λ��id�����Ա�ʶ����
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


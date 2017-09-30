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
#include "LT8920_slave.h"
#include "stdio.h"

//�����궨��
#define LISTEN_MS 100
#define MAX_RSSI 20

//��������
void WriteReg(uint8_t addr, uint8_t H, uint8_t L);
uint16_t ReadReg(uint8_t addr);
bool SaveRemoterID(uint8_t remoterID);
uint8_t LoadRemoterID(void);
uint8_t CalcSlaveDeviceID(void);

//ȫ�ֱ���
static uint8_t TxRxBytes;
static uint8_t DeviceID;
static uint8_t MasterID;


/*******************************************************************************
*����˵��: //��ʼ��
*******************************************************************************/
bool LT8920_SlaveInit(uint8_t packet_length)
{
    //��ʼ���Ĵ�����оƬ
    for(int i=0; i<3; i++)
    {
        if(LT8920_Init())
        {
            //�Զ�ѡ��ͨ��
            LT8920_SelectIdleChannel(0, 80, 100, 20);

            //��ʼ���ڲ�����
            TxRxBytes = packet_length;
            DeviceID = CalcSlaveDeviceID();
            MasterID = LoadRemoterID();
            return true;
        }
    }
    return false;
}

/*******************************************************************************
*����˵��: //�ӻ��ȴ����
*******************************************************************************/
bool LT8920_WaitPairing(uint32_t waiting_ms)
{
    
    uint32_t startTime = HAL_GetTick();
    uint8_t buf[TxRxBytes], ch=0;
    uint8_t remoterID,remoterCMD,remoterBytes;
    
    //��ⳬʱ
    while((HAL_GetTick() - startTime) < waiting_ms)
    {
        //�趨Ƶ��
        LT8920_SetChannel(ch);
        
         //���չ㲥���ݰ�
        if(LT8920_Receive(&remoterID, &remoterCMD, buf, &remoterBytes, LISTEN_MS))
        {printf("received ch = %d \n",ch);
            //�������
            if(remoterCMD == FUN_PAIR_REQUEST)
            {
                //��Ӧ�㲥���ݰ�
                WriteReg(35, 0x07, 0x00 );//�ط�������ʱ��Ϊ7��
                HAL_Delay(1);
                LT8920_Transmit(DeviceID, FUN_PAIR_RESPONSE, buf, TxRxBytes, 100);
                WriteReg(35, 0x03, 0x00 );//�ط������Ļ�3��
                HAL_Delay(1);
            
                //�趨Ƶ��
                LT8920_SetChannel(buf[1]);            
                //��¼����ID
                SaveRemoterID(remoterID);
                MasterID = remoterID;
                return true;
            }
            //��������
            if((remoterCMD == FUN_FIND_SLAVE) && (remoterID == MasterID))
            {
                //��Ӧ����
                LT8920_Transmit(DeviceID, FUN_FIND_RESPONSE, buf, TxRxBytes, 100);
                //�趨Ƶ��
                LT8920_SetChannel(buf[1]); 
                return true;
            }
        }
        
        //�л�Ƶ��
        //if(ch >= 80)    ch  = 0;
        //else            ch += 8;
    }

    return false;
}



/*******************************************************************************
*����˵��: //�ӻ��ȴ�����
*******************************************************************************/
bool LT8920_WaitConnect(uint32_t waiting_ms)
{
    uint32_t startTime = HAL_GetTick();
    uint8_t buf[TxRxBytes], ch=0;
    uint8_t remoterID,remoterCMD,remoterBytes;
    
    //��ⳬʱ
    while((HAL_GetTick() - startTime) < waiting_ms)
    {
        //�趨Ƶ��
        LT8920_SetChannel(ch);
        
         //���չ㲥���ݰ�
        if(LT8920_Receive(&remoterID, &remoterCMD, buf, &remoterBytes, LISTEN_MS))
        {
            //��������
            if((remoterCMD == FUN_FIND_SLAVE) && (remoterID == MasterID))
            {
                //��Ӧ����
                LT8920_Transmit(DeviceID, FUN_FIND_RESPONSE, buf, TxRxBytes, 100);
                //�趨Ƶ��
                LT8920_SetChannel(buf[1]); 
                return true;
            }
        }
        
        //�л�Ƶ��
        if(ch >= 80)    ch  = 0;
        else            ch += 8;
    }

    return false;
}

/*******************************************************************************
*����˵��: //�ӻ��ȴ�����
*******************************************************************************/
bool LT8920_WaitCommand(uint8_t* rx, uint8_t* feedback, uint32_t timeout, uint8_t* lost_count)
{
    //��������
    uint32_t startTime = HAL_GetTick();
    uint8_t remoterID,remoterCMD,remoterBytes;
    
    if(LT8920_Receive(&remoterID, &remoterCMD, rx, &remoterBytes, timeout))
    {
        switch(remoterCMD)
        {
            //�������
            case FUN_PAIR_REQUEST:
            {
                if((remoterID== MasterID) && (rx[0] == DeviceID))
                {
                    //������Ի�Ӧ
                    LT8920_Transmit(DeviceID, FUN_PAIR_RESPONSE, feedback, TxRxBytes, 100);
                    LT8920_SetChannel(rx[1]);
                    lost_count = 0;
                    return true;
                }
            }break;
            
            //���Ҵӻ�����
            case FUN_FIND_SLAVE:
            {
                if((remoterID== MasterID) && (rx[0] == DeviceID))
                {
                    //������Ի�Ӧ
                    LT8920_Transmit(DeviceID, FUN_FIND_RESPONSE, feedback, TxRxBytes, 100);
                    LT8920_SetChannel(rx[1]);
                    lost_count = 0;
                    return true;
                }
            }break;
            
            //��������
            case FUN_CTRL_REQUEST:
            {
                LT8920_Transmit(DeviceID, FUN_CTRL_RESPONSE, feedback, TxRxBytes, 100);
                lost_count = 0;
                return true;
            };
            
            //ֱ�ӿ�������
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


uint8_t LoadRemoterID(void)
{
    uint32_t addr = 0x08000000 + (15*1024);

    //��ȡ����
    uint32_t flashData = *(__IO uint32_t*)(addr);
    
    return (flashData & 0x000000FF);
}

/*******************************************************************************
*�ڲ�����
*����˵��: //���㱾��ID
*******************************************************************************/
uint8_t CalcSlaveDeviceID(void)
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
    
    return crc;
}







/*************************END OF FILE*****************************************/


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
#define WAIT_SLAVE_RESPOND_MS 100
#define MAX_RSSI 20
#define MAX_SAVED_ID 8

//��������
uint8_t CalcDeviceID(void);
bool SaveSlaveID(uint8_t ID);
bool LoadSavedID(uint8_t index, uint8_t* id);
bool LoadNextSavedID(uint8_t index, uint8_t* NextID);

//ȫ�ֱ���
static uint8_t TxRxBytes;
static uint8_t DeviceID;
static uint8_t LockedSlaveID;
static uint8_t LockedSlaveIndex;
static bool    LockedFlag;

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
    LockedFlag = false;
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
    uint8_t respones, receiverID, receivedBytes;
    bool return_value = false;
    if(LT8920_Receive(&receiverID, &respones, buf, &receivedBytes, waiting_ms))
    {
        if(respones == FUN_PAIR_RESPONSE)
        {
            //���ӻ���IDд��flash
            SaveSlaveID(receiverID);
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
    uint8_t id, buf[TxRxBytes];
    uint8_t report_id,report_cmd;
    for(int i=0; i<MAX_SAVED_ID; i++)
    {
        //�����ӻ�ID
        if(LoadSavedID(i, &id))
        {
            //������ӻ�������
            buf[0] = id;                    //id
            buf[1] = LT8920_GetChannel();   //ch
            LT8920_Transmit(DeviceID, FUN_FIND_SLAVE, buf, TxRxBytes, 100);
        
            //�ȴ��ӻ���������
            if(LT8920_Receive(&report_id, &report_cmd, buf, TxRxBytes, WAIT_SLAVE_RESPOND_MS))
            {
                //�ӻ��л�Ӧ
                if(report_cmd == FUN_FIND_RESPONSE)
                {
                    LockedSlaveID = id;
					LockedSlaveIndex = i;
                    LockedFlag = true;
                    return true;
                }
            }
        }
        else
        {
            break;
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
		if(LoadNextSavedID(LockedSlaveIndex+i, &id))
		{
			//������ӻ�������
			buf[0] = id;        			//id
			buf[1] = LT8920_GetChannel();   //ch
			LT8920_Transmit(DeviceID, FUN_FIND_SLAVE, buf, 100);
        
			//�ȴ��ӻ���������
			if(LT8920_Receive(&report_id, &report_cmd, buf, WAIT_SLAVE_RESPOND_MS))
			{
				//�ӻ��л�Ӧ
				if(report_cmd == FUN_FIND_RESPONSE)
				{
					if(LockedSlaveID != id)
					{
						LockedSlaveID = id;
						LockedSlaveIndex = i;
						LockedFlag = true;
						return true;
					}
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
*�ڲ�����
*����˵��: ��IDд��flash
*д�������ݷ���1��ԭ�����ݺ�д��������ͬ����0
*******************************************************************************/
bool SaveSlaveID(uint8_t ID)
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
bool LoadSavedID(uint8_t index, uint8_t* id)
{
    
}

/*******************************************************************************
*�ڲ�����
*����˵��: ��ȡflash�е�ID��һ��id
*******************************************************************************/
bool LoadNextSavedID(uint8_t index, uint8_t* NextID)
{
	
}

/*******************************************************************************
*�ڲ�����
*����˵��: //���㱾��ID
*******************************************************************************/
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
    
    return crc;
}







/*************************END OF FILE*****************************************/


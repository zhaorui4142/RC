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
#define MAX_SAVED_ID 8

//��������
uint8_t CalcMasterDeviceID(void);
uint8_t SaveSlaveID(uint8_t ID);
bool LoadSavedID(uint8_t index, uint8_t* id);

//ȫ�ֱ���
static uint8_t TxRxBytes;
static uint8_t DeviceID;
static uint8_t LockedSlaveID;
static uint8_t LockedSlaveIndex;
//static bool    LockedFlag;

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
    DeviceID = CalcMasterDeviceID();
    //LockedFlag = false;
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
    if(LT8920_Receive(&receiverID, &respones, buf, &receivedBytes, waiting_ms))
    {
        if(respones == FUN_PAIR_RESPONSE)
        {
            //���ӻ���IDд��flash
            LockedSlaveID = receiverID;
            LockedSlaveIndex = SaveSlaveID(receiverID);
            
            return true;
        }  
    }
    
    return false;
}

/*******************************************************************************
*����˵��: //�������Ҵӻ�,һ�����ҵ��ӻ�����һֱ������ӻ�������
*******************************************************************************/
bool LT8920_FindSlave(void)
{
    uint8_t id, buf[TxRxBytes];
    uint8_t report_id,report_cmd, report_bytes;
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
*����˵��: //�����л��ӻ�
*******************************************************************************/
bool LT8920_ChangeSlave(void)
{
    uint8_t id, buf[TxRxBytes];
    uint8_t report_id,report_cmd,report_bytes;
    int idx = LockedSlaveIndex, i = 0;
    
    while(i++ < MAX_SAVED_ID)
    {
        //������һ���ӻ���ID
		if(LoadSavedID(idx, &id))
		{
			//������ӻ�������
			buf[0] = id;        			//id
			buf[1] = LT8920_GetChannel();   //ch
			LT8920_Transmit(DeviceID, FUN_FIND_SLAVE, buf, TxRxBytes, 100);
        
			//�ȴ��ӻ���������
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
*����˵��: //�������ͣ����������ӻ��յ�Ҫ���ͷ����ź�
*******************************************************************************/
bool LT8920_CommunicateToSlaveWithFeedback(uint8_t* tx, uint8_t* rx, uint8_t* lost_count)
{
    uint8_t report_id,report_cmd,report_bytes;

    //��������
	LT8920_Transmit(DeviceID, FUN_CTRL_REQUEST, tx, TxRxBytes, 100);
        
	//�ȴ��ӻ���������
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
*����˵��: //�������ͣ��������������дӻ������յ������ǲ���Ӧ
*******************************************************************************/
bool LT8920_CommunicateToSlaveWithoutFeedback(uint8_t* tx)
{
    //��������
	LT8920_Transmit(DeviceID, FUN_CTRL_FORCE, tx, TxRxBytes, 100);
    return true;
}    




/*******************************************************************************
*�ڲ�����
*����˵��: ��IDд��flash
*���� index
*******************************************************************************/
uint8_t SaveSlaveID(uint8_t ID)
{
    //����¼8����FIFO����32λ����д��
    uint32_t addr = 0x08000000 + (15*1024);//��15ҳ��λ��
    uint32_t flashData = *(__IO uint32_t*)(addr);
    
    //��鵱ǰID�Ƿ����,˳�����ԭ�ȵ�ID
    uint8_t dat[MAX_SAVED_ID];
    for(int i=0; i<MAX_SAVED_ID; i++)
    {
        flashData = *(__IO uint32_t*)(addr + (i*4));
        dat[i] = flashData & 0x000000ff;
        
        if(dat[i] == ID)  return i;
    }
    
    //���flash��û�д�ID����д��FIFO����
    HAL_FLASH_Unlock();
    //����ҳ
    FLASH_EraseInitTypeDef f;
    f.TypeErase = FLASH_TYPEERASE_PAGES;
    f.PageAddress = addr;
    f.NbPages = 1;
    uint32_t PageError = 0;
    HAL_FLASHEx_Erase(&f, &PageError);
    //���flash
    HAL_FLASH_Program(TYPEPROGRAM_WORD, addr, (uint32_t)ID);
    for(int i=0; i<MAX_SAVED_ID; i++)
    {
        HAL_FLASH_Program(TYPEPROGRAM_WORD, (addr + i*4), (uint32_t)dat[i]);
    }
    HAL_FLASH_Lock();
        
    return 0;
}


/*******************************************************************************
*�ڲ�����
*����˵��: ��ȡflash�е�ID
*******************************************************************************/
bool LoadSavedID(uint8_t index, uint8_t* id)
{
    uint32_t addr = 0x08000000 + (15*1024);//��15ҳ��λ��
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
*�ڲ�����
*����˵��: //���㱾��ID
*******************************************************************************/
uint8_t CalcMasterDeviceID(void)
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


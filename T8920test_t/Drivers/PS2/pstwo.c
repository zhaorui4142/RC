
/*********************************************************

**********************************************************/	 

//头文件包含
#include "pstwo.h"

//常数宏定义
#define CTRL_CLK        5
#define CTRL_CLK_HIGH   5
#define CTRL_BYTE_DELAY 4


//内部函数声明区
bool ReadWriteSpi(uint8_t* tx, uint8_t* rx, uint8_t bytes); //串行口输入输出
void SendCommandBytes(uint8_t* string, uint8_t len);
void ReconfigGamepad(void);
bool EnablePressures(void);
void EnableRumble(void);
uint8_t ReadType(void);

//静态变量声明区
__align(4) static uint8_t CMD_EnterConfig[5]   = {0x01,0x43,0x00,0x01,0x00};
__align(4) static uint8_t CMD_ReadType[9]      = {0x01,0x45,0x00,0x5A,0x5A,0x5A,0x5A,0x5A,0x5A};
__align(4) static uint8_t CMD_SetMode[9]       = {0x01,0x44,0x00,0x01,0x03,0x00,0x00,0x00,0x00};
__align(4) static uint8_t CMD_SetBytesLarge[9] = {0x01,0x4F,0x00,0xFF,0xFF,0x03,0x00,0x00,0x00};
__align(4) static uint8_t CMD_ExitConfig[9]    = {0x01,0x43,0x00,0x00,0x5A,0x5A,0x5A,0x5A,0x5A};
__align(4) static uint8_t CMD_EnableRumble[5]  = {0x01,0x4D,0x00,0x00,0x01};
__align(4) static uint8_t PS2data[21];

__align(4) static uint8_t TxBuf[32];
__align(4) static uint8_t RxBuf[32];

static uint8_t read_delay,controller_type;
static uint16_t buttons,last_buttons;
static uint32_t last_read;
static bool en_Pressures,en_Rumble;


/****************************************************************************************/
//配置SPI端口
void PS2X_ConfigSPI(void)
{
    hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;

    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_LSB;

    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Error_Handler();
    }
}

/****************************************************************************************/
//按钮状态改变
bool PS2X_IsButtonOnToggle(uint16_t button) 
{
    return (((last_buttons ^ buttons) & button) > 0);
}

/****************************************************************************************/
//按键被按下
bool PS2X_IsButtonOnPress(unsigned int button)
{
    return(PS2X_IsButtonOnToggle(button) & PS2X_GetButtonState(button));
}

/****************************************************************************************/
//按键被松开
bool PS2X_IsButtonOnRelease(unsigned int button)
{
    return((PS2X_IsButtonOnToggle(button)) & ((~last_buttons & button) > 0));
}

/****************************************************************************************/
//查询某个按键状态
bool PS2X_GetButtonState(uint16_t button) 
{
    return ((~buttons & button) > 0);
}

/****************************************************************************************/
//查询全部按钮的状态
uint16_t PS2X_GetButtonData(void)
{
   return (~buttons);
}

/****************************************************************************************/
//查询模拟量输出
uint8_t PS2X_GetAnalogValue(uint8_t button)
{
   return PS2data[button];
}

/****************************************************************************************/
//串行口输入输出
bool ReadWriteSpi(uint8_t* tx, uint8_t* rx, uint8_t bytes) 
{
    if(rx != NULL)
    {
        for(int i=0; i<bytes; i++)
        {
            Delay_us(41);
            if(HAL_SPI_TransmitReceive(&hspi1, tx+i, rx+i, 1, 500) != HAL_OK)
                return false;
        }
        return true;
    }
    else
    {
        for(int i=0; i<bytes; i++)
        {
            Delay_us(41);
            if(HAL_SPI_Transmit(&hspi1, tx+i, 1, 500) != HAL_OK)
                return false;
        }
        return true;
    }
    
}


/****************************************************************************************/
//读取数据
bool PS2X_ReadGamepad(bool motor1, uint8_t motor2)
{
    //振动设置motor1  0xFF开，其他关，motor2  0x40~0xFF
    //检测手柄等待时间
    uint32_t ElapsedMsFromLastRead = HAL_GetTick() - last_read;
printf("PS2X_ReadGamepad 1 \n");
    if (ElapsedMsFromLastRead > 1500) //waited to long
        ReconfigGamepad();

    if(ElapsedMsFromLastRead < read_delay)  //waited too short
        HAL_Delay(read_delay - ElapsedMsFromLastRead);

    if(motor2 < 0x40)
        motor2 = 0x40; //noting below 40 will make it spin

    TxBuf[0] = 0x01;
    TxBuf[1] = 0x42;
    TxBuf[2] = 0;
    TxBuf[3] = motor1;
    TxBuf[4] = motor2;

    // Try a few times to get valid data...
    for (uint8_t RetryCnt = 0; RetryCnt < 5; RetryCnt++) 
    {
        // low enable joystick
        PS2X_ATT_CLR(); 

        //HAL_Delay(CTRL_BYTE_DELAY);
        //Send the command to send button and joystick data;
        ReadWriteSpi(TxBuf, PS2data, 9); 

        //if controller is in full data return mode, get the rest of data
        if(PS2data[1] == 0x79)  
            ReadWriteSpi(TxBuf+9, PS2data+9, 12); 

        // HI disable joystick
        PS2X_ATT_SET(); 
    
        // Check to see if we received valid data or not.  
        // We should be in analog mode for our data to be valid (analog == 0x7_)
        if ((PS2data[1] & 0xf0) == 0x70)
            break;

        // If we got to here, we are not in analog mode, try to recover...
        ReconfigGamepad(); // try to get back into Analog mode.
        HAL_Delay(read_delay);
   }

   // If we get here and still not in analog mode (=0x7_), try increasing the read_delay...
   if ((PS2data[1] & 0xf0) != 0x70) 
   {
      if (read_delay < 10)
         read_delay++;   // see if this helps out...
   }
   
    //store the previous buttons states
    last_buttons = buttons; 
   
    //store as one value for multiple functions
    buttons =  (uint16_t)(PS2data[4] << 8) + PS2data[3];   

    last_read = HAL_GetTick();
   
    // 1 = OK = analog mode - 0 = NOK
    return ((PS2data[1] & 0xf0) == 0x70);  
}


/****************************************************************************************/
//配置
uint8_t PS2X_ConfigGamepad(bool pressures, bool rumble) 
{
    printf("start config gamepad \n");
    //new error checking. First, read gamepad a few times to see if it's talking
    PS2X_ReadGamepad(false, 0x00);
    PS2X_ReadGamepad(false, 0x00);

    //see if it talked - see if mode came back. 
    //If still anything but 41, 73 or 79, then it's not talking
    printf("PS2data1: %x \n", PS2data[1]);
    if(PS2data[1] != 0x41 && PS2data[1] != 0x73 && PS2data[1] != 0x79)
    { 
        return 1; //return error code 1
    }

    //try setting mode, increasing delays if need be.
    read_delay = 1;
    

    
    for(int y = 0; y <= 10; y++) 
    {
        SendCommandBytes(CMD_EnterConfig, sizeof(CMD_EnterConfig));

        //read type
        HAL_Delay(CTRL_BYTE_DELAY);
        PS2X_ATT_CLR(); 
        HAL_Delay(CTRL_BYTE_DELAY);
        ReadWriteSpi(CMD_ReadType, RxBuf, 9); 
        PS2X_ATT_SET(); 

        controller_type = RxBuf[3];

        SendCommandBytes(CMD_SetMode, sizeof(CMD_SetMode));
        
        if(rumble)
        {
            SendCommandBytes(CMD_EnableRumble, sizeof(CMD_EnableRumble));
            en_Rumble = true; 
        }
        if(pressures)
        {
            SendCommandBytes(CMD_SetBytesLarge, sizeof(CMD_SetBytesLarge));
            en_Pressures = true;
        }
        
        SendCommandBytes(CMD_ExitConfig, sizeof(CMD_ExitConfig));

        PS2X_ReadGamepad(false, 0x00);

        if(pressures)
        {
            if(PS2data[1] == 0x79)
                break;
            if(PS2data[1] == 0x73)
                return 3;
        }

        if(PS2data[1] == 0x73)
            break;

        if(y == 10)
        {
            return 2; //exit function with error
        }
        read_delay += 1; //add 1ms to read_delay
    }
    return 0; //no error if here
}

/****************************************************************************************/
void SendCommandBytes(uint8_t* string, uint8_t len) 
{
    PS2X_ATT_CLR(); // low enable joystick
    ReadWriteSpi(string, NULL, len); 
    PS2X_ATT_SET(); //high disable joystick
    HAL_Delay(read_delay);                  //wait a few
}

/****************************************************************************************/
uint8_t ReadType() 
{
  if(controller_type == 0x03)
    return 1;
  else if(controller_type == 0x01)
    return 2;
  else if(controller_type == 0x0C)  
    return 3;  //2.4G Wireless Dual Shock PS2 Game Controller
	
  return 0;
}

/****************************************************************************************/
void EnableRumble()
{
  SendCommandBytes(CMD_EnterConfig, sizeof(CMD_EnterConfig));
  SendCommandBytes(CMD_EnableRumble, sizeof(CMD_EnableRumble));
  SendCommandBytes(CMD_ExitConfig, sizeof(CMD_ExitConfig));
  en_Rumble = true;
}

/****************************************************************************************/
bool EnablePressures()
{
  SendCommandBytes(CMD_EnterConfig, sizeof(CMD_EnterConfig));
  SendCommandBytes(CMD_SetBytesLarge, sizeof(CMD_SetBytesLarge));
  SendCommandBytes(CMD_ExitConfig, sizeof(CMD_ExitConfig));

  PS2X_ReadGamepad(false, 0x00);
  PS2X_ReadGamepad(false, 0x00);

  if(PS2data[1] != 0x79)
    return false;

  en_Pressures = true;
    return true;
}

/****************************************************************************************/
void ReconfigGamepad()
{
  SendCommandBytes(CMD_EnterConfig, sizeof(CMD_EnterConfig));
  SendCommandBytes(CMD_SetMode, sizeof(CMD_SetMode));
  if (en_Rumble)
    SendCommandBytes(CMD_EnableRumble, sizeof(CMD_EnableRumble));
  if (en_Pressures)
    SendCommandBytes(CMD_SetBytesLarge, sizeof(CMD_SetBytesLarge));
  SendCommandBytes(CMD_ExitConfig, sizeof(CMD_ExitConfig));
}

/****************************************************************************************/



















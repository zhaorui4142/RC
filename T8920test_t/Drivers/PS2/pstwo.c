
/*********************************************************

**********************************************************/	 

//头文件包含
#include "pstwo.h"



#define CTRL_CLK        5
#define CTRL_CLK_HIGH   5
#define CTRL_BYTE_DELAY 4


static uint8_t enter_config[5]={0x01,0x43,0x00,0x01,0x00};
static uint8_t type_read[9]={0x01,0x45,0x00,0x5A,0x5A,0x5A,0x5A,0x5A,0x5A};
static uint8_t set_mode[9]={0x01,0x44,0x00,0x01,0x03,0x00,0x00,0x00,0x00};
static uint8_t set_bytes_large[9]={0x01,0x4F,0x00,0xFF,0xFF,0x03,0x00,0x00,0x00};
static uint8_t exit_config[9]={0x01,0x43,0x00,0x00,0x5A,0x5A,0x5A,0x5A,0x5A};
static uint8_t enable_rumble[5]={0x01,0x4D,0x00,0x00,0x01};





static uint8_t i;
static uint16_t last_buttons;
static uint16_t buttons;
static uint32_t last_read;
static uint8_t read_delay;
static uint8_t controller_type;
static bool en_Rumble;
static bool en_Pressures;
static uint8_t PS2data[21];






bool PS2X_ButtonStateChanged(uint16_t button);//按钮状态改变
bool PS2X_ButtonPressed(unsigned int button);//按键被按下
bool PS2X_ButtonReleased(unsigned int button);//按键被松开
bool PS2X_Button(uint16_t button);//查询某个按键状态
uint16_t PS2X_ButtonDataByte();//查询全部按钮的状态
uint8_t PS2X_Analog(uint8_t button);//查询模拟量输出
bool PS2X_TxRxBytes(uint8_t* tx, uint8_t* rx, uint8_t bytes); //串行口输入输出
bool PS2X_read_gamepad(bool motor1, uint8_t motor2);//读取数据
void PS2X_sendCommandString(uint8_t* string, uint8_t len);
void PS2X_reconfig_gamepad(void);


/****************************************************************************************/
//按钮状态改变
bool PS2X_ButtonStateChanged(uint16_t button) 
{
    return (((last_buttons ^ buttons) & button) > 0);
}

/****************************************************************************************/
//按键被按下
bool PS2X_ButtonPressed(unsigned int button)
{
    return(PS2X_ButtonStateChanged(button) & PS2X_Button(button));
}

/****************************************************************************************/
//按键被松开
bool PS2X_ButtonReleased(unsigned int button)
{
    return((PS2X_ButtonStateChanged(button)) & ((~last_buttons & button) > 0));
}

/****************************************************************************************/
//查询某个按键状态
bool PS2X_Button(uint16_t button) 
{
    return ((~buttons & button) > 0);
}

/****************************************************************************************/
//查询全部按钮的状态
uint16_t PS2X_ButtonDataByte()
{
   return (~buttons);
}

/****************************************************************************************/
//查询模拟量输出
uint8_t PS2X_Analog(uint8_t button)
{
   return PS2data[button];
}

/****************************************************************************************/
//串行口输入输出
bool PS2X_TxRxBytes(uint8_t* tx, uint8_t* rx, uint8_t bytes) 
{
    if(HAL_SPI_TransmitReceive(&hspi1, tx, rx, bytes, 500) == HAL_OK)
        return true;
    else
        return false;
}


/****************************************************************************************/
//读取数据
bool PS2X_read_gamepad(bool motor1, uint8_t motor2)
{
    //振动设置motor1  0xFF开，其他关，motor2  0x40~0xFF
    //检测手柄等待时间
    uint32_t ElapsedMsFromLastRead = HAL_GetTick() - last_read;

    if (ElapsedMsFromLastRead > 1500) //waited to long
        PS2X_reconfig_gamepad();

    if(ElapsedMsFromLastRead < read_delay)  //waited too short
        HAL_Delay(read_delay - ElapsedMsFromLastRead);

    if(motor2 < 0x40)
        motor2 = 0x40; //noting below 40 will make it spin

    uint8_t TxBuf[21] = {0x01,0x42,0,motor1,motor2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    // Try a few times to get valid data...
    for (uint8_t RetryCnt = 0; RetryCnt < 5; RetryCnt++) 
    {
        // low enable joystick
        ATT_CLR(); 

        HAL_Delay(CTRL_BYTE_DELAY);
        //Send the command to send button and joystick data;
        PS2X_TxRxBytes(TxBuf, PS2data, 9); 

        //if controller is in full data return mode, get the rest of data
        if(PS2data[1] == 0x79)  
            PS2X_TxRxBytes(TxBuf+9, PS2data+9, 12); 

        // HI disable joystick
        ATT_SET(); 
    
        // Check to see if we received valid data or not.  
        // We should be in analog mode for our data to be valid (analog == 0x7_)
        if ((PS2data[1] & 0xf0) == 0x70)
            break;

        // If we got to here, we are not in analog mode, try to recover...
        PS2X_reconfig_gamepad(); // try to get back into Analog mode.
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
uint8_t PS2X_config_gamepad(uint8_t clk, uint8_t cmd, uint8_t att, uint8_t dat, bool pressures, bool rumble) 
{
    //new error checking. First, read gamepad a few times to see if it's talking
    PS2X_read_gamepad(false, 0x00);
    PS2X_read_gamepad(false, 0x00);

    //see if it talked - see if mode came back. 
    //If still anything but 41, 73 or 79, then it's not talking
    if(PS2data[1] != 0x41 && PS2data[1] != 0x73 && PS2data[1] != 0x79)
    { 
        return 1; //return error code 1
    }

    //try setting mode, increasing delays if need be.
    read_delay = 1;
    
    uint8_t RxBuf[9];
    
    for(int y = 0; y <= 10; y++) 
    {
        PS2X_sendCommandString(enter_config, sizeof(enter_config));

        //read type
        HAL_Delay(CTRL_BYTE_DELAY);
        ATT_CLR(); 
        HAL_Delay(CTRL_BYTE_DELAY);
        PS2X_TxRxBytes(type_read, RxBuf, 9); 
        ATT_SET(); 

        controller_type = RxBuf[3];

        PS2X_sendCommandString(set_mode, sizeof(set_mode));
        
        if(rumble)
        {
            PS2X_sendCommandString(enable_rumble, sizeof(enable_rumble));
            en_Rumble = true; 
        }
        if(pressures)
        {
            PS2X_sendCommandString(set_bytes_large, sizeof(set_bytes_large));
            en_Pressures = true;
        }
        
        PS2X_sendCommandString(exit_config, sizeof(exit_config));

        PS2X_read_gamepad(false, 0x00);

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
void PS2X_sendCommandString(uint8_t* string, uint8_t len) 
{
    uint8_t rx[len];
    ATT_CLR(); // low enable joystick
    PS2X_TxRxBytes(string, rx, len); 
    ATT_SET(); //high disable joystick
    HAL_Delay(read_delay);                  //wait a few
}

/****************************************************************************************/
uint8_t PS2X_readType() 
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
void PS2X_enableRumble() 
{
  PS2X_sendCommandString(enter_config, sizeof(enter_config));
  PS2X_sendCommandString(enable_rumble, sizeof(enable_rumble));
  PS2X_sendCommandString(exit_config, sizeof(exit_config));
  en_Rumble = true;
}

/****************************************************************************************/
bool PS2X_enablePressures() 
{
  PS2X_sendCommandString(enter_config, sizeof(enter_config));
  PS2X_sendCommandString(set_bytes_large, sizeof(set_bytes_large));
  PS2X_sendCommandString(exit_config, sizeof(exit_config));

  PS2X_read_gamepad(false, 0x00);
  PS2X_read_gamepad(false, 0x00);

  if(PS2data[1] != 0x79)
    return false;

  en_Pressures = true;
    return true;
}

/****************************************************************************************/
void PS2X_reconfig_gamepad()
{
  PS2X_sendCommandString(enter_config, sizeof(enter_config));
  PS2X_sendCommandString(set_mode, sizeof(set_mode));
  if (en_Rumble)
    PS2X_sendCommandString(enable_rumble, sizeof(enable_rumble));
  if (en_Pressures)
    PS2X_sendCommandString(set_bytes_large, sizeof(set_bytes_large));
  PS2X_sendCommandString(exit_config, sizeof(exit_config));
}

/****************************************************************************************/



















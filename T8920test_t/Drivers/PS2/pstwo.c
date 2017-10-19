
/*********************************************************

**********************************************************/	 

//头文件包含
#include "pstwo.h"

//常数宏定义
#define CTRL_CLK        5
#define CTRL_CLK_HIGH   5
#define CTRL_BYTE_DELAY 4

#define READ_DELAY_MS   15

//内部函数声明区
uint8_t GamepadTxRx(uint8_t* tx, uint8_t* rx);
void ReconfigGamepad(void);
bool EnablePressures(void);
void EnableRumble(void);
uint8_t ReadType(void);

//静态变量声明区
__align(4) static uint8_t CMD_EnterConfig[9]   = {0x01,0x43,0x00,0x01,0x00,0x00,0x00,0x00,0x00};
__align(4) static uint8_t CMD_ExitConfig[9]    = {0x01,0x43,0x00,0x00,0x5A,0x5A,0x5A,0x5A,0x5A};
__align(4) static uint8_t CMD_ReadType[9]      = {0x01,0x45,0x00,0x5A,0x5A,0x5A,0x5A,0x5A,0x5A};
__align(4) static uint8_t CMD_SetMode[9]       = {0x01,0x44,0x00,0x01,0x03,0x00,0x00,0x00,0x00};
__align(4) static uint8_t CMD_SetBytesLarge[9] = {0x01,0x4F,0x00,0xFF,0xFF,0x03,0x00,0x00,0x00};
__align(4) static uint8_t CMD_EnableRumble[9]  = {0x01,0x4D,0x00,0x00,0x01,0xFF,0xFF,0xFF,0xFF};
__align(4) static uint8_t CMD_ReadGamepad[21]  = {0x01,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
__align(4) static uint8_t PS2data[21];

__align(4) static uint8_t RxBuf[21];

static uint8_t controller_type;
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


//读取数据
uint8_t GamepadTxRx(uint8_t* tx, uint8_t* rx)
{
    uint8_t mode,bytes;
    //片选
    PS2X_ATT_CLR(); 
    Delay_us(41);
    
    //发送0x01
    if(HAL_SPI_TransmitReceive(&hspi1, tx, rx, 1, 500) != HAL_OK)
    {
        PS2X_ATT_SET(); 
        return 0;
    }
        
    tx++, rx++;
    Delay_us(41);

    //发送cmd 同时接收mode
    if(HAL_SPI_TransmitReceive(&hspi1, tx, rx, 1, 500) != HAL_OK)
    {
        PS2X_ATT_SET(); 
        return 0;
    }
        
    Delay_us(41);
    mode = rx[0];
    tx++, rx++;
    
    //计算剩余字节
    switch(mode)
    {
        case 0x41:
            bytes = 3;
            break;
        
        case 0x73:
            bytes = 7;
            break;
        
        case 0x79:
            bytes = 19;
            break;
        
        case 0xF3:
            bytes = 7;
            break;
        
        default:
            PS2X_ATT_SET(); 
            return 0;
    }
    
    //发送剩余字节
    for(int i=0; i<bytes; i++)
    {
        if(HAL_SPI_TransmitReceive(&hspi1, tx+i, rx+i, 1, 500) != HAL_OK)
            return 0;
        Delay_us(41);
    }
    
    //取消片选
    PS2X_ATT_SET(); 
    
    //返回手柄当前工作模式
    return mode;
}

/****************************************************************************************/
//读取数据
bool PS2X_ReadGamepad(bool motor1, uint8_t motor2)
{
    //振动设置motor1  0xFF开，其他关，motor2  0x40~0xFF
    //检测手柄等待时间
    uint32_t ElapsedMsFromLastRead = HAL_GetTick() - last_read;

    if (ElapsedMsFromLastRead > 1500) //waited to long
        ReconfigGamepad();

    if(ElapsedMsFromLastRead < READ_DELAY_MS)  //waited too short
        HAL_Delay(READ_DELAY_MS - ElapsedMsFromLastRead);

    if(motor2 < 0x40)
        motor2 = 0x40; //noting below 40 will make it spin

    CMD_ReadGamepad[3] = motor1;
    CMD_ReadGamepad[4] = motor2;
    
    
    // Try a few times to get valid data...
    for (uint8_t RetryCnt = 0; RetryCnt < 5; RetryCnt++) 
    {
        GamepadTxRx(CMD_ReadGamepad, PS2data);
    
        // Check to see if we received valid data or not.  
        // We should be in analog mode for our data to be valid (analog == 0x7_)
        if ((PS2data[1] & 0xf0) == 0x70)
            break;

        // If we got to here, we are not in analog mode, try to recover...
        ReconfigGamepad(); // try to get back into Analog mode.
        HAL_Delay(READ_DELAY_MS);
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
    en_Rumble = rumble; 
    en_Pressures = pressures;
    
    //new error checking. First, read gamepad a few times to see if it's talking
    PS2X_ReadGamepad(false, 0x00);
    PS2X_ReadGamepad(false, 0x00);

    //see if it talked - see if mode came back. 
    //If still anything but 41, 73 or 79, then it's not talking
    if(PS2data[1] != 0x73 && PS2data[1] != 0x79)
        return 1; //return error code 1
    else
        return 0; //no error if here
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
    HAL_Delay(READ_DELAY_MS);
    GamepadTxRx(CMD_EnterConfig, RxBuf);
    HAL_Delay(READ_DELAY_MS);
    GamepadTxRx(CMD_EnableRumble, RxBuf);
    HAL_Delay(READ_DELAY_MS);
    GamepadTxRx(CMD_ExitConfig, RxBuf);
    en_Rumble = true;
}

/****************************************************************************************/
bool EnablePressures()
{
    HAL_Delay(READ_DELAY_MS);
    GamepadTxRx(CMD_EnterConfig, RxBuf);
    HAL_Delay(READ_DELAY_MS);
    GamepadTxRx(CMD_SetBytesLarge, RxBuf);
    HAL_Delay(READ_DELAY_MS);
    GamepadTxRx(CMD_ExitConfig, RxBuf);
    
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
    //进入设置
    HAL_Delay(READ_DELAY_MS);
    GamepadTxRx(CMD_EnterConfig, RxBuf);
    
    //读取控制器类型
    HAL_Delay(READ_DELAY_MS);
    GamepadTxRx(CMD_ReadType, RxBuf);
    controller_type = RxBuf[3];
    
    //设置红灯模式
    HAL_Delay(READ_DELAY_MS);
    GamepadTxRx(CMD_SetMode, RxBuf);

    //使能震动
    if (en_Rumble)
    {
        HAL_Delay(READ_DELAY_MS);
        GamepadTxRx(CMD_EnableRumble, RxBuf);
    }
    
    //使能压感
    if (en_Pressures)
    {
        HAL_Delay(READ_DELAY_MS);
        GamepadTxRx(CMD_SetBytesLarge, RxBuf);
    }
    
    //退出设置
    HAL_Delay(READ_DELAY_MS);
    GamepadTxRx(CMD_ExitConfig, RxBuf);
}

/****************************************************************************************/



















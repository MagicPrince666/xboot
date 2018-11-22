#include "pstwo.h"


u16_t Handkey;
u8_t Comd[2]={0x01,0x42};	
u8_t Data[9]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //
u16_t MASK[]={
    PSB_SELECT,
    PSB_L3,
    PSB_R3 ,
    PSB_START,
    PSB_PAD_UP,
    PSB_PAD_RIGHT,
    PSB_PAD_DOWN,
    PSB_PAD_LEFT,
    PSB_L2,
    PSB_R2,
    PSB_L1,
    PSB_R1 ,
    PSB_GREEN,
    PSB_RED,
    PSB_BLUE,
    PSB_PINK
	};	//


void PS2_Init(void)
{
	gpio_set_cfg(F1C100S_GPIOA0,0x01);
	gpio_set_cfg(F1C100S_GPIOE10,0x01);  //初始化端口功能
	gpio_set_cfg(F1C100S_GPIOE9,0x01);
	gpio_set_cfg(F1C100S_GPIOE8,0x01);
	gpio_set_cfg(F1C100S_GPIOE7,0x01);
	gpio_set_pull(F1C100S_GPIOE10,GPIO_PULL_DOWN); 
	gpio_set_pull(F1C100S_GPIOA0,GPIO_PULL_DOWN);
	gpio_set_pull(F1C100S_GPIOE9,GPIO_PULL_UP); 
	gpio_set_pull(F1C100S_GPIOE8,GPIO_PULL_DOWN); 
	gpio_set_pull(F1C100S_GPIOE7,GPIO_PULL_UP);   
	gpio_set_drv(F1C100S_GPIOE10,GPIO_DRV_WEAKER);
	gpio_set_drv(F1C100S_GPIOA0,GPIO_DRV_WEAKER);
	gpio_set_drv(F1C100S_GPIOE9,GPIO_DRV_WEAKER);
	gpio_set_drv(F1C100S_GPIOE8,GPIO_DRV_WEAKER);
	gpio_set_drv(F1C100S_GPIOE7,GPIO_DRV_WEAKER);
	gpio_set_direction(F1C100S_GPIOA0,GPIO_DIRECTION_INPUT);
	gpio_set_direction(F1C100S_GPIOE10,GPIO_DIRECTION_INPUT);
	gpio_set_direction(F1C100S_GPIOE9,GPIO_DIRECTION_OUTPUT);
	gpio_set_direction(F1C100S_GPIOE8,GPIO_DIRECTION_OUTPUT);
	gpio_set_direction(F1C100S_GPIOE7,GPIO_DIRECTION_OUTPUT);
	//gpio_set_rate(F1C100S_GPIOA0,GPIO_RATE_FAST);
	//gpio_set_rate(F1C100S_GPIOE10,GPIO_RATE_FAST);
	gpio_set_rate(F1C100S_GPIOE9,GPIO_RATE_FAST);
	gpio_set_rate(F1C100S_GPIOE8,GPIO_RATE_FAST);
	gpio_set_rate(F1C100S_GPIOE7,GPIO_RATE_FAST);

	gpio_set_value(F1C100S_GPIOA0,0);
	gpio_set_value(F1C100S_GPIOE10,0);
	gpio_set_value(F1C100S_GPIOE9,1);
	gpio_set_value(F1C100S_GPIOE8,0);
	gpio_set_value(F1C100S_GPIOE7,1);										  
}

//
void PS2_Cmd(u8_t CMD)
{
	volatile u16_t ref=0x01;
	Data[1] = 0;
	for(ref=0x01;ref<0x0100;ref<<=1)
	{
		if(ref&CMD)
		{
			PS2_JOYPAD_CMND_1;                   //
		}
		else PS2_JOYPAD_CMND_0;

		PS2_JOYPAD_CLOCK_1;                        //
		delay_us(5);
		PS2_JOYPAD_CLOCK_0;
		delay_us(5);
		PS2_JOYPAD_CLOCK_1;
		if(PS2_JOYPAD_DATA)
			Data[1] = ref|Data[1];
	}
	delay_us(16);
}


u8_t PS2_RedLight(void)
{
	PS2_JOYPAD_ATT_0;
	PS2_Cmd(Comd[0]);  
	PS2_Cmd(Comd[1]);  
	PS2_JOYPAD_ATT_1;
	if( Data[1] == 0X73)   return 0 ;
	else return 1;

}

void PS2_ReadData(void)
{
	volatile u8_t byte=0;
	volatile u16_t ref=0x01;

	PS2_JOYPAD_ATT_0;

	PS2_Cmd(Comd[0]);  
	PS2_Cmd(Comd[1]);  

	for(byte=2;byte<9;byte++)
	{
		for(ref=0x01;ref<0x100;ref<<=1)
		{
			PS2_JOYPAD_CLOCK_1;
			delay_us(5);
			PS2_JOYPAD_CLOCK_0;
			delay_us(5);
			PS2_JOYPAD_CLOCK_1;
		      if(PS2_JOYPAD_DATA)
		      Data[byte] = ref|Data[byte];
		}
        delay_us(16);
	}
	PS2_JOYPAD_ATT_1;	
}


u8_t PS2_DataKey()
{
	u8_t index;

	PS2_ClearData();
	PS2_ReadData();

	Handkey=(Data[4]<<8)|Data[3];     
	for(index=0;index<16;index++)
	{	    
		if((Handkey&(1<<(MASK[index]-1)))==0)
		return index+1;
	}
	return 0;         
}


u8_t PS2_AnologData(u8_t button)
{
	return Data[button];
}


void PS2_ClearData()
{
	u8_t a;
	for(a=0;a<9;a++)
		Data[a]=0x00;
}


/******************************************************
Function:    void PS2_Vibration(u8 motor1, u8 motor2)
Description: �ֱ��𶯺�����
Calls:		 void PS2_Cmd(u8 CMD);
Input: motor1:�Ҳ�С�𶯵�� 0x00�أ�������
	   motor2:�����𶯵�� 0x40~0xFF �������ֵԽ�� ��Խ��
******************************************************/
void PS2_Vibration(u8_t motor1, u8_t motor2)
{
	PS2_JOYPAD_ATT_0;
	delay_us(16);
  	PS2_Cmd(0x01);  //
	PS2_Cmd(0x42);  //
	PS2_Cmd(0X00);
	PS2_Cmd(motor1);
	PS2_Cmd(motor2);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_JOYPAD_ATT_1;
	delay_us(16);  
}
//short poll
void PS2_ShortPoll(void)
{
	PS2_JOYPAD_ATT_0;
	delay_us(16);
	PS2_Cmd(0x01);  
	PS2_Cmd(0x42);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x00);
	PS2_Cmd(0x00);
	PS2_JOYPAD_ATT_1;
	delay_us(16);	
}

void PS2_EnterConfing(void)
{
  PS2_JOYPAD_ATT_0;
	delay_us(16);
	PS2_Cmd(0x01);  
	PS2_Cmd(0x43);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x01);
	PS2_Cmd(0x00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_JOYPAD_ATT_1;
	delay_us(16);
}


void PS2_TurnOnAnalogMode(void)
{
	PS2_JOYPAD_ATT_0;
	PS2_Cmd(0x01);  
	PS2_Cmd(0x44);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x01); //analog=0x01;digital=0x00  ������÷���ģʽ
	PS2_Cmd(0xEE); //Ox03�������ã�������ͨ��������MODE������ģʽ��
				   //0xEE������������ã���ͨ��������MODE������ģʽ��
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_JOYPAD_ATT_1;
	delay_us(16);
}

void PS2_VibrationMode(void)
{
	PS2_JOYPAD_ATT_0;
	delay_us(16);
	PS2_Cmd(0x01);  
	PS2_Cmd(0x4D);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x00);
	PS2_Cmd(0X01);
	PS2_JOYPAD_ATT_1;
	delay_us(16);	
}
//
void PS2_ExitConfing(void)
{
    PS2_JOYPAD_ATT_0;
	delay_us(16);
	PS2_Cmd(0x01);  
	PS2_Cmd(0x43);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x00);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	PS2_JOYPAD_ATT_1;
	delay_us(16);
}

void PS2_SetInit(void)
{
	PS2_ShortPoll();
	PS2_ShortPoll();
	PS2_ShortPoll();
	PS2_EnterConfing();		
	PS2_TurnOnAnalogMode();	
	PS2_VibrationMode();	
	PS2_ExitConfing();	
}




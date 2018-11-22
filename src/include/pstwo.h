#ifndef __PSTWO_H
#define __PSTWO_H

#include <xboot.h>
#include <init.h>
#include <gpio/gpio.h>
#include <f1c100s-gpio.h>

#define PS2_JOYPAD_ACK        gpio_get_value(F1C100S_GPIOA0)
#define PS2_JOYPAD_DATA       gpio_get_value(F1C100S_GPIOE10)     //E10 MISO
#define PS2_JOYPAD_CMND_1     gpio_set_value(F1C100S_GPIOE8,1)      //E9 CLK
#define PS2_JOYPAD_ATT_1      gpio_set_value(F1C100S_GPIOE7,1)      //E8 MOSI
#define PS2_JOYPAD_CLOCK_1    gpio_set_value(F1C100S_GPIOE9,1)      //E7 CS

#define PS2_JOYPAD_CMND_0     gpio_set_value(F1C100S_GPIOE8,0)      //E9
#define PS2_JOYPAD_ATT_0      gpio_set_value(F1C100S_GPIOE7,0)      //E8
#define PS2_JOYPAD_CLOCK_0    gpio_set_value(F1C100S_GPIOE9,0)      //E7

#define delay_us ndelay

//These are our button constants
#define PSB_SELECT      1
#define PSB_L3          2
#define PSB_R3          3
#define PSB_START       4
#define PSB_PAD_UP      5
#define PSB_PAD_RIGHT   6
#define PSB_PAD_DOWN    7
#define PSB_PAD_LEFT    8
#define PSB_L2          9
#define PSB_R2          10
#define PSB_L1          11
#define PSB_R1          12
#define PSB_GREEN       13
#define PSB_RED         14
#define PSB_BLUE        15
#define PSB_PINK        16

#define PSB_TRIANGLE    13
#define PSB_CIRCLE      14
#define PSB_CROSS       15
#define PSB_SQUARE      16

//#define WHAMMY_BAR		8

//These are stick values
#define PSS_RX 5                //
#define PSS_RY 6
#define PSS_LX 7
#define PSS_LY 8

extern u8_t Data[9];
extern u16_t MASK[16];
extern u16_t Handkey;

void PS2_Init(void);
u8_t PS2_RedLight(void);
void PS2_ReadData(void);
void PS2_Cmd(u8_t CMD);	
u8_t PS2_DataKey(void);
u8_t PS2_AnologData(u8_t button); 
void PS2_ClearData(void);	  
void PS2_Vibration(u8_t motor1, u8_t motor2);

void PS2_EnterConfing(void);
void PS2_TurnOnAnalogMode(void);
void PS2_VibrationMode(void); 
void PS2_ExitConfing(void);
void PS2_SetInit(void);

#endif






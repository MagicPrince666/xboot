/*
 * init/main.c
 *
 * Copyright(c) 2007-2019 Jianjun Jiang <8192542@qq.com>
 * Official site: http://xboot.org
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <xboot.h>
#include <init.h>
#include <dma/dma.h>
#include <pwm/pwm.h>
#include <uart/uart.h>
#include <gpio/gpio.h>
#include <shell/shell.h>
#include <f1c100s-gpio.h>
#include "pstwo.h"
#include "usb.h"


void uart2_task(struct task_t * task, void * data)
{
	
	const char str[] = "hello uart2\r\n";

	struct uart_t * uart2 = search_uart("uart-16550.2");
 	if(uart2 == NULL){
 		printf("no device found\r\n");
 	}
 	else {
 		uart_set(uart2,115200,8,0,1);	
		printf("init device\r\n");
 		uart_write(uart2,(u8_t *)str,sizeof(str));	
 	}

	u8_t * uart1_buf = malloc(1024);
 	int recv_cnt = 0;

	while(1)
	{
		if(uart2 != NULL)
		{
			recv_cnt = uart_read(uart2,uart1_buf,1024);
			if(recv_cnt > 0)
			{
				uart1_buf[recv_cnt] = 0;
				printf("%s\r\n",uart1_buf);
			}
			uart_write(uart2,(u8_t *)str,sizeof(str));

			//printf("uart2 send buffer\r\n");
		}

		mdelay(500);
	}
}


int xboot_main(int argc, char * argv[])
{
	/* Do initial memory */
	do_init_mem();

	/* Do initial scheduler */
	do_init_sched();

	/* Do initial vfs */
	do_init_vfs();

	/* Do all initial calls */
	do_initcalls();

	/* Do show logo */
	do_showlogo();

	/* Do auto boot */
	do_autoboot();

// 	/* Do initial pwm */	
// /*
// 	struct pwm_t* pwm0 = search_pwm("pwm-f1c100s.0");

// 	pwm_config(pwm0, 500000, 1000000, 1);
// 	pwm_enable(pwm0);

// 	struct pwm_t* pwm1 = search_pwm("pwm-f1c100s.1");
	
// 	pwm_config(pwm1, 1500000, 20000000, 1);
// 	//pwm_config(pwm0, 800000, 1000000, 1);
// 	pwm_enable(pwm1);
// */

// 	gpio_set_cfg(F1C100S_GPIOE2,0x01);  //初始化端口功能
// 	gpio_set_cfg(F1C100S_GPIOE3,0x01);
// 	gpio_set_cfg(F1C100S_GPIOE4,0x01);
// 	gpio_set_cfg(F1C100S_GPIOE5,0x01);
// 	gpio_set_pull(F1C100S_GPIOE2,GPIO_PULL_NONE); 
// 	gpio_set_pull(F1C100S_GPIOE3,GPIO_PULL_NONE); 
// 	gpio_set_pull(F1C100S_GPIOE4,GPIO_PULL_NONE); 
// 	gpio_set_pull(F1C100S_GPIOE5,GPIO_PULL_NONE);   
// 	gpio_set_drv(F1C100S_GPIOE2,GPIO_DRV_WEAKER);
// 	gpio_set_drv(F1C100S_GPIOE3,GPIO_DRV_WEAKER);
// 	gpio_set_drv(F1C100S_GPIOE4,GPIO_DRV_WEAKER);
// 	gpio_set_drv(F1C100S_GPIOE5,GPIO_DRV_WEAKER);
// 	gpio_set_direction(F1C100S_GPIOE2,GPIO_DIRECTION_OUTPUT);
// 	gpio_set_direction(F1C100S_GPIOE3,GPIO_DIRECTION_OUTPUT);
// 	gpio_set_direction(F1C100S_GPIOE4,GPIO_DIRECTION_OUTPUT);
// 	gpio_set_direction(F1C100S_GPIOE5,GPIO_DIRECTION_OUTPUT);
// 	gpio_set_rate(F1C100S_GPIOE2,GPIO_RATE_FAST);
// 	gpio_set_rate(F1C100S_GPIOE3,GPIO_RATE_FAST);
// 	gpio_set_rate(F1C100S_GPIOE4,GPIO_RATE_FAST);
// 	gpio_set_rate(F1C100S_GPIOE5,GPIO_RATE_FAST);

// 	gpio_set_value(F1C100S_GPIOE2,1);
// 	gpio_set_value(F1C100S_GPIOE3,1);
// 	gpio_set_value(F1C100S_GPIOE4,1);
// 	gpio_set_value(F1C100S_GPIOE5,0);

	

 	//usb_device_init(USB_TYPE_USB_COM);

// /*
// 	u8_t key = 0;
// 	u8_t l_lx = 0,l_ly = 0,l_rx = 0,l_ry = 0;
// 	u8_t lx,ly,rx,ry;
// 	int speed = 0;
// 	PS2_Init();
// 	PS2_SetInit();
// */
// 	/* Run loop */
// 	while(1)
// 	{
// 		/* Run shell */
// 		//run_shell();
		
// 		if(uart1 != NULL)
// 		{
// 			recv_cnt = uart_read(uart1,uart1_buf,1024);
// 			if(recv_cnt > 0)
// 			{
// 				uart1_buf[recv_cnt] = 0;
// 				printf("%s\r\n",uart1_buf);
// 			}

// 			uart_write(uart1,(u8_t *)str,sizeof(str));

// 			//printf("uart2 send buffer\r\n");
// 		}
// 		/*
// 		key = PS2_DataKey();
// 		if(key != 0)               
// 		{
// 			printf("key = %d\r\n",key);
// 			if(key == 12)
// 			{
// 				PS2_Vibration(0xFF,0x00);
// 				mdelay(500);
// 			}
// 			else if(key == 11)
// 			{
// 				PS2_Vibration(0x00,0xFF); 
// 				mdelay(500);
// 			}
// 			else
// 				PS2_Vibration(0x00,0x00); 
			
// 			switch(key)
// 			{
// 				case 5:;
// 				case 7:pwm_config(pwm1, 1500000, 20000000, 1);break;
// 				case 6:pwm_config(pwm1, 2000000, 20000000, 1);break;
// 				case 8:pwm_config(pwm1, 1000000, 20000000, 1);break;
// 			}
// 		}
		
// 		lx = PS2_AnologData(PSS_LX);
// 		ly = PS2_AnologData(PSS_LY);
// 		rx = PS2_AnologData(PSS_RX);
// 		ry = PS2_AnologData(PSS_RY);

// 		if(ry < 120)
// 		{
// 			speed = (128 - ry)/128 * 1000000;
// 			gpio_set_value(F1C100S_GPIOE3,1);
// 			gpio_set_value(F1C100S_GPIOE4,0);
// 			pwm_config(pwm0, speed, 1000000, 1);
// 		}
// 		else if(ry > 135)
// 		{
// 			speed = (ry -128)/128 * 1000000;
// 			gpio_set_value(F1C100S_GPIOE3,0);
// 			gpio_set_value(F1C100S_GPIOE4,1);
// 			pwm_config(pwm0, speed, 1000000, 1);
// 		}
// 		else
// 		{
// 			gpio_set_value(F1C100S_GPIOE3,1);
// 			gpio_set_value(F1C100S_GPIOE4,1);
// 			pwm_config(pwm0, 0, 1000000, 1);
// 		}

// 		if(l_lx != lx)
// 		{
// 			l_lx = lx;
// 			printf("LX = %d\r\n",lx);
// 		}
// 		if(l_ly != ly)
// 		{
// 			l_ly = ly;
// 			printf("LY = %d\r\n",ly);	
// 		}
// 		if(l_rx != rx)
// 		{
// 			l_rx = rx;
// 			printf("RX = %d\r\n",rx);
// 		}
// 		if(l_ry != ry)
// 		{
// 			l_ry = ry;
// 			printf("RY = %d\r\n",ry);
// 		}

// 		mdelay(100);
// 		*/
// 	}

//	struct task_t * task2 = task_create(NULL, "uart2", uart2_task, NULL, 0, 0);
//	task_resume(task2);

#if	defined(CONFIG_SHELL_TASK) && (CONFIG_SHELL_TASK > 0)
	/* Create shell task */
	struct task_t * task = task_create(NULL, "shell", shell_task, NULL, 0, 0);
	
	/* Resume shell task */
	task_resume(task);

#endif



	/* Scheduler loop */
	scheduler_loop();

	//free(uart1_buf);
	/* Do all exit calls */
	do_exitcalls();

	/* Xboot return */
	return 0;
}

#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"

//ALIENTEK Mini STM32开发板范例代码11
//TFTLCD显示实验   
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司


int LCD_color[14]={WHITE,BLACK,BLUE,BRED,GRED,GBLUE,RED,MAGENTA,GREEN,CYAN,
                         YELLOW,BROWN,BRRED,GRAY
     
                        };

                        
 int main(void)
 { 
     
     
     int num;
	 u8 x=0;
	u8 lcd_id[12];			//存放LCD ID字符串	
	delay_init();	    	 //延时函数初始化	  
	uart_init(9600);	 	//串口初始化为9600
	LED_Init();		  		//初始化与LED连接的硬件接口
 	LCD_Init();
	POINT_COLOR=RED; 
	sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//将LCD ID打印到lcd_id数组。				 	
  	
     


     POINT_COLOR=RED;
     
     LCD_DrawRectangle(5,100,60,200);
     
     LCD_DrawLine(5,130,60,130);
     
     LCD_Draw_Circle(90,90,25);
     
     while(1)
     {
         delay_ms(1000);
       LCD_Fill(6,131,59,199,LCD_color[num%14]);
       num++;
     
     
     
     }   
   
     
     
     
}

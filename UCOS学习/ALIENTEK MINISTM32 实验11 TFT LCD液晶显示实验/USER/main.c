#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"

//ALIENTEK Mini STM32�����巶������11
//TFTLCD��ʾʵ��   
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾


int LCD_color[14]={WHITE,BLACK,BLUE,BRED,GRED,GBLUE,RED,MAGENTA,GREEN,CYAN,
                         YELLOW,BROWN,BRRED,GRAY
     
                        };

                        
 int main(void)
 { 
     
     
     int num;
	 u8 x=0;
	u8 lcd_id[12];			//���LCD ID�ַ���	
	delay_init();	    	 //��ʱ������ʼ��	  
	uart_init(9600);	 	//���ڳ�ʼ��Ϊ9600
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�
 	LCD_Init();
	POINT_COLOR=RED; 
	sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//��LCD ID��ӡ��lcd_id���顣				 	
  	
     


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

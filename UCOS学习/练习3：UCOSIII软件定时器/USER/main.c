#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "includes.h"
#include "key.h"

/************************************************
 ALIENTEK MiniSTM32������UCOSʵ��
 ��6-3 UCOSIII ʱ��Ƭ��ת����
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

//UCOSIII���������ȼ��û�������ʹ�ã�ALIENTEK
//����Щ���ȼ��������UCOSIII��5��ϵͳ�ڲ�����
//���ȼ�0���жϷ������������� OS_IntQTask()
//���ȼ�1��ʱ�ӽ������� OS_TickTask()
//���ȼ�2����ʱ���� OS_TmrTask()
//���ȼ�OS_CFG_PRIO_MAX-2��ͳ������ OS_StatTask()
//���ȼ�OS_CFG_PRIO_MAX-1���������� OS_IdleTask()
//����֧�֣�www.openedv.com
//�Ա����̣�http://eboard.taobao.com  
//������������ӿƼ����޹�˾  
//���ߣ�����ԭ�� @ALIENTEK

//�������ȼ�
#define START_TASK_PRIO		3
//�����ջ��С	
#define START_STK_SIZE 		128
//������ƿ�
OS_TCB StartTaskTCB;
//�����ջ	
CPU_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *p_arg);

//�������ȼ�
#define TASK1_TASK_PRIO		4
//�����ջ��С	
#define TASK1_STK_SIZE 		128
//������ƿ�
OS_TCB Task1_TaskTCB;
//�����ջ	
CPU_STK TASK1_TASK_STK[TASK1_STK_SIZE];
void task1_task(void *p_arg);



OS_TMR tmr1; //��ʱ��1
OS_TMR tmr2; //��ʱ��2

void tmr1_callback(void *p_tmr, void *p_arg);//��ʱ��1�ص�����
void tmr2_callback(void *p_tmr, void *p_arg);//��ʱ��1�ص�����


int lcd_color[14]={RED,WHITE,BLUE,BRED,GRED,GBLUE,BLACK,
                    MAGENTA,GREEN,CYAN,YELLOW,BROWN,BRRED,GRAY};



//������
int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	delay_init();  	//ʱ�ӳ�ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�жϷ�������
	uart_init(115200); 	//���ڳ�ʼ��
	
    
    INTX_DISABLE();  //���жϣ���ֹ�δ�ʱ���������ʼ��ɧ��
	LED_Init();         //LED��ʼ��	
	LCD_Init();			//LCD��ʼ��	
	KEY_Init();
    
	POINT_COLOR = RED;
	LCD_ShowString(30,10,200,16,16,"Soft Timer");	
	LCD_ShowString(30,30,200,16,16,"KEY_UP TMR1 OPEN");
	LCD_ShowString(30,50,200,16,16,"KEY0 TRM2 OPEN ");
	LCD_ShowString(30,70,200,16,16,"KEY1 ALL TMR STOP");

	
    LCD_DrawRectangle(5,100,60,200);
    LCD_DrawRectangle(80,100,135,200);
    
    LCD_DrawLine(5,120,60,120);
    LCD_DrawLine(80,120,135,120);
    
    INTX_ENABLE();//�����ж�
    
	OSInit(&err);		//��ʼ��UCOSIII
	OS_CRITICAL_ENTER();//�����ٽ���			 
	//������ʼ����
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//������ƿ�
				 (CPU_CHAR	* )"start task", 		//��������
                 (OS_TASK_PTR )start_task, 			//������
                 (void		* )0,					//���ݸ��������Ĳ���
                 (OS_PRIO	  )START_TASK_PRIO,     //�������ȼ�
                 (CPU_STK   * )&START_TASK_STK[0],	//�����ջ����ַ
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//�����ջ�����λ
                 (CPU_STK_SIZE)START_STK_SIZE,		//�����ջ��С
                 (OS_MSG_QTY  )0,					//�����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
                 (OS_TICK	  )0,					//��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
                 (void   	* )0,					//�û�����Ĵ洢��
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //����ѡ��
                 (OS_ERR 	* )&err);				//��Ÿú�������ʱ�ķ���ֵ
	OS_CRITICAL_EXIT();	//�˳��ٽ���	 
	OSStart(&err);      //����UCOSIII
}


//��ʼ������
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//ͳ������                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //��ʹ��ʱ��Ƭ��ת��ʱ��
	 //ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
	
    
    OSTmrCreate(
                   ( OS_TMR*)             &tmr1,
                   (CPU_CHAR*)            "tmr1",
                   (OS_TICK)               20,  //20*10=200MS
                   (OS_TICK)               100,  //1000
                   (OS_OPT)                OS_OPT_TMR_PERIODIC,
                   (OS_TMR_CALLBACK_PTR)   tmr1_callback,
                   (void*)                 0,
                   (OS_ERR*)               &err);
    
     OSTmrCreate(
                   ( OS_TMR*)             &tmr2,
                   (CPU_CHAR*)            "tmr2",
                   (OS_TICK)                200,
                   (OS_TICK)               0,//������period
                   (OS_OPT)                OS_OPT_TMR_ONE_SHOT,
                   (OS_TMR_CALLBACK_PTR)   tmr2_callback,
                   (void*)                 0,
                   (OS_ERR*)               &err);
    
   OS_CRITICAL_ENTER();	//�����ٽ���
	//����TASK1����
	OSTaskCreate((OS_TCB 	* )&Task1_TaskTCB,		
				 (CPU_CHAR	* )"Task1 task", 		
                 (OS_TASK_PTR )task1_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )TASK1_TASK_PRIO,     
                 (CPU_STK   * )&TASK1_TASK_STK[0],	
                 (CPU_STK_SIZE)TASK1_STK_SIZE/10,	
                 (CPU_STK_SIZE)TASK1_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  		
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
				 
	 
	OS_CRITICAL_EXIT();	//�˳��ٽ���
	OSTaskDel((OS_TCB*)0,&err);	//ɾ��start_task��������
}

void tmr1_callback(void *p_tmr, void *p_arg)//��ʱ��1�ص�����
{
   static u8 tmr1_num=0;
    
   LCD_ShowNum(8,108,tmr1_num,3,16);
  
    
   LCD_Fill(6,121,59,199,lcd_color[tmr1_num%14]);
   tmr1_num++;
    

}

void tmr2_callback(void *p_tmr, void *p_arg)//��ʱ��1�ص�����
{
   static u8 tmr2_num=0;
    
   LCD_ShowNum(83,108,tmr2_num,3,16);
  
    
   LCD_Fill(81,121,134,199,lcd_color[tmr2_num%14]);
   tmr2_num++;
    
    LED1=!LED1;
    
    printf("tmr2���н���\r\n");
    

}


//task1������
void task1_task(void *p_arg)
{
	u8 t,num;
    OS_ERR err;
    
    while(1)
    {
        t=KEY_Scan(0);
        switch(t)
        {
            case WKUP_PRES:
                OSTmrStart(&tmr1,&err);
                printf("����tmr1\r\n");
                break;
            
            case KEY0_PRES:
                 OSTmrStart(&tmr2,&err);
                printf("����tmr2\r\n");
                break;
            
            case KEY1_PRES:
                OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err);
                OSTmrStop(&tmr2,OS_OPT_TMR_NONE,0,&err);
                printf("��tmr\r\n");
                break;
            
        }
        num++;
        if(num==50)
        {    
            num=0;
            LED0=!LED0;           
        }

        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err);
    }
}




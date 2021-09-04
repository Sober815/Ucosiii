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


//�������ȼ�
#define TASK2_TASK_PRIO		5
//�����ջ��С	
#define TASK2_STK_SIZE 		128
//������ƿ�
OS_TCB Task2_TaskTCB;
//�����ջ	
CPU_STK TASK2_TASK_STK[TASK2_STK_SIZE];
void task2_task(void *p_arg);

//ͬ���ź���
OS_SEM SYN_SEM;


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
	LCD_ShowString(30,10,200,16,16,"SYN-mission ");	
	LCD_ShowString(30,30,200,16,16,"KEY_UP ");
//	LCD_ShowString(30,50,200,16,16,"KEY0 TRM2 OPEN ");
//	LCD_ShowString(30,70,200,16,16,"KEY1 ALL TMR STOP");

	
    LCD_DrawRectangle(5,100,60,200);
    //LCD_DrawRectangle(80,100,135,200);
    
    LCD_DrawLine(5,120,60,120);
    //LCD_DrawLine(80,120,135,120);
    
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
	
	OSSemCreate(
                   (OS_SEM *)       &SYN_SEM,
                   (CPU_CHAR  *)    "sem1",
                   (OS_SEM_CTR)     0,//��ֵΪ0���ź���,���������Ҫpost�ͷ����Ժ���ܵȴ��ɹ�
                   (OS_ERR * )      &err);
    

    
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
    //����TASK2����
	OSTaskCreate((OS_TCB 	* )&Task2_TaskTCB,		
				 (CPU_CHAR	* )"Task2 task", 		
                 (OS_TASK_PTR )task2_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )TASK2_TASK_PRIO,     
                 (CPU_STK   * )&TASK2_TASK_STK[0],	
                 (CPU_STK_SIZE)TASK2_STK_SIZE/10,	
                 (CPU_STK_SIZE)TASK2_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  		
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);	                 
				 
	 
	OS_CRITICAL_EXIT();	//�˳��ٽ���
	OSTaskDel((OS_TCB*)0,&err);	//ɾ��start_task��������
}



//task1������
void task1_task(void *p_arg)
{

    OS_ERR err;
    u8 t;
    
    
    while(1)
    {
        t=KEY_Scan(0);
        
        if(t==WKUP_PRES)
        {
        
        OSSemPost(&SYN_SEM,OS_OPT_POST_1,&err);
            //��ʾ�ź���ֵ
        LCD_ShowNum(8,101,SYN_SEM.Ctr,3,16);

        
        
        }        
    
        OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);  
    }
    
}

//task2������
void task2_task(void *p_arg)
{

    
     OS_ERR err;
    u8 num;
    while(1)
    {
        
        printf("����ִ������2\r\n");    
        OSSemPend(&SYN_SEM,0,OS_OPT_PEND_BLOCKING,0,&err);
        LCD_Fill(6,121,59,199,lcd_color[num%14]);
        num++;
        LED1=!LED1;
        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err);  
    }
    
}




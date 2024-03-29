#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "includes.h"
#include "key.h"

/************************************************
 ALIENTEK MiniSTM32开发板UCOS实验
 例6-3 UCOSIII 时间片轮转调度
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

//UCOSIII中以下优先级用户程序不能使用，ALIENTEK
//将这些优先级分配给了UCOSIII的5个系统内部任务
//优先级0：中断服务服务管理任务 OS_IntQTask()
//优先级1：时钟节拍任务 OS_TickTask()
//优先级2：定时任务 OS_TmrTask()
//优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
//优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()
//技术支持：www.openedv.com
//淘宝店铺：http://eboard.taobao.com  
//广州市星翼电子科技有限公司  
//作者：正点原子 @ALIENTEK

//任务优先级
#define START_TASK_PRIO		3
//任务堆栈大小	
#define START_STK_SIZE 		128
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);

//任务优先级
#define TASK1_TASK_PRIO		4
//任务堆栈大小	
#define TASK1_STK_SIZE 		128
//任务控制块
OS_TCB Task1_TaskTCB;
//任务堆栈	
CPU_STK TASK1_TASK_STK[TASK1_STK_SIZE];
void task1_task(void *p_arg);



OS_TMR tmr1; //定时器1
OS_TMR tmr2; //定时器2

void tmr1_callback(void *p_tmr, void *p_arg);//定时器1回调函数
void tmr2_callback(void *p_tmr, void *p_arg);//定时器1回调函数


int lcd_color[14]={RED,WHITE,BLUE,BRED,GRED,GBLUE,BLACK,
                    MAGENTA,GREEN,CYAN,YELLOW,BROWN,BRRED,GRAY};



//主函数
int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	delay_init();  	//时钟初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断分组配置
	uart_init(115200); 	//串口初始化
	
    
    INTX_DISABLE();  //关中断，防止滴答定时器对外设初始化骚扰
	LED_Init();         //LED初始化	
	LCD_Init();			//LCD初始化	
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
    
    INTX_ENABLE();//开启中断
    
	OSInit(&err);		//初始化UCOSIII
	OS_CRITICAL_ENTER();//进入临界区			 
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err);      //开启UCOSIII
}


//开始任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
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
                   (OS_TICK)               0,//不设置period
                   (OS_OPT)                OS_OPT_TMR_ONE_SHOT,
                   (OS_TMR_CALLBACK_PTR)   tmr2_callback,
                   (void*)                 0,
                   (OS_ERR*)               &err);
    
   OS_CRITICAL_ENTER();	//进入临界区
	//创建TASK1任务
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
				 
	 
	OS_CRITICAL_EXIT();	//退出临界区
	OSTaskDel((OS_TCB*)0,&err);	//删除start_task任务自身
}

void tmr1_callback(void *p_tmr, void *p_arg)//定时器1回调函数
{
   static u8 tmr1_num=0;
    
   LCD_ShowNum(8,108,tmr1_num,3,16);
  
    
   LCD_Fill(6,121,59,199,lcd_color[tmr1_num%14]);
   tmr1_num++;
    

}

void tmr2_callback(void *p_tmr, void *p_arg)//定时器1回调函数
{
   static u8 tmr2_num=0;
    
   LCD_ShowNum(83,108,tmr2_num,3,16);
  
    
   LCD_Fill(81,121,134,199,lcd_color[tmr2_num%14]);
   tmr2_num++;
    
    LED1=!LED1;
    
    printf("tmr2运行结束\r\n");
    

}


//task1任务函数
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
                printf("开启tmr1\r\n");
                break;
            
            case KEY0_PRES:
                 OSTmrStart(&tmr2,&err);
                printf("开启tmr2\r\n");
                break;
            
            case KEY1_PRES:
                OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err);
                OSTmrStop(&tmr2,OS_OPT_TMR_NONE,0,&err);
                printf("关tmr\r\n");
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




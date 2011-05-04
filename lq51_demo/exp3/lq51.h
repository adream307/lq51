#ifndef	_LQ51_H_
#define	_LQ51_H_

#define LQ_VERSION		3000

#define	lqTimerTick		10000		/*定时器发生中断的时间间隔*/
#define Timer0_Data	-lqTimerTick
#define	lqRamTop		0x7F		/*堆栈栈顶位置*/
#define	lqMaxID			2			/*用户任务个数，即空闲任务ID号，这个值最大为7*/
#define lqFlgMax		1			/*标志事件个数，最多只能建立16个*/
#define lqSemMax		2			/*定义信号量个数，最多只能建立16个信号量*/
#define lqMsgMax		2			/*定义邮箱个数，最多只能建立16个邮件*/

#define LQ_DELAY_EN			0		/*任务延迟*/
#define LQ_FLG_EN			1		/*标志事件*/
#define LQ_SEM_EN			0		/*信号量事件*/
#define LQ_MSG_EN			1		/*邮箱事件*/
#define LQ_TASK_TMO_CHK_EN	0		/*任务超时返回检查*/
#define LQ_CHK_MSG_EN		0		/*检查邮箱是否有消息的函数 lqIsMSgEmpty*/
		
									/*任务状态的低四位表是这个任务正在等待那个信号量或邮件*/
#define	K_TMO			0x10		/*任务在一定时间内等待某个事件发生时，置位这个标志位。如果任务在等待某个事件而这位未置位，则无限等待改事件*/
#define	K_FLG			0x20		/*任务等待某一个标志事件*/
#define K_SEM			0x40		/*任务等待信号量时置位这个标志位*/
#define K_MSG			0x80		/*任务等待邮箱时置位这个标志位*/
#define MSG_TMO			0xFF		/*邮件事件超时返回*/

#define	B_TMO			4
#define	B_FLG			5
#define	B_SEM			6
#define	B_MSG			7

#ifndef	_LQ_IN_ASM_
/*********************************
/设置中断服务子程序的入口地址
/*********************************/
extern void ISR_Timer0(void);		/*定时器0中断服务子程序,这个不能改，是系统设定的*/
extern void Timer1_ISR(void);        /*定时器1中断服务子程序*/
#ifdef _IN_LQ51_C_
unsigned int code  lqISREnter[]={
	0,						/*0号中断入口函数，外部中断0*/
	ISR_Timer0,				/*1号中断入口函数，定时器0，这个不能改，是系统设定的*/
	0,						/*2号中断入口函数，外部中断1*/
	Timer1_ISR,						/*3号中断入口函数，定时器1*/
	0,						/*4号中断入口函数，串口中断SPI*/
	0						/*5号中断入口函数，定时器2*/
};
#endif
/*********************************
/中断服务子程序入口地址设置结束
/*********************************/

extern void lqStart(void);
extern void lqSche(void);
#define lqRunningTask()	lqCrt			/*当前正在运行的任务*/

#if	LQ_DELAY_EN
extern void lqDelay(unsigned char tmo);	/*任务延迟*/
#endif

#if	LQ_FLG_EN			/*标志事件*/
extern unsigned char lqWaitFlg(unsigned char index,unsigned char tmo);
extern char lqSendFlgISR(unsigned char index);
extern void lqSendFlg(unsigned char index);
#endif

#if	LQ_SEM_EN
						/*信号量事件*/
extern unsigned char lqWaitSem(unsigned char index,unsigned char tmo);
extern char lqSendSemISR(unsigned char index);
extern void lqSendSem(unsigned char index);
#endif

#if	LQ_MSG_EN
						/*邮箱事件*/
extern unsigned char lqWaitMsg(unsigned char index,unsigned char tmo);
extern char lqIsMsgEmpty(unsigned char index);
extern char lqSendMsgISR(unsigned char index,unsigned char Msg);
extern void lqSendMsg(unsigned char index,unsigned char Msg);
#endif

#if	LQ_TASK_TMO_CHK_EN		/*任务超时检查*/
extern char lqIsTaskTmo();
#endif

#endif

#endif
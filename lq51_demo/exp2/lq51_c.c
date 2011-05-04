#define _IN_LQ51_C_
#include "lq51.h"
#include<reg52.h>
/******************************************
/			  定义全局变量
/******************************************/
unsigned char data lqTaskStack[lqMaxID+1];			/*任务堆栈指针*/
unsigned char data lqSPtemp;						/*记录ID号=当前任务ID+1的堆栈底部*/
unsigned char data lqTaskTimer[lqMaxID];			/*任务定时器*/
unsigned char data lqTaskState[lqMaxID];			/*任务状态表*/
unsigned char data lqRdyTbl;						/*就绪表*/
unsigned char data lqSwitchType;					/*任务切换类型，如果通过中断切换任务则在相应位置1，否则置0*/
unsigned char data lqIntNum;						/*进入中断服务子程序后系呕把中断号传给这个变量*/
unsigned char data lqCrt;							/*当前正在运行的任务*/

#if	LQ_FLG_EN				/*该事件的最高位表示该标志位是否置位，其他位表示等待该标志量的任务，当标志位置位时，所有等待这个标志的任务都被激活*/
unsigned char data lqFlgData[lqFlgMax]={
0,0
};
#endif

#if	LQ_SEM_EN							/*信号量数据结构*/
										/*一个信号量数据结构包含两个字节，第一个字节为信号量值*/
unsigned char data lqSemData[lqSemMax*2]={			/*第二个字节为等待这个信号量的任务*/
	0,0,
	0,0};
#endif												

#if	LQ_MSG_EN							/*消息邮箱数据结构*/
										/*一个消息邮箱数据结构包含两个字节，第一个字节为消息邮箱的消息*/
unsigned char data lqMsgData[lqMsgMax*2]={		/*第二个字节的最高位表示当前邮箱是否有消息0表示没有1表示有，其他为表示等待这个消息的任务*/
	0,0};
#endif		


const unsigned char code lqMap[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

/*****************************************
/			开始函数
/这个函数必须在0号任务的最开始的部分调用
/	void lqStart(void);
/*****************************************/
void lqStart(void)
{
	char i;
	for(i=0;i<lqMaxID;++i){
		lqTaskTimer[i] = 0;
		lqTaskState[i] = 0;
	}
	lqRdyTbl=0xFF;
	lqSwitchType=0x00;
	EA = 1;
	lqCrt=0;
}

#if LQ_DELAY_EN
/***************************************************
/				任务延迟一段时间
/	void lqDelay(unsigned char tmo)
/	tmo:延迟时间
/***************************************************/
void lqDelay(unsigned char tmo)
{
	EA=0;
	lqRdyTbl &= ~lqMap[lqCrt];
	lqTaskState[lqCrt] |= K_TMO;
	lqTaskTimer[lqCrt] = tmo;
	EA=1;
	lqSche();
}
#endif

#if	LQ_FLG_EN
/**************************************************
/			等待标志
/	char lqWaitFlg(unsigned char index,unsigned char tmo)
/*************************************************/
char lqWaitFlg(unsigned char index,unsigned char tmo)
{
	EA=0;
	lqTaskState[lqCrt] &= 0xF0;
	lqTaskState[lqCrt]|= index;		/*将当前索引值保存到任务状态的低四位，因为函数是不可重入的*/
	if(lqFlgData[index] & 0x80){	/*标志量置位*/
		lqFlgData[index] = 0;
		EA=1;
		return	K_FLG;
	}
	lqFlgData[index] |= lqMap[lqCrt];
	lqRdyTbl &= ~lqMap[lqCrt];
	lqTaskState[lqCrt] |= K_FLG;
	if(tmo){
		lqTaskState[lqCrt] |= K_TMO;
		lqTaskTimer[lqCrt] = tmo;
	}
	EA=1;
	lqSche();
	EA=0;
	index = 0x0F & lqTaskState[lqCrt];
	if(lqTaskState[lqCrt] & K_FLG){		/*等待超时返回*/
		lqTaskState[lqCrt] &= ~K_FLG;
		lqTaskState[lqCrt] |= K_TMO;	/*超时标记*/
		lqFlgData[index] &= ~lqMap[lqCrt];
		EA=1;
		return K_TMO;
	}
	EA=1;
	return K_FLG;
}

/************************************************
/		中断服务子程序发送标志
/	char lqSendFlgISR(unsigned char index)
/	返回值：
/	1 -- 有更高优先级的任务就绪，需要执行任务切换
/	0 -- 不需要执行任务切换
/************************************************/
char lqSendFlgISR(unsigned char index)
{
	char i,j;
	EA=0;
	if(lqFlgData[index] & 0x7F){		/*有任务在等待这个标志事件*/
		i=lqMaxID;
		for(j=0;j<lqMaxID;++j){
			if(lqFlgData[index] & lqMap[j]){	/*任务等待这个标志事件*/
				lqTaskState[j] &= ~K_FLG;
				lqTaskState[j] &= ~K_TMO;
				lqTaskTimer[j] = 0;
				lqRdyTbl |= lqMap[j];
				if(j<i){
					i=j;
				}
			}
		}
		lqFlgData[index] = 0;
		if(i<lqCrt){
			EA=1;
			return 1;
		}
		EA=1;
		return 0;
	}
	lqFlgData[index] = 0x80;
	EA=1;
	return 0;
}
/********************************************
/		任务发送标志事件
/	void lqSendFlg(unsigned char index)
/********************************************/
void lqSendFlg(unsigned char index)
{
	if(lqSendFlgISR(index)){
		lqSche();
	}
}
#endif

#if	LQ_SEM_EN
/************************************************
/		等待一个信号量
/	char lqWaitSem(unsigned char index,unsigned char tmo)
/	index:
/************************************************/
unsigned char lqWaitSem(unsigned char index,unsigned char tmo)
{
	EA=0;
	lqTaskState[lqCrt] &= 0xF0;
	lqTaskState[lqCrt]|= index;		/*因为函数不可重，所以存储信号量索引*/
	index *= 2;
	if(lqSemData[index]){
		--lqSemData[index];
		EA=1;
		return K_SEM;
	}
	lqRdyTbl &= ~lqMap[lqCrt];
	lqSemData[index+1] |= lqMap[lqCrt];
	lqTaskState[lqCrt] |= K_SEM;
	if(tmo){
		lqTaskState[lqCrt] |= K_TMO;
		lqTaskTimer[lqCrt] = tmo;
	}
	EA=1;
	lqSche();
	EA=0;
	index = 0x0F & lqTaskState[lqCrt];
	if(lqTaskState[lqCrt] & K_SEM){
		lqTaskState[lqCrt] &= ~K_SEM;
		lqTaskState[lqCrt] |= K_TMO;
		lqSemData[index*2+1] &= ~lqMap[lqCrt];
		EA=1;
		return K_TMO;
	}
	EA=1;
	return K_SEM;
}

/*********************************************
/			中断子程序发送信号量
/	char lqSendSemISR(unsigned char index)
/*********************************************/
char lqSendSemISR(unsigned char index)
{
	char j;
	EA=0;
	index*=2;
	if(lqSemData[index+1]){
		for(j=0;j<lqMaxID;++j){
			if(lqSemData[index+1] & lqMap[j]){
				break;
			}
		}
		lqSemData[index+1] &= ~lqMap[j];
		lqTaskTimer[j] = 0;
		lqTaskState[j] &= ~K_SEM;
		lqTaskState[j] &= ~K_TMO;
		lqRdyTbl |= lqMap[j];
		if(j<lqCrt){
			EA=1;
			return 1;
		}
		EA=1;
		return 0;
	}
	++lqSemData[index];
	EA=1;
	return 0;
}

/************************************************
/			任务发送信号量
/	void lqSendSem(unsinged char index)
/***********************************************/
void lqSendSem(unsigned char index)
{
	if(lqSendSemISR(index)){
		lqSche();
	}
}
#endif

#if	LQ_MSG_EN
/*********************************************
/			等待消息邮箱
/	unsigned char lqWaitMsg(unsigned char index,unsigned char tmo)
/	返回当前邮箱的值,如果超时，则返回0xFF
/	邮箱中的值为 0x00~0xFF
/	lqMsgData[2*index+1]的最高为0表示当前没邮件，
/	否则表示当前有邮件
/********************************************/
unsigned char lqWaitMsg(unsigned char index,unsigned char tmo)
{
	EA=0;
	lqTaskState[lqCrt] &= 0xF0;
	lqTaskState[lqCrt] |= index;
	index *= 2;
	if(lqMsgData[index+1] & 0x80){		/*当前邮箱有邮件*/
		lqMsgData[index+1] &= 0x7F;
		EA=1;
		return lqMsgData[index];
	}
	lqRdyTbl &= ~lqMap[lqCrt];
	lqMsgData[index+1] |= lqMap[lqCrt];
	lqTaskState[lqCrt] |= K_MSG;
	if(tmo){
		lqTaskState[lqCrt] |= K_TMO;
		lqTaskTimer[lqCrt] = tmo;
	}
	EA=1;
	lqSche();
	EA=0;
	index = lqTaskState[lqCrt] & 0x0F;
	index *= 2;
	if(lqTaskState[lqCrt] & K_MSG){
		lqTaskState[lqCrt] &= ~K_MSG;
		lqTaskState[lqCrt] |= K_TMO;
		lqMsgData[index] = MSG_TMO;
		lqMsgData[index+1] &= ~lqMap[lqCrt];
	}
	lqMsgData[index+1] &= 0x7F;
	EA=1;
	return lqMsgData[index];
}
/*******************************************
/		检查邮箱是否为空，
/		如果为空则返回1否则返回0
/	char lqIsMsgEmpty(unsigned char index)
/******************************************/
#if	LQ_CHK_MSG_EN
char lqIsMsgEmpty(unsigned char index)
{
	EA=0;
	if(lqMsgData[index*2+1] & 0x80){
		EA=1;
		return 0;				/*邮箱不为空，邮箱中有消息*/
	}
	EA=1;
	return 1;					/*邮箱为空，邮箱中无消息*/
}
#endif
/*******************************************
/		中断子程序发送邮件
/	如果有任务在等待这个邮件，并且发生邮件后当前任务
/	不是最高优先级的任务，那么返回1;其他情况返回0
/	如果邮箱不为空，新邮件不会覆盖原先的邮件
/	char lqSendMsgISR(unsigned char index,unsigned char Msg)
/*******************************************/
char lqSendMsgISR(unsigned char index,unsigned char Msg)
{
	EA=0;
	index *= 2;
	if(lqMsgData[index+1] & 0x80){
		EA=1;
		return 0;					/*邮箱不为空*/
	}
	lqMsgData[index] = Msg;
	++index;
	lqMsgData[index] |= 0x80;
	if(lqMsgData[index] & 0x7F){	/*有任务在等待这个邮件*/
		for(Msg=0;Msg<lqMaxID;++Msg){
			if(lqMsgData[index] & lqMap[Msg]){
				break;
			}
		}
		lqMsgData[index] &= ~lqMap[Msg];
		lqTaskState[Msg] &= ~K_MSG;
		lqRdyTbl |= lqMap[Msg];
		if(Msg < lqCrt){
			EA=1;
			return 1;
		}
	}
	EA = 1;
	return 0;
}
/********************************************
/		任务发送邮件
/	void lqSendMsg(unsigned char index,unsigned char Msg
/*******************************************/
void lqSendMsg(unsigned char index,unsigned char Msg)
{
	if(lqSendMsgISR(index,Msg)){
		lqSche();
	}
}
#endif

#if	LQ_TASK_TMO_CHK_EN
/***********************************************
/			任务超时返回检查
/	如果当前任务是超时返回，那么任务状态(lqTaskState)的B_TMO位置位,否则清零该标志位
/	一般情况下只针对邮箱事件返回后做超时检查
/	返回值：
/	1  --  当前任务是超时返回
/	0  --  当前任务不是超时返回
/	char lqIsTaskTmo()
/************************************************/
char lqIsTaskTmo()
{
	if(lqTaskState[lqCrt] & K_TMO){
		return 1;
	}
	return 0;
}
#endif
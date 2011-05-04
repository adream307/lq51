#define _IN_LQ51_C_
#include "lq51.h"
#include<reg52.h>
/******************************************
/			  ����ȫ�ֱ���
/******************************************/
unsigned char data lqTaskStack[lqMaxID+1];			/*�����ջָ��*/
unsigned char data lqSPtemp;						/*��¼ID��=��ǰ����ID+1�Ķ�ջ�ײ�*/
unsigned char data lqTaskTimer[lqMaxID];			/*����ʱ��*/
unsigned char data lqTaskState[lqMaxID];			/*����״̬��*/
unsigned char data lqRdyTbl;						/*������*/
unsigned char data lqSwitchType;					/*�����л����ͣ����ͨ���ж��л�����������Ӧλ��1��������0*/
unsigned char data lqIntNum;						/*�����жϷ����ӳ����ϵŻ���жϺŴ����������*/
unsigned char data lqCrt;							/*��ǰ�������е�����*/

#if	LQ_FLG_EN				/*���¼������λ��ʾ�ñ�־λ�Ƿ���λ������λ��ʾ�ȴ��ñ�־�������񣬵���־λ��λʱ�����еȴ������־�����񶼱�����*/
unsigned char data lqFlgData[lqFlgMax]={
0,0
};
#endif

#if	LQ_SEM_EN							/*�ź������ݽṹ*/
										/*һ���ź������ݽṹ���������ֽڣ���һ���ֽ�Ϊ�ź���ֵ*/
unsigned char data lqSemData[lqSemMax*2]={			/*�ڶ����ֽ�Ϊ�ȴ�����ź���������*/
	0,0,
	0,0};
#endif												

#if	LQ_MSG_EN							/*��Ϣ�������ݽṹ*/
										/*һ����Ϣ�������ݽṹ���������ֽڣ���һ���ֽ�Ϊ��Ϣ�������Ϣ*/
unsigned char data lqMsgData[lqMsgMax*2]={		/*�ڶ����ֽڵ����λ��ʾ��ǰ�����Ƿ�����Ϣ0��ʾû��1��ʾ�У�����Ϊ��ʾ�ȴ������Ϣ������*/
	0,0};
#endif		


const unsigned char code lqMap[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

/*****************************************
/			��ʼ����
/�������������0��������ʼ�Ĳ��ֵ���
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
/				�����ӳ�һ��ʱ��
/	void lqDelay(unsigned char tmo)
/	tmo:�ӳ�ʱ��
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
/			�ȴ���־
/	char lqWaitFlg(unsigned char index,unsigned char tmo)
/*************************************************/
char lqWaitFlg(unsigned char index,unsigned char tmo)
{
	EA=0;
	lqTaskState[lqCrt] &= 0xF0;
	lqTaskState[lqCrt]|= index;		/*����ǰ����ֵ���浽����״̬�ĵ���λ����Ϊ�����ǲ��������*/
	if(lqFlgData[index] & 0x80){	/*��־����λ*/
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
	if(lqTaskState[lqCrt] & K_FLG){		/*�ȴ���ʱ����*/
		lqTaskState[lqCrt] &= ~K_FLG;
		lqTaskState[lqCrt] |= K_TMO;	/*��ʱ���*/
		lqFlgData[index] &= ~lqMap[lqCrt];
		EA=1;
		return K_TMO;
	}
	EA=1;
	return K_FLG;
}

/************************************************
/		�жϷ����ӳ����ͱ�־
/	char lqSendFlgISR(unsigned char index)
/	����ֵ��
/	1 -- �и������ȼ��������������Ҫִ�������л�
/	0 -- ����Ҫִ�������л�
/************************************************/
char lqSendFlgISR(unsigned char index)
{
	char i,j;
	EA=0;
	if(lqFlgData[index] & 0x7F){		/*�������ڵȴ������־�¼�*/
		i=lqMaxID;
		for(j=0;j<lqMaxID;++j){
			if(lqFlgData[index] & lqMap[j]){	/*����ȴ������־�¼�*/
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
/		�����ͱ�־�¼�
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
/		�ȴ�һ���ź���
/	char lqWaitSem(unsigned char index,unsigned char tmo)
/	index:
/************************************************/
unsigned char lqWaitSem(unsigned char index,unsigned char tmo)
{
	EA=0;
	lqTaskState[lqCrt] &= 0xF0;
	lqTaskState[lqCrt]|= index;		/*��Ϊ���������أ����Դ洢�ź�������*/
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
/			�ж��ӳ������ź���
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
/			�������ź���
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
/			�ȴ���Ϣ����
/	unsigned char lqWaitMsg(unsigned char index,unsigned char tmo)
/	���ص�ǰ�����ֵ,�����ʱ���򷵻�0xFF
/	�����е�ֵΪ 0x00~0xFF
/	lqMsgData[2*index+1]�����Ϊ0��ʾ��ǰû�ʼ���
/	�����ʾ��ǰ���ʼ�
/********************************************/
unsigned char lqWaitMsg(unsigned char index,unsigned char tmo)
{
	EA=0;
	lqTaskState[lqCrt] &= 0xF0;
	lqTaskState[lqCrt] |= index;
	index *= 2;
	if(lqMsgData[index+1] & 0x80){		/*��ǰ�������ʼ�*/
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
/		��������Ƿ�Ϊ�գ�
/		���Ϊ���򷵻�1���򷵻�0
/	char lqIsMsgEmpty(unsigned char index)
/******************************************/
#if	LQ_CHK_MSG_EN
char lqIsMsgEmpty(unsigned char index)
{
	EA=0;
	if(lqMsgData[index*2+1] & 0x80){
		EA=1;
		return 0;				/*���䲻Ϊ�գ�����������Ϣ*/
	}
	EA=1;
	return 1;					/*����Ϊ�գ�����������Ϣ*/
}
#endif
/*******************************************
/		�ж��ӳ������ʼ�
/	����������ڵȴ�����ʼ������ҷ����ʼ���ǰ����
/	����������ȼ���������ô����1;�����������0
/	������䲻Ϊ�գ����ʼ����Ḳ��ԭ�ȵ��ʼ�
/	char lqSendMsgISR(unsigned char index,unsigned char Msg)
/*******************************************/
char lqSendMsgISR(unsigned char index,unsigned char Msg)
{
	EA=0;
	index *= 2;
	if(lqMsgData[index+1] & 0x80){
		EA=1;
		return 0;					/*���䲻Ϊ��*/
	}
	lqMsgData[index] = Msg;
	++index;
	lqMsgData[index] |= 0x80;
	if(lqMsgData[index] & 0x7F){	/*�������ڵȴ�����ʼ�*/
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
/		�������ʼ�
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
/			����ʱ���ؼ��
/	�����ǰ�����ǳ�ʱ���أ���ô����״̬(lqTaskState)��B_TMOλ��λ,��������ñ�־λ
/	һ�������ֻ��������¼����غ�����ʱ���
/	����ֵ��
/	1  --  ��ǰ�����ǳ�ʱ����
/	0  --  ��ǰ�����ǳ�ʱ����
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
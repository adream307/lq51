#ifndef	_LQ51_H_
#define	_LQ51_H_

#define LQ_VERSION		3000

#define	lqTimerTick		10000		/*��ʱ�������жϵ�ʱ����*/
#define Timer0_Data	-lqTimerTick
#define	lqRamTop		0x7F		/*��ջջ��λ��*/
#define	lqMaxID			2			/*�û��������������������ID�ţ����ֵ���Ϊ7*/
#define lqFlgMax		1			/*��־�¼����������ֻ�ܽ���16��*/
#define lqSemMax		2			/*�����ź������������ֻ�ܽ���16���ź���*/
#define lqMsgMax		2			/*����������������ֻ�ܽ���16���ʼ�*/

#define LQ_DELAY_EN			0		/*�����ӳ�*/
#define LQ_FLG_EN			1		/*��־�¼�*/
#define LQ_SEM_EN			0		/*�ź����¼�*/
#define LQ_MSG_EN			1		/*�����¼�*/
#define LQ_TASK_TMO_CHK_EN	0		/*����ʱ���ؼ��*/
#define LQ_CHK_MSG_EN		0		/*��������Ƿ�����Ϣ�ĺ��� lqIsMSgEmpty*/
		
									/*����״̬�ĵ���λ��������������ڵȴ��Ǹ��ź������ʼ�*/
#define	K_TMO			0x10		/*������һ��ʱ���ڵȴ�ĳ���¼�����ʱ����λ�����־λ����������ڵȴ�ĳ���¼�����λδ��λ�������޵ȴ����¼�*/
#define	K_FLG			0x20		/*����ȴ�ĳһ����־�¼�*/
#define K_SEM			0x40		/*����ȴ��ź���ʱ��λ�����־λ*/
#define K_MSG			0x80		/*����ȴ�����ʱ��λ�����־λ*/
#define MSG_TMO			0xFF		/*�ʼ��¼���ʱ����*/

#define	B_TMO			4
#define	B_FLG			5
#define	B_SEM			6
#define	B_MSG			7

#ifndef	_LQ_IN_ASM_
/*********************************
/�����жϷ����ӳ������ڵ�ַ
/*********************************/
extern void ISR_Timer0(void);		/*��ʱ��0�жϷ����ӳ���,������ܸģ���ϵͳ�趨��*/
extern void Timer1_ISR(void);        /*��ʱ��1�жϷ����ӳ���*/
#ifdef _IN_LQ51_C_
unsigned int code  lqISREnter[]={
	0,						/*0���ж���ں������ⲿ�ж�0*/
	ISR_Timer0,				/*1���ж���ں�������ʱ��0��������ܸģ���ϵͳ�趨��*/
	0,						/*2���ж���ں������ⲿ�ж�1*/
	Timer1_ISR,						/*3���ж���ں�������ʱ��1*/
	0,						/*4���ж���ں����������ж�SPI*/
	0						/*5���ж���ں�������ʱ��2*/
};
#endif
/*********************************
/�жϷ����ӳ�����ڵ�ַ���ý���
/*********************************/

extern void lqStart(void);
extern void lqSche(void);
#define lqRunningTask()	lqCrt			/*��ǰ�������е�����*/

#if	LQ_DELAY_EN
extern void lqDelay(unsigned char tmo);	/*�����ӳ�*/
#endif

#if	LQ_FLG_EN			/*��־�¼�*/
extern unsigned char lqWaitFlg(unsigned char index,unsigned char tmo);
extern char lqSendFlgISR(unsigned char index);
extern void lqSendFlg(unsigned char index);
#endif

#if	LQ_SEM_EN
						/*�ź����¼�*/
extern unsigned char lqWaitSem(unsigned char index,unsigned char tmo);
extern char lqSendSemISR(unsigned char index);
extern void lqSendSem(unsigned char index);
#endif

#if	LQ_MSG_EN
						/*�����¼�*/
extern unsigned char lqWaitMsg(unsigned char index,unsigned char tmo);
extern char lqIsMsgEmpty(unsigned char index);
extern char lqSendMsgISR(unsigned char index,unsigned char Msg);
extern void lqSendMsg(unsigned char index,unsigned char Msg);
#endif

#if	LQ_TASK_TMO_CHK_EN		/*����ʱ���*/
extern char lqIsTaskTmo();
#endif

#endif

#endif
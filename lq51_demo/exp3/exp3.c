#include "lq51.h"
#include<reg52.h>

#define FLG_REQUEST_NUM_ONE	0
#define MSG_NUM_ONE 0
#define MSG_NUM_TWO 1

void Task0() _task_ 0
{
	char sum=0;
	char n1,n2;
	
	TMOD |= 0x10;
	TH1=0x00;
	TL1=0x00;
	TR1=1;
	ET1=1;
	lqStart();
	while(1){
		lqSendFlg(FLG_REQUEST_NUM_ONE);
		n1=lqWaitMsg(MSG_NUM_ONE,100);
		n2=lqWaitMsg(MSG_NUM_TWO,0);
		sum=n1+n2;
		P1=sum;
	}
}

void Task1() _task_ 1
{
	char i=0;
	while(1){
		++i;
		lqWaitFlg(FLG_REQUEST_NUM_ONE,0);
		lqSendMsg(MSG_NUM_ONE,i);
	}
}

void Timer1_ISR() using 0
{
	TF1=0;
	lqSendMsgISR(MSG_NUM_TWO,10);
}
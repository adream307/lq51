#include"lq51.h"
#include<reg52.h>

void Task0() _task_ 0
{
	char i;
	lqStart();
	while(1){
		i=lqWaitMsg(0,0);
		P1=i;
	}
}

void Task1() _task_ 1
{
	char i=0;
	while(1){
		++i;
		lqDelay(100);
		lqSendMsg(0,i);
	}
}
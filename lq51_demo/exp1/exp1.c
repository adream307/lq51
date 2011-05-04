#include"lq51.h"
#include<reg52.h>
void Task0(void) _task_ 0
{
	char i=0;
	lqStart();
	while(1){
		++i;
		P0=i;
		lqDelay(100);
	}	
}

void Task1(void) _task_ 1
{
	char i=50;
	while(1){
		P1=i;
		++i;
		lqDelay(100);
	}
}

void Task2(void)_task_ 2
{
	char i=100;
	while(1){
		P2=i;
		++i;
		lqDelay(100);
	}
}


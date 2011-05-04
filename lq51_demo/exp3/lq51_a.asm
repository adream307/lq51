$NOMOD51 DEBUG
#define	_LQ_IN_ASM_	1
$INCLUDE (lq51.h)

EXTRN	DATA	(lqTaskStack)		;任务堆栈
EXTRN	DATA	(lqSPtemp)
EXTRN	DATA	(lqIntNum)
EXTRN	DATA	(lqCrt)
EXTRN	DATA	(lqRdyTbl)
EXTRN	DATA	(lqTaskTimer)
EXTRN	DATA	(lqTaskState)
EXTRN	DATA	(lqSwitchType)
EXTRN	CODE	(lqISREnter)
EXTRN	CODE	(lqMap)
;定义特殊功能寄存器
PCON    DATA    087H
PSW     DATA    0D0H
ACC     DATA    0E0H
B       DATA    0F0H
SP      DATA    81H
DPL     DATA    82H
DPH     DATA    83H
TCON    DATA    88H
TMOD    DATA    89H
TL0     DATA    8AH
TL1     DATA    8BH
TH0     DATA    8CH
TH1     DATA    8DH
IE      DATA    0A8H

; TCON
TF1     BIT     8FH
TR1     BIT     8EH
TF0     BIT     8DH
TR0     BIT     8CH
IE1     BIT     8BH
IT1     BIT     8AH
IE0     BIT     89H
IT0     BIT     88H
; IE 
EA      BIT     0AFH
ES      BIT     0ACH
ET1     BIT     0ABH
EX1     BIT     0AAH
ET0     BIT     0A9H
EX0     BIT     0A8H

		NAME	?LQ51_SCHE
;------------------------------------------------
;中断服务子程序入口设置
;------------------------------------------------
;------------------------------------------------
;	0号中断入口，外部中断0
;------------------------------------------------
		CSEG	AT	03H
                CLR	EA
		ORL	lqIntNum,#00H
		JMP	lqInterruptISR
;------------------------------------------------
;	1号中断入口，0号定时器
;------------------------------------------------
		CSEG	AT	0BH
                CLR	EA
		ORL	lqIntNum,#01H
		JMP	lqInterruptISR
;------------------------------------------------
;	2号中断入口，外部中断1
;------------------------------------------------
		CSEG	AT	13H
                CLR	EA
		ORL	lqIntNum,#02H
		JMP	lqInterruptISR
;------------------------------------------------
;	3号中断入口，1号定时器
;------------------------------------------------
		CSEG	AT	1BH
                CLR	EA
		ORL	lqIntNum,#03H
		JMP	lqInterruptISR
;------------------------------------------------
;	4号中断入口，串口中断SPI
;------------------------------------------------
		CSEG	AT	23H
                CLR	EA
		ORL	lqIntNum,#04H
		JMP	lqInterruptISR
;------------------------------------------------
;	5号中断入口，2号定时器
;------------------------------------------------
		CSEG	AT	2BH
                CLR	EA
		ORL	lqIntNum,#05H
		JMP	lqInterruptISR
;-------------------------------------------------
;	中断服务子程序入口设置结束
;------------------------------------------------
;------------------------------------------------
;	定时器0中断服务子程序
;------------------------------------------------
PUBLIC	ISR_Timer0
?LQ51?Timer0?S	SEGMENT CODE
		RSEG	?LQ51?Timer0?S
ISR_Timer0:
		USING	0
		CLR  	EA
		CLR	TR0
		CLR	TF0
		MOV	A,TL0
		ADD	A,#LOW(Timer0_Data + 8)
		MOV	TL0,A
		MOV	A,TH0
		ADDC	A,#HIGH(Timer0_Data + 8)
		MOV	TH0,A
		SETB	TR0
		CLR  	A
		MOV  	R7,A
?MY?C0001:
		MOV  	A,#LOW (lqTaskTimer)
		ADD  	A,R7
		MOV  	R0,A
		MOV  	A,@R0
		JZ   	?MY?C0003
		DEC  	@R0
		MOV  	A,#LOW (lqTaskTimer)
		ADD  	A,R7
		MOV  	R0,A
		MOV  	A,@R0
		JNZ  	?MY?C0003
		MOV  	A,#LOW (lqTaskState)
		ADD  	A,R7
		MOV  	R0,A
		MOV	A,#K_TMO
		CPL	A
		ANL  	A,@R0
		MOV  	@R0,A
		MOV  	A,R7
		MOV  	DPTR,#lqMap
		MOVC 	A,@A+DPTR
		ORL  	lqRdyTbl,A
?MY?C0003:
		INC  	R7
		CJNE 	R7,#lqMaxID,?MY?C0001
?MY?C0002:
		SETB 	EA
		RET

PUBLIC	lqSche
?LQ51?SCHE?S	SEGMENT CODE
		RSEG	?LQ51?SCHE?S
lqSche:
		CLR	EA
		MOV	A,#lqTaskStack
		ADD	A,lqCrt
		MOV	R0,A
		MOV	A,lqCrt
		XRL	A,#lqMaxID
		JNZ	?C012
		MOV	SP,@R0
		SJMP	?C013
?C012:		
		MOV	@R0,SP
?C013:
		MOV	DPTR,#lqMap
		MOV	R7,#0
?C015:
		MOV	A,R7
		MOVC	A,@A+DPTR
		ANL	A,lqRdyTbl
		JNZ	?C014
		INC	R7
		SJMP	?C015
?C014:
		MOV	A,lqCrt
		XRL	A,R7
		JNZ	?C024
		JMP	?C016
?C024:
		MOV	A,#lqTaskStack
		ADD	A,R7
		MOV	R0,A
		MOV	A,@R0
		MOV	R6,A
		CLR	C
		MOV	A,R7
		SUBB	A,lqCrt
		JNC	?C017
		MOV	A,SP
		CLR	C
		SUBB	A,R6
		MOV	R5,A
?C018:		
		MOV	R0,SP
		MOV	A,@R0
		MOV	R0,lqSPtemp
		MOV	@R0,A
		DEC	lqSPtemp
		DEC	SP
		DJNZ	R5,?C018
		MOV	A,lqSPtemp
		CLR	C
		SUBB	A,SP
		MOV	R5,A
		MOV	R3,lqCrt
		INC	R3
		MOV	A,R7
		MOV	R4,A
?C020:
		INC	R4
		MOV	A,R3
		XRL	A,R4
		JZ	?C016
		MOV	A,R4
		ADD	A,#lqTaskStack
		MOV	R0,A
		MOV	A,@R0
		ADD	A,R5
		MOV	@R0,A
		SJMP	?C020
		JMP	?C016
?C017:
		MOV	A,R6
		CLR	C
		SUBB	A,lqSPtemp
		MOV	R5,A
?C021:		
		INC	SP
		INC	lqSPtemp
		MOV	R0,lqSPtemp
		MOV	A,@R0
		MOV	R0,SP
		MOV	@R0,A
		DJNZ	R5,?C021
		MOV	A,lqSPtemp
		SUBB	A,SP
		MOV	R5,A
		MOV	A,R7
		MOV	R4,A
		MOV	R3,lqCrt
		INC	R4
?C022:		
		INC	R3
		MOV	A,R4
		XRL	A,R3
		JZ	?C016
		MOV	A,R3
		ADD	A,#lqTaskStack
		MOV	R0,A
		MOV	A,@R0
		CLR	C
		SUBB	A,R5
		MOV	@R0,A
		SJMP	?C022
?C016:		
		MOV	lqCrt,R7
		MOV	A,lqCrt
		XRL	A,#lqMaxID
		JZ	?C023
		MOV	DPTR,#lqMap
		MOV	A,lqCrt
		MOVC	A,@A+DPTR
		ANL	A,lqSwitchType
		JZ	?C023
		MOV	A,lqCrt
		MOVC	A,@A+DPTR
		CPL	A
		ANL	A,lqSwitchType
		MOV	lqSwitchType,A
	  	POP	B
	  	POP	DPL
	  	POP	DPH
	  	POP	PSW
	  	POP	AR7
	  	POP	AR6
	  	POP	AR5
	  	POP	AR4
	  	POP	AR3
	  	POP	AR2
	  	POP	AR1
	  	POP	AR0
		POP	ACC
?C023:
		SETB 	EA
		RET
PUBLIC	?RTX_TASKENTRY
?RTX?TASKENT?S  SEGMENT CODE
		RSEG	?RTX?TASKENT?S
?RTX_TASKENTRY:	DS	2

;------------------------------------------------
; Start lq51 Kernel
;------------------------------------------------

?RTX?CODE       SEGMENT CODE
                RSEG	?RTX?CODE
		USING	0
EXTRN CODE (?C_STARTUP)
PUBLIC	main
main:		MOV	R0,#lqTaskStack
		MOV	@R0,SP

		MOV	DPTR,#?RTX?TASKENT?S		;设置0号任务的堆栈
		MOV	A,#1
		MOVC	A,@A+DPTR
		PUSH	ACC
		CLR	A
		MOVC	A,@A+DPTR
		PUSH	ACC

		MOV	A,#lqMaxID
		MOV	R7,A				;设置循环次数
		MOV	R0,#lqRamTop			;数据段的最高地址
		MOV	A,#lqTaskStack
		ADD	A,R7
		MOV	R1,A				;R1指向任务堆栈指针表

		MOV	A,R0				;设置空闲任务的入口地址
		MOV	@R1,A
		MOV	A,#HIGH(lqIdleTask)
		MOV	@R0,A
		DEC	R0
		MOV	A,#LOW(lqIdleTask)
		MOV	@R0,A
		DEC	R0
		DEC	R1
		DEC	R7
		MOV	A,R7
		JZ	main_exit

main1:		MOV	A,R0				;设置任务的入口地址
		MOV	@R1,A
		MOV	A,R7
		ADD	A,R7
		MOVC	A,@A+DPTR
		MOV	@R0,A
		DEC	R0
		MOV	A,R7
		ADD	A,R7
		INC	A
		MOVC	A,@A+DPTR
		MOV	@R0,A
		DEC	R0
		DEC	R1
		DJNZ	R7,main1
main_exit:
		MOV	A,R0
		MOV	lqSPtemp,A
		ORL	TMOD,#01H
		MOV	TL0,#LOW (-lqTimerTick)
		MOV	TH0,#HIGH (-lqTimerTick)
		SETB	TR0
		SETB	ET0
		RET
lqIdleTask:
		ORL	PCON,#01H
		SJMP	lqIdleTask

;---------------------------------------------------
;		中断服务程序
; 当发生中断后先执行这段程序，然后转去执行用户程序
;---------------------------------------------------
lqInterruptISR:
		PUSH	ACC
		PUSH	AR0
		PUSH	AR1
		PUSH	AR2
		PUSH	AR3
		PUSH	AR4
		PUSH	AR5
		PUSH	AR6
		PUSH	AR7
		PUSH	PSW
		PUSH	DPH
		PUSH	DPL
		PUSH	B
		MOV	DPTR,#lqMap
		MOV	A,lqCrt
		MOVC	A,@A+DPTR
		ORL	lqSwitchType,A
		MOV	A,#LOW(lqISRNest)
		PUSH	ACC
		MOV	A,#HIGH(lqISRNest)
		PUSH	ACC
		MOV	DPTR,#lqISREnter
		MOV	A,#1FH
		ANL	A,lqIntNum
		RL	A
		INC	A
		MOVC	A,@A+DPTR
		PUSH	ACC
		MOV	A,#1FH
		ANL	A,lqIntNum
		RL	A
		MOVC	A,@A+DPTR
		PUSH	ACC
		ANL	lqIntNum,#0E0H
		MOV	A,lqIntNum
		ADD	A,#20H
		MOV	lqIntNum,A
		SETB	EA
		RET
;中断服务子程序嵌套，最多支持7级嵌套
lqISRNest:
		CLR	EA
		SJMP	?MY?C0005
?MY?C0006:	RETI			;中断返回
?MY?C0005:	CALL	?MY?C0006
		MOV	A,#0E0H
		ANL	A,lqIntNum
		CLR	C
		SUBB	A,#20H
		MOV	lqIntNum,A
		JNZ	?MY?C0004
		JMP	lqSche
?MY?C0004:
		POP	B
	  	POP	DPL
	  	POP	DPH
	  	POP	PSW
	  	POP	AR7
	  	POP	AR6
	  	POP	AR5
	  	POP	AR4
	  	POP	AR3
	  	POP	AR2
	  	POP	AR1
	  	POP	AR0
		POP	ACC
		SETB	EA
		RET
		END



;寄存器数据按一下顺序入栈
;PUSH	ACC
;PUSH	AR0
;PUSH	AR1
;PUSH	AR2
;PUSH	AR3
;PUSH	AR4
;PUSH	AR5
;PUSH	AR6
;PUSH	AR7
;PUSH	PSW
;PUSH	DPH
;PUSH	DPL
;PUSH	B

//###########################################################################
//
// FILE:	SciCom.c for STM32F4xx
//
// TITLE:	SCI Communication
//
//###########################################################################
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  v1.0| 12 Dec 2016 | J.Z. | Update for STM32F4xx
//  v1.1| 24 Mar 2017 | J.Z. | Modification for address in little endian
//###########################################################################

#include "stm32f4_discovery.h"
#include "aProjHead.h"
#include "Data.h"

//extern void flash_parameter(void);
//interrupt void SCI_TX_isr(void);
//interrupt void SCI_RX_isr(void);
//char TxString(char* s, int length); //function to send a string throguh SCI port
//char RxString(char* s, int length); //function to receive a string through SCI port
void CmdDecode(void); //command translation
char CmdA(char tmp);
char CmdIdle(char tmp);
char CmdOne(char tmp);
char CmdB(char tmp);
char CmdC(char tmp);
char CmdD(char tmp);
char CmdE(char tmp);
char CmdR(char tmp);
char CmdV(void);
char CmdVS(void);
char CmdW(char tmp, char level);
char CmdS(char tmp, char level);
char CmdX(char tmp, char Level);
char CmdF(char tmp, char Level);
char CmdM(char tmp, char Level);
char CmdL(char tmp, char Level); 

#define SCI_Buffer 256  //The buffer size of SCI communication
// Global variables
char TxBuf[SCI_Buffer], RxBuf[SCI_Buffer];
//WTxIndx and WRxIndx are the index for writing data into the buffer
//RTxIndx and RRxIndx are the index for reading data from the buffer
char WTxIndx=0, RTxIndx=0;	//8-bit index from 0 to 255
char WRxIndx=0, RRxIndx=0;  //8-bit index from 0 to 255
char MenuLevel=0, SCITxReady=0;  //Control of the SCI communication
char RxNum = 0, ctmp = '@';;
int *baddr, *eaddr, *addr; //beginning address, ending address and the address for read/write of a memory block
struct DataSet DataSetCom;
struct CurveData CurveCom;
struct TRDataBlk SCIDataBlk; //Global structure for SCI data block transmission and receiving
struct SLDataBlk SLData;        //Received data block of communication for Simulink
struct SLDataSent SLDataS;      //Data block to be sent

/* Configuration of UART6 */
void USART6_Configure(void)
{
  USART_InitTypeDef USART_InitStructx;
  
  /* Enable peripheral clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
  
  /* Initialization of seria port */
    /* USART_InitStruct members default value */
  USART_InitStructx.USART_BaudRate = 9600;
  USART_InitStructx.USART_WordLength = USART_WordLength_8b;
  USART_InitStructx.USART_StopBits = USART_StopBits_1;
  USART_InitStructx.USART_Parity = USART_Parity_No ;
  USART_InitStructx.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStructx.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  
  USART_Init(USART6, &USART_InitStructx);
    /* enable interrupt */
  USART_Cmd(USART6, ENABLE);
  USART_ClearFlag(USART6, USART_FLAG_TC | USART_FLAG_RXNE);
  USART_ClearITPendingBit(USART6, USART_IT_TC | USART_IT_RXNE);
  //The RXNE and TC interrupts should be enabled individually. 
  //Otherwise unexpected effect may happen.
  USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);
  USART_ITConfig(USART6, USART_IT_TC, ENABLE);
}

void USART6ISR(void)
{ /* SCI interrupt function of STM32F407 */
  //uint16_t rsartdata;
  
  if (USART_GetFlagStatus(USART6, USART_FLAG_RXNE) == 1)
  { /* An character is received */
    RxBuf[WRxIndx++] = USART_ReceiveData(USART6);
    USART_ClearITPendingBit(USART6, USART_IT_RXNE);
  }
  else if (USART_GetFlagStatus(USART6, USART_FLAG_TC) == 1)
  { /* A transmission is completed */
            
    if (WTxIndx!=RTxIndx)
    {//send the next character in a string
      //SciaRegs.SCITXBUF = TxBuf[RTxIndx++];
      USART_SendData(USART6, TxBuf[RTxIndx++]);
      USART_ClearFlag(USART6, USART_FLAG_TC);
      USART_ClearITPendingBit(USART6, USART_IT_TC);
    }
    else 
    {
      SCITxReady = 1;
      USART_ClearFlag(USART6, USART_FLAG_TC);
      USART_ClearITPendingBit(USART6, USART_IT_TC);
    }
  }
  else
  {
    while (1);
  }
}

void CmdDecode(void) //command translation
{
	char temp;
 
	temp = WRxIndx - RRxIndx;
	switch(MenuLevel)
	{
		case 0:
		MenuLevel = CmdIdle(temp);
		break;
		case 1:
		MenuLevel = CmdOne(temp);
		break;
		case 11: //read an addressed word
		MenuLevel = CmdA(temp);
		break;
		case 12: //write an addressed word
		MenuLevel = CmdB(temp);
		break;
		case 13: //read an addressed float
		MenuLevel = CmdC(temp);
		break;
		case 14: //write an addressed float
		MenuLevel = CmdD(temp);
		break;
		case 15: //System Enable/Disable
		MenuLevel = CmdE(temp);
		break;
		case 16: //System reset
		MenuLevel = CmdR(temp);
		break;
		case 17: //read a data set
		MenuLevel = CmdV();
		break;
		case 171: //prepare to send a data set
		MenuLevel = CmdVS();
		break;
		case 18: //get dynamic waveforms
		MenuLevel = CmdW(temp, MenuLevel);
		break;
		case 181: //get dynamic waveforms
		MenuLevel = CmdW(temp, MenuLevel);
		break;
		case 182: //get sampling mode
		MenuLevel = CmdW(temp, MenuLevel);
		break;
		case 183: //get sampling mode
		MenuLevel = CmdW(temp, MenuLevel);
		break;
		case 19: //Set for data set sampling
		MenuLevel = CmdS(temp, MenuLevel);
		break;
		case 191: //Set for data set sampling
		MenuLevel = CmdS(temp, MenuLevel);
		break;
		case 192: //Set for curve sampling
		MenuLevel = CmdS(temp, MenuLevel);
		break;
		case 20: //Write a struct
			MenuLevel = CmdX(temp, MenuLevel);
			break;
		case 21:	//read a data file with the given beginning address and ending address
			MenuLevel = CmdF(temp, MenuLevel);
			break;
		case 22:	//read a binary file from a memory block, write a binary file into a memory block
			MenuLevel = CmdM(temp, MenuLevel);
			break;
		case 221:	//read a binary file from a memory block, write a binary file into a memory block
			MenuLevel = CmdM(temp, MenuLevel);
			break;
		case 222:	//read a binary file from a memory block, write a binary file into a memory block
			MenuLevel = CmdM(temp, MenuLevel);
			break;
		case 223:	//read a binary file from a memory block, write a binary file into a memory block
			MenuLevel = CmdM(temp, MenuLevel);
			break;

		case 220:
			if (SCITxReady == 1)
			{
				SCITxReady = 0;
				WTxIndx = 0;
				RTxIndx = 0;
				if (SCIDataBlk.Length != 0) MenuLevel = 221;
				else MenuLevel = 0;
			}
			break;
                case  23:
			MenuLevel = CmdL(temp, MenuLevel);
                        break;
          
		case 2: //After the transmitt buffer is empty, communication returns to idle state
		if (SCITxReady == 1) 
		{
			SCITxReady = 0;
		//	WTxIndx = 0;
		//	RTxIndx = 0;
			if (NewSample == 6) MenuLevel = 181;
			else MenuLevel = 0;
		}
		break;
		default:
		MenuLevel = 0;
		break;
	}
}

char CmdIdle(char tmp)
	{
		if (tmp == 0) return 0;
		else if (RxBuf[RRxIndx++] == '@') return 1;
		else
		{
			RRxIndx = 0;
			WRxIndx = 0;	//clear buffer
			return 0;
		}
		
	}

char CmdOne(char tmp)
	{
		char cmd; //mode following the instruction
//		if (tmp >= 2)
		if (tmp != 0)
		{
			cmd = RxBuf[RRxIndx++];
//			mode = RxBuf[RRxIndx++];
			switch(cmd)
			{
				case 'A':
					return 11;
				case 'B':
					return 12;
				case 'C':
					return 13;
				case 'D':
					return 14;
				case 'E':
					return 15;
                                case 'L':
                                        return 23;
				case 'R':
					return 16;
				case 'V':
					return 17;
				case 'W':
					return 18;
				case 'S':
					return 19;
				case 'X':
					return 20;
				case 'F':
					return 21;
				case 'M':
					return 22;
				default:
					return 0;
			}
		}
		else return 1;
	}

char CmdA(char tmp)
	{
		int i, *p;
		long addr;

	//address of the memory should be transferred in 4 bytes for 32-bit in binary little endian format
		if (tmp >=4)
		{
	//		for (i=0; i<4; i++)
	//		addr = addr*256+(long) RxBuf[RRxIndx++];
                        addr = (long) RxBuf[RRxIndx++];
                        addr += (long) RxBuf[RRxIndx++]*256;
                        addr += (long) RxBuf[RRxIndx++]*65536;
                        addr += (long) RxBuf[RRxIndx++]*16777216;
                        
			p = (int*) addr;
			WTxIndx=0;
			RTxIndx=0;
			TxBuf[WTxIndx++]=(char) *p;
			TxBuf[WTxIndx++]=(char) *p>>8;
		//	SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
                        USART_SendData(USART6, ctmp);
			RRxIndx = 0;
			WRxIndx = 0;	//clear buffer
			return 2;
		}
		else return 11;
	}

char CmdB(char tmp)
	{
		int i, *p;
		long addr;
	//address of the memory should be transferred in 4 bytes for 32-bit in binary little endian format
		if (tmp >=6)
		{
		//	for (i=0; i<4; i++)
		//	addr = addr*256+(long) RxBuf[RRxIndx++];
                        addr = (long) RxBuf[RRxIndx++];
                        addr += (long) RxBuf[RRxIndx++]*256;
                        addr += (long) RxBuf[RRxIndx++]*65536;
                        addr += (long) RxBuf[RRxIndx++]*16777216;

			p = (int*) addr;
			*p = RxBuf[RRxIndx++];
			*p += RxBuf[RRxIndx++]*256;
			RRxIndx = 0;
			WRxIndx = 0;	//clear buffer
			//SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
			USART_SendData(USART6, ctmp);
                        return 2;
		}
		else return 12;
	}

char CmdC(char tmp)
	{
	//read a float data from a data memory
		int i; 
		float *p;
		long addr;
		union Float2Byte F2B;
	//address of the memory should be transferred in 4 bytes for 32-bit in binary little endian format
		if (tmp >=4)
		{
                        addr = (long) RxBuf[RRxIndx++];
                        addr += (long) RxBuf[RRxIndx++]*256;
                        addr += (long) RxBuf[RRxIndx++]*65536;
                        addr += (long) RxBuf[RRxIndx++]*16777216;
			p = (float*) addr;
			F2B.Float = *p;
			WTxIndx=0;
			RTxIndx=0;
			//data format is little endian
			TxBuf[WTxIndx++]=(char) F2B.Word.WordL;
			TxBuf[WTxIndx++]=(char) (F2B.Word.WordL>>8);
			TxBuf[WTxIndx++]=(char) F2B.Word.WordH;
			TxBuf[WTxIndx++]=(char) (F2B.Word.WordH>>8);
			//SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
			USART_SendData(USART6, ctmp);
                        RRxIndx = 0;
			WRxIndx = 0;	//clear buffer
			return 2;
		}
		else return 13;
	}

char CmdD(char tmp)
	{
	//write a float data from a data memory
		int i; 
		float *p;
		long addr;
		union Float2Byte F2B;
	//address of the memory should be transferred in 4 bytes for 32-bit in binary little endian format
		if (tmp >=8)
		{
                        addr = (long) RxBuf[RRxIndx++];
                        addr += (long) RxBuf[RRxIndx++]*256;
                        addr += (long) RxBuf[RRxIndx++]*65536;
                        addr += (long) RxBuf[RRxIndx++]*16777216;
			p = (float*) addr;
			WTxIndx=0;
			RTxIndx=0;
			//data format is little endian
			F2B.Word.WordL = RxBuf[RRxIndx++];
			F2B.Word.WordL += RxBuf[RRxIndx++]<<8;
			F2B.Word.WordH = RxBuf[RRxIndx++];
			F2B.Word.WordH += RxBuf[RRxIndx++]<<8;
			*p = F2B.Float;
			//SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
			USART_SendData(USART6, ctmp);
                        RRxIndx = 0;
			WRxIndx = 0;	//clear buffer
			return 2;
		}
		else return 14;
	}

char CmdE(char tmp)
	{
		return 0;
	}

char CmdR(char tmp)
	{
		return 0;
	}

char CmdS(char tmp, char level)
	{
	//Set for data set sampling. Data number should be less than or equal to 10.
	//Need to be upgraded for error control and recovery!
		char delay, cx;
		int i, j;
		static int cnt, bnum;
		long addr;
		if (level == 19)
		{
			if (tmp >= 2)
			{
				delay = RxBuf[RRxIndx++];
				if (delay <= 0)
				{//set for sampling a data set
					DataSetCom.DataNum = RxBuf[RRxIndx++];
					bnum = 5*DataSetCom.DataNum;
					cnt = 0;
					return 191;
				}
				else
				{//set for sampling a curve
					CurveCom.delay = delay;
					CurveCom.DataNum = RxBuf[RRxIndx++];
					bnum = 5*CurveCom.DataNum;
					cnt = 0;
					return 192;
				}
			}
			else return level;
		}
		else if (tmp >= bnum)
		{
			if (level == 191)
			{
				for (i = 0; i<DataSetCom.DataNum; i++)
				{
                                        addr = (long) RxBuf[RRxIndx++];
                                        addr += (long) RxBuf[RRxIndx++]*256;
                                        addr += (long) RxBuf[RRxIndx++]*65536;
                                        addr += (long) RxBuf[RRxIndx++]*16777216;
					DataSetCom.Addr[i]=addr;
				}
				for (i = 0; i<DataSetCom.DataNum; i++)
				{
					DataSetCom.WrdNum[i]= RxBuf[RRxIndx++];
				}
				//SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
                                USART_SendData(USART6, ctmp);
			}
			else
			{
				if (CurveCom.DataNum << 2)
				{
					for (i = 0; i<CurveCom.DataNum; i++)
					{
                                                addr = (long) RxBuf[RRxIndx++];
                                                addr += (long) RxBuf[RRxIndx++]*256;
                                                addr += (long) RxBuf[RRxIndx++]*65536;
                                                addr += (long) RxBuf[RRxIndx++]*16777216;

						CurveCom.Addr[i]=addr;
					}
					for (i = 0; i<CurveCom.DataNum; i++)
					{
						CurveCom.WrdNum[i]= RxBuf[RRxIndx++];
					}
					//SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
                                        USART_SendData(USART6, ctmp);
				}
				else
				{
					//SciaRegs.SCITXBUF = 'F'; //Start a new conversion by sending an operation mode
                                        cx = 'F';
                                        USART_SendData(USART6, cx);
				}
			}
			RRxIndx = 0;
			WRxIndx = 0;	//clear buffer
			return 2;
		}
		else
		{ //to be upgraded for restricted waiting time
			cnt++;
			return level;
		}
	}

char CmdV(void)
	{
		//start sampling a new data set
		NewSample = 1;
		RRxIndx = 0;
		WRxIndx = 0;	//clear buffer
		return 171;
	}

char CmdVS(void)
	{
		int i;
		//prepare to send a new data set
		if (NewSample == 2)
		{
			NewSample = 0;
			WTxIndx=0;
			RTxIndx=0;
			for (i=0; i<DataSetCom.ByteNum; i++) TxBuf[WTxIndx++]=ADCbuf[i];
			//SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
                        USART_SendData(USART6, ctmp);
			return 2;			
		}
		return 171;
	}

char CmdW(char tmp, char level) //tmp: number of newly SCI received characters
	{
		int i, isetval, *ip;
		float *fp;
		static char mode, form; //mode: sampling mode; form: setting data format: 0: integer, 1: float, 2: long
		static long addr;
		static int n;
		union Float2Byte fsetval;
		switch (NewSample)
		{
			case 0:
				switch (level)
				{
					case 18:
						if (tmp >=1)
						{
							mode = RxBuf[RRxIndx++];
							if (mode == 0)
							{
								NewSample = 5;
								n = 0;
								ADCCnt =0;
								CurveCom.tcnt = 0;
								WTxIndx = 0;
								RTxIndx = 0;
								RRxIndx = 0;
								WRxIndx = 0;	//clear buffer
								return 181;
							}
							else if (mode == 1) return 182;
							else return 0; //for illegal mode
						}
						else return level;
					case 181:
						return level;
					case 182:
						if (tmp >= 5)
						{
                                                        addr = (long) RxBuf[RRxIndx++];
                                                        addr += (long) RxBuf[RRxIndx++]*256;
                                                        addr += (long) RxBuf[RRxIndx++]*65536;
                                                        addr += (long) RxBuf[RRxIndx++]*16777216;
							form = RxBuf[RRxIndx++];
							return 183;
						}
						else return level;
					case 183:
						if (form == 1)
						{ //integer step setting format
							if (tmp < 2) return level;
							else
							{
								ip = (int*) addr;
								isetval = RxBuf[RRxIndx++];
								isetval += RxBuf[RRxIndx++]*256;
								*ip += isetval;
								NewSample = 5;
								n = 0;
								ADCCnt =0;
								CurveCom.tcnt = 0;
								WTxIndx = 0;
								RTxIndx = 0;
								RRxIndx = 0;
								WRxIndx = 0;	//clear buffer
								return 181;
							}
						}
						else if (form == 2)
						{ //float integer setting format
							if (tmp < 4) return level;
							else
							{
								fp = (float*) addr;
								//data format is little endian
								fsetval.Word.WordL = RxBuf[RRxIndx++];
								fsetval.Word.WordL += RxBuf[RRxIndx++]<<8;
								fsetval.Word.WordH = RxBuf[RRxIndx++];
								fsetval.Word.WordH += RxBuf[RRxIndx++]<<8;
								*fp += fsetval.Float;
								NewSample = 5;
								n = 0;
								ADCCnt =0;
								CurveCom.tcnt = 0;
								WTxIndx = 0;
								RTxIndx = 0;
								RRxIndx = 0;
								WRxIndx = 0;	//clear buffer
								return 181;
							}
						}
						else return 0;
					default:
						return 0;
				}
			case 5:
				return level;
			case 6:
				for (i = 0; i<125; i++)
				{
					TxBuf[WTxIndx++] = ADCbuf[n];
					TxBuf[WTxIndx++] = ADCbuf[n++]>>8;
				}
				//SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
                                USART_SendData(USART6, ctmp);
				if (n >= 1000) NewSample = 0;
				return 2;
			default:
				NewSample = 0;
			return 0;
		}
	}

                
char CmdX(char tmp, char Level)
	{ //This version is designed for the processor with memory in byte (compared to word for DSP)
	//Command to write a struct
	//write a float data from a data memory
		char temp=0;
		int i, bnum;
		char *p;
                long addr;
                
	//address of the memory should be transferred in 4 bytes for 32-bit in binary little endian format
		if (tmp >=6)
		{
                        addr = (long) RxBuf[RRxIndx++];
                        addr += (long) RxBuf[RRxIndx++]*256;
                        addr += (long) RxBuf[RRxIndx++]*65536;
                        addr += (long) RxBuf[RRxIndx++]*16777216;
			p = (char*) addr;
			bnum = RxBuf[RRxIndx++];	//byte equal to 2*the number of words in the struct
			temp= RxBuf[RRxIndx++];
			if (temp > 0)
			{
				temp  = WRxIndx - RRxIndx;
				while (temp < bnum) temp = WRxIndx - RRxIndx;   //This line may result in a long time delay
				for (i=0; i< bnum; i++)
				{
				//data format is little endian
					*p = RxBuf[RRxIndx++];
		//			*p += RxBuf[RRxIndx++]<<8;
					p++;
				}
				WTxIndx=0;
				RTxIndx=0;
				//SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
                                USART_SendData(USART6, ctmp);
				RRxIndx = 0;
				WRxIndx = 0;	//clear buffer
				return 2;
			}
			else
			{
				WTxIndx=0;
				RTxIndx=0;
				//data format is little endian
				for (i=0; i< bnum; i++)
				{
					TxBuf[WTxIndx++]=(char) *p++;
			//		TxBuf[WTxIndx++]=(char) *p++>>8;
				}
				//SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
                                USART_SendData(USART6, ctmp);
				RRxIndx = 0;
				WRxIndx = 0;	//clear buffer
				return 2;
			}
		}
		else return Level;
	}

          
char CmdF(char tmp, char Level)
{//test function for writing system parameters into Flash memory section FlashB
	//flash_parameter();
	return 0;
}

char CmdM(char tmp, char Level)
{//read a binary file from a memory block, write a binary file into a memory block
	int i;
	uint16_t *ip, wordnum;

	switch (Level)
	{
		case 22:
			if (tmp <7) return Level;
			else
			{
				SCIDataBlk.Mode = RxBuf[RRxIndx++];
			//	SCIDataBlk.Addr = 0;
			//	for (i = 0; i<4; i++)
			//		SCIDataBlk.Addr = SCIDataBlk.Addr*256+(long) RxBuf[RRxIndx++];
                                
                                SCIDataBlk.Addr = (long) RxBuf[RRxIndx++];
                                SCIDataBlk.Addr += (long) RxBuf[RRxIndx++]*256;
                                SCIDataBlk.Addr += (long) RxBuf[RRxIndx++]*65536;
                                SCIDataBlk.Addr += (long) RxBuf[RRxIndx++]*16777216;

				SCIDataBlk.Length = RxBuf[RRxIndx++];
				if (SCIDataBlk.Mode == 0)
				{
					WTxIndx = 0;
					RTxIndx = 0;
					RRxIndx = 0;
					WRxIndx = 0;	//clear buffer
					ip =(uint16_t *) SCIDataBlk.Addr;
					wordnum = SCI_Buffer/2;
					if (SCIDataBlk.Length <= wordnum)
					{
						wordnum = SCIDataBlk.Length;
						SCIDataBlk.Length = 0;
					}
					else SCIDataBlk.Length -= wordnum;
					for (i=0; i< wordnum; i++)
					{
						TxBuf[WTxIndx++]=(char) *ip;
						TxBuf[WTxIndx++]=(char) *ip>>8;
						ip++;
					}
					if (SCIDataBlk.Length != 0)
						SCIDataBlk.Addr += wordnum;
					//SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
                                        USART_SendData(USART6, ctmp);
					RRxIndx = 0;
					WRxIndx = 0;	//clear buffer
					return 220;
				}
				else if (SCIDataBlk.Mode == 1) return 222;
				else return 0; //for illegal mode
			}
		case 221:
			ip =(uint16_t *) SCIDataBlk.Addr;
			wordnum = SCI_Buffer/2;
			if (SCIDataBlk.Length <= wordnum)
			{
				wordnum = SCIDataBlk.Length;
				SCIDataBlk.Length = 0;
			}
			else SCIDataBlk.Length -= wordnum;
			for (i=0; i< wordnum; i++)
			{
				TxBuf[WTxIndx++]=(char) *ip;
				TxBuf[WTxIndx++]=(char) *ip>>8;
				ip++;
			}
			if (SCIDataBlk.Length != 0)
				SCIDataBlk.Addr += wordnum;
			//SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
                        USART_SendData(USART6, ctmp);
			RRxIndx = 0;
			WRxIndx = 0;	//clear buffer
			return 220;

		case 222: //For writing parameters, the number of the parameters should not more than the buffer size.
			if (SCIDataBlk.Length > SCI_Buffer/2) SCIDataBlk.Length = SCI_Buffer/2;
			return 223;
		case 223:
			if (tmp < 2*SCIDataBlk.Length) return Level;
			else
			{
				ip =(uint16_t *) SCIDataBlk.Addr;
				for (i=0; i< SCIDataBlk.Length; i++)
				{
					*ip = RxBuf[RRxIndx++];
					*ip = *ip * 256 + RxBuf[RRxIndx++];
					ip++;
				}
			}
			WTxIndx = 0;
			RTxIndx = 0;
			RRxIndx = 0;
			WRxIndx = 0;	//clear buffer
			//SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
			USART_SendData(USART6, ctmp);
                        return 2;
		default:
			return 0;
	}
}

char CmdL(char tmp, char Level)
{//Communication function for Simulink operation
	//char temp=0;
	int i, bnum;
	char *p;
                
	//address of the memory should be transferred in 4 bytes for 32-bit in binary little endian format
	if (tmp >=8)
	{
         //read the setting value
            SLData.RSetVal = (long) RxBuf[RRxIndx++];
            SLData.RSetVal += (long) RxBuf[RRxIndx++]*256;
            SLData.RSetVal += (long) RxBuf[RRxIndx++]*65536;
            SLData.RSetVal += (long) RxBuf[RRxIndx++]*16777216;
          //read the control word
            SLData.RCtrlWd = (long) RxBuf[RRxIndx++];
            SLData.RCtrlWd += (long) RxBuf[RRxIndx++]*256;
            SLData.RCtrlWd += (long) RxBuf[RRxIndx++]*65536;
            SLData.RCtrlWd += (long) RxBuf[RRxIndx++]*16777216;
            //prepare the data to be sent
            p = (char*) SLData.SAddr;
            bnum = SLData.SNum*4;
            WTxIndx=0;
            RTxIndx=0;
            //data format is little endian
            for (i=0; i< bnum; i++)
            {
		TxBuf[WTxIndx++]=(char) *p++;
		//	TxBuf[WTxIndx++]=(char) *p++>>8;
            }
            //SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
            USART_SendData(USART6, ctmp);
            RRxIndx = 0;
            WRxIndx = 0;	//clear buffer
            return 2;
        }
	else return Level;
}

void SLComInit(void)
{//Initialization of the communication for Simulink
  SLData.RSetVal = 0;
  SLDataS.data1 = 0.234;
  SLDataS.data2 = 0;
  SLDataS.data3 = 3.14156;
  SLDataS.data4 = 203.452;
  SLData.SAddr = (int32_t) &SLDataS;
  SLData.SNum = 4;
}

void SLCom(void)
{
  SLDataS.data2 = (float) SLData.RSetVal / 16777216;
}
//===========================================================================
// End of SourceCode.
//===========================================================================

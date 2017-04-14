//###########################################################################
//
// FILE:	A_SciCom.c for TMS320F2812
//
// TITLE:	SCI Communication
//
//###########################################################################
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  v5.0| 29July2013 | J.Z. | Program reconstruction
//  v6.0| 7 Aug 2013  | J.Z. | Active rectifier
//  v6.1| 25 June2014 | J.Z. | Develop for LED testbed
//  v6.2| 4 July 2014 | J.Z. | First general-purpose SCI communication
//  v6.3| 24 Aug 2014 | J.Z. | Update the function for step response test
//###########################################################################


#include "SciCom.h"

extern void flash_parameter(void);
interrupt void SCI_TX_isr(void);
interrupt void SCI_RX_isr(void);
//char TxString(char* s, int length); //function to send a string throguh SCI port
//char RxString(char* s, int length); //function to receive a string through SCI port
void CmdDecode(); //command translation
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

#define SCI_Buffer 256  //The buffer size of SCI communication
// Global variables
char TxBuf[SCI_Buffer], RxBuf[SCI_Buffer];
//WTxIndx and WRxIndx are the index for writing data into the buffer
//RTxIndx and RRxIndx are the index for reading data from the buffer
char WTxIndx=0, RTxIndx=0;	//8-bit index from 0 to 255
char WRxIndx=0, RRxIndx=0;  //8-bit index from 0 to 255
char MenuLevel=0, SCITxReady=0;  //Control of the SCI communication
char RxNum = 0;
struct DataSet DataSetCom;
struct CurveData CurveCom;
int *baddr, *eaddr, *addr; //beginning address, ending address and the address for read/write of a memory block
struct TRDataBlk SCIDataBlk; //Global structure for SCI data block transmission and receiving

interrupt void SCI_TX_isr(void)
	{
	//SCI_TX interrupt is only generated when TXRDY changes from 0 to 1.
	//It is not necessary to disable TX or disable TX interrupt in order to avoid
	//continuous coming interrupts from TXRDY!
        if (WTxIndx!=RTxIndx)
		{
			//send the next character in a string
			SciaRegs.SCITXBUF = TxBuf[RTxIndx++];
		}
		else SCITxReady = 1;
	    PieCtrlRegs.PIEACK.all = 0x0100;
	}

interrupt void SCI_RX_isr(void)
{
	RxBuf[WRxIndx++]= SciaRegs.SCIRXBUF.all;
	PieCtrlRegs.PIEACK.all = 0x0100;   // Acknowledge interrupt to PIE
}

void CmdDecode() //command translation
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
		case 2: //After the transmitt buffer is empty, communication returns to idle state
		if (SCITxReady == 1) 
		{
			SCITxReady = 0;
			WTxIndx = 0;
			RTxIndx = 0;
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
		long addr=0;
	//address of the memory should be transferred in 4 bytes for 32-bit in binary little endian format
		if (tmp >=4)
		{
			for (i=0; i<4; i++)
			addr = addr*256+(long) RxBuf[RRxIndx++];
			p = (int*) addr;
			WTxIndx=0;
			RTxIndx=0;
			TxBuf[WTxIndx++]=(char) *p;
			TxBuf[WTxIndx++]=(char) *p>>8;
			SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
			RRxIndx = 0;
			WRxIndx = 0;	//clear buffer
			return 2;
		}
		else return 11;
	}

char CmdB(char tmp)
	{
		int i, *p;
		long addr=0;
	//address of the memory should be transferred in 4 bytes for 32-bit in binary little endian format
		if (tmp >=6)
		{
			for (i=0; i<4; i++)
			addr = addr*256+(long) RxBuf[RRxIndx++];
			p = (int*) addr;
			*p = RxBuf[RRxIndx++];
			*p += RxBuf[RRxIndx++]*256;
			RRxIndx = 0;
			WRxIndx = 0;	//clear buffer
			SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
			return 2;
		}
		else return 12;
	}

char CmdC(char tmp)
	{
	//read a float data from a data memory
		int i; 
		float *p;
		long addr=0;
		union Float2Byte F2B;
	//address of the memory should be transferred in 4 bytes for 32-bit in binary little endian format
		if (tmp >=4)
		{
			for (i=0; i<4; i++)
			addr = addr*256+(long) RxBuf[RRxIndx++];
			p = (float*) addr;
			F2B.Float = *p;
			WTxIndx=0;
			RTxIndx=0;
			//data format is little endian
			TxBuf[WTxIndx++]=(char) F2B.Word.WordL;
			TxBuf[WTxIndx++]=(char) F2B.Word.WordL>>8;
			TxBuf[WTxIndx++]=(char) F2B.Word.WordH;
			TxBuf[WTxIndx++]=(char) F2B.Word.WordH>>8;
			SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
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
		long addr=0;
		union Float2Byte F2B;
	//address of the memory should be transferred in 4 bytes for 32-bit in binary little endian format
		if (tmp >=8)
		{
			for (i=0; i<4; i++)
			addr = addr*256+(long) RxBuf[RRxIndx++];
			p = (float*) addr;
			WTxIndx=0;
			RTxIndx=0;
			//data format is little endian
			F2B.Word.WordL = RxBuf[RRxIndx++];
			F2B.Word.WordL += RxBuf[RRxIndx++]<<8;
			F2B.Word.WordH = RxBuf[RRxIndx++];
			F2B.Word.WordH += RxBuf[RRxIndx++]<<8;
			*p = F2B.Float;
			SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
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
		char delay;
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
					addr = 0;
					for (j = 0; j<4; j++)
					addr = addr*256+(long) RxBuf[RRxIndx++];
					DataSetCom.Addr[i]=addr;
				}
				for (i = 0; i<DataSetCom.DataNum; i++)
				{
					DataSetCom.WrdNum[i]= RxBuf[RRxIndx++];
				}
				SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
			}
			else
			{
				if (CurveCom.DataNum << 2)
				{
					for (i = 0; i<CurveCom.DataNum; i++)
					{
						addr = 0;
						for (j = 0; j<4; j++)
						addr = addr*256+(long) RxBuf[RRxIndx++];
						CurveCom.Addr[i]=addr;
					}
					for (i = 0; i<CurveCom.DataNum; i++)
					{
						CurveCom.WrdNum[i]= RxBuf[RRxIndx++];
					}
					SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
				}
				else
				{
					SciaRegs.SCITXBUF = 'F'; //Start a new conversion by sending an operation mode
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
			SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
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
							addr = 0;
							for (i = 0; i<4; i++)
							addr = addr*256+(long) RxBuf[RRxIndx++];
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
				SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
				if (n >= 1000) NewSample = 0;
				return 2;
			default:
				NewSample = 0;
			return 0;
		}
	}

char CmdX(char tmp, char Level)
	{
	//Command to write a struct
	//write a float data from a data memory
		char temp=0;
		int i, wnum, bnum;
		int *p;
		long addr=0;
	//address of the memory should be transferred in 4 bytes for 32-bit in binary little endian format
		if (tmp >=6)
		{
			for (i=0; i<4; i++)
			addr = addr*256+(long) RxBuf[RRxIndx++];
			p = (int*) addr;
			wnum = RxBuf[RRxIndx++];	//byte equal to 2*the number of words in the struct
			bnum = 2*wnum;
			temp= RxBuf[RRxIndx++];
			if (temp > 0)
			{
				temp  = WRxIndx - RRxIndx;
				while (temp < bnum) temp = WRxIndx - RRxIndx;
				for (i=0; i< wnum; i++)
				{
				//data format is little endian
					*p = RxBuf[RRxIndx++];
					*p += RxBuf[RRxIndx++]<<8;
					p++;
				}
				WTxIndx=0;
				RTxIndx=0;
				SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
				RRxIndx = 0;
				WRxIndx = 0;	//clear buffer
				return 2;
			}
			else
			{
				WTxIndx=0;
				RTxIndx=0;
				//data format is little endian
				for (i=0; i< wnum; i++)
				{
					TxBuf[WTxIndx++]=(char) *p;
					TxBuf[WTxIndx++]=(char) *p++>>8;
				}
				SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
				RRxIndx = 0;
				WRxIndx = 0;	//clear buffer
				return 2;
			}
		}
		else return Level;
	}

char CmdF(char tmp, char Level)
{//test function for writing system parameters into Flash memory section FlashB
	flash_parameter();
	return 0;
}

char CmdM(char tmp, char Level)
{
	int i;
	Uint16 *ip, wordnum;

	switch (Level)
	{
		case 22:
			if (tmp <7) return Level;
			else
			{
				SCIDataBlk.Mode = RxBuf[RRxIndx++];
				SCIDataBlk.Addr = 0;
				for (i = 0; i<4; i++)
					SCIDataBlk.Addr = SCIDataBlk.Addr*256+(long) RxBuf[RRxIndx++];
				SCIDataBlk.Length = RxBuf[RRxIndx++];
				if (SCIDataBlk.Mode == 0)
				{
					WTxIndx = 0;
					RTxIndx = 0;
					RRxIndx = 0;
					WRxIndx = 0;	//clear buffer
					ip =(Uint16 *) SCIDataBlk.Addr;
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
					SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
					RRxIndx = 0;
					WRxIndx = 0;	//clear buffer
					return 220;
				}
				else if (SCIDataBlk.Mode == 1) return 222;
				else return 0; //for illegal mode
			}
		case 221:
			ip =(Uint16 *) SCIDataBlk.Addr;
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
			SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
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
				ip =(Uint16 *) SCIDataBlk.Addr;
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
			SciaRegs.SCITXBUF = '@'; //Start a new conversion by sending an operation mode
			return 2;
		default:
			return 0;
	}
}
//===========================================================================
// End of SourceCode.
//===========================================================================

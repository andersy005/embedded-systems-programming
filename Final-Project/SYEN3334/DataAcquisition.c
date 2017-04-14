//########################################################################################
//
// FILE:	Data.c for STM32F4xx
//
// TITLE:	Data sample and system control
//
//########################################################################################
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|============================================================
//  v1.0| Dec.12,2016 | J.Z. | version developed from a_control.c
//########################################################################################

#include "DSP281x_Device.h"
#include "A_Const.h"
#include "A_SciCom.h"
#include "A_PWMGen.h"
#include "A_SmartLoadTest.h"
#include "Lookup_table.h"
#include "math.h"

#define BatTestFun1s 0
#define BatTestFun2s 1
//#include "ECan_Com.h"

// Prototype statements for functions found within this file.
//interrupt void cpu_timer0_isr(void);	//CPU timer for control period setting
interrupt void adc_isr(void);
void DataSample(void);
void SysCtrlIni(void);
#if EnT1PINT
interrupt void T1PINT_isr(void);  //Monitoring the sampling instances (zj, 6/24/2015)
#endif
void CtrlFun(void);	//Periodic control function
void TrapeInteger(_iq *var, _iq u); //Discrete integrator based on bilinear (trapezoidal) approximation
void SOGIFun(void);	//SOGI function for SOGI based PLL
#if SOGI_PLL2
void SOGIFun2(void);	//2nd SOGI function for SOGI based PLL for test
#endif
void IQ_PI_REG(struct PIC_IQparam CtrlParam, struct PIC_IQstate *p); //PI_controller in IQ format
_iq CurSetConv(float ISet);	//Convert the float-point setting to iq format
_iq RectifierCtrl(void);
//void PWM_Update(_iq iqDutyCtrl); //Asymmetric control of switching power poles
//void PWM_Update2(_iq iqDutyCtrl); //Symmetric control of switching power poles
_iq FdFrdVoltCmp(void);		//Evaluate the feed forward voltage compensation
//_iq FdFrdVoltCmp2(void);		//Evaluate the feed forward voltage compensation2
void DisableActiveRectifier(void);
_iq NotchFilter(_iq u);	//Notch filter
void LnFrqEva(void);
//Battery test function
void BatTestFun1(void);
void BatTestFun2(void);
void EnablePhaseC(void);
void EnablePhaseC2(void);
void DisablePhaseC(void);
void DisRecEnaPhaseC2(void);
void AdcData(void);
void AdcCal(void);
void CurIcCtrl(void);
void CurIcCtrl2(void);
void fBatTestInit(void);
void fVacMonitor(void);				//Monitor the ac input line voltage and process the minimum voltage error


unsigned int Voltage_A;
unsigned int Voltage_B;

//System parameter definition
#ifdef __cplusplus
#pragma DATA_SECTION("sysparamfile")
struct PIC_IQparam PLL_CtrlParam;
#pragma DATA_SECTION("sysparamfile")
struct PIC_IQparam iqIcCurCtrlParam;
#pragma DATA_SECTION("sysparamfile")
struct PIC_IQparam iqVdcCtrlParam;
#pragma DATA_SECTION("sysparamfile")
struct PIC_IQparam iqIacCtrlParam;
#pragma DATA_SECTION("sysparamfile")
struct PIC_IQparam iqBatVdcCtrlParam;
#pragma DATA_SECTION("sysparamfile")
int IaOffset;	//The offset of AD converted value for phase-a current.
#pragma DATA_SECTION("sysparamfile")
int IbOffset;	//The offset of AD converted value for phase-b current.
#pragma DATA_SECTION("sysparamfile")
int IcOffset;	//The offset of AD converted value for phase-a current.
#pragma DATA_SECTION("sysparamfile")
int VdcOffset;  //The offset of AD converted value for dc-link voltage
#pragma DATA_SECTION("sysparamfile")
int IdcOffset;  //The offset of AD converted value for dc-link current
#pragma DATA_SECTION("sysparamfile")
int CvtTmpOffset;  //The feedback gain of converter temperture
#pragma DATA_SECTION("sysparamfile")
float IaFbGain;	//The feedback gain of AD converted value for phase-a current.
#pragma DATA_SECTION("sysparamfile")
float IbFbGain;	//The feedback gain of AD converted value for phase-b current.
#pragma DATA_SECTION("sysparamfile")
float IcFbGain;	//The feedback gain of AD converted value for phase-a current.
#pragma DATA_SECTION("sysparamfile")
float VdcFbGain;  //The feedback gain of AD converted value for dc-link voltage
#pragma DATA_SECTION("sysparamfile")
float IdcFbGain;  //The feedback gain of AD converted value for dc-link voltage
#pragma DATA_SECTION("sysparamfile")
float CvtTmpFbGain;  //The feedback gain of converter temperture
#pragma DATA_SECTION("sysparamfile")
float ADSetGain;  //The gain of ADC setting
#pragma DATA_SECTION("sysparamfile")
_iq iqVacFbGain;  //The gain of Vac detection
#pragma DATA_SECTION("sysparamfile")
int iqVacOffset;	//The offset of Vac detecting circuit
#pragma DATA_SECTION("sysparamfile")
_iq iqKsogi;	//Gain of SOGI PLL
#pragma DATA_SECTION("sysparamfile")
_iq iqWfreqHalfPrd;	//Parameter for SOGI
#pragma DATA_SECTION("sysparamfile")
_iq iqMaxDCVoltage; //Maximum DC link voltage
#pragma DATA_SECTION("sysparamfile")
_iq iqVdcSet; //Set DC link voltage
#pragma DATA_SECTION("sysparamfile")
_iq iqMinDCVoltage; //Minimum DC link voltage
#pragma DATA_SECTION("sysparamfile")
_iq iqMaxACCurrent; //Maximum AC current
#pragma DATA_SECTION("sysparamfile")
_iq iqKWoL; //Maximum AC current
#pragma DATA_SECTION("sysparamfile")
_iq iqKVac2Vdc; //Maximum AC current
#pragma DATA_SECTION("sysparamfile")
_iq iqNotchCoef[5]; //Notch coefficients
#pragma DATA_SECTION("sysparamfile")
struct OpCycle BatTestCfg; //Data structure for battery test
#pragma DATA_SECTION("sysparamfile")
_iq iqCFreqDev; //Critical frequency deviation
#pragma DATA_SECTION("sysparamfile")
_iq iqFullChgBatOCV; //Fully charged battery OCV
#pragma DATA_SECTION("sysparamfile")
_iq iqFullDisChgBatOCV; //Fully discharged battery OCV
#pragma DATA_SECTION("sysparamfile")
_iq iqFullChgBatVt; //Fully charged battery terminal voltage
#pragma DATA_SECTION("sysparamfile")
_iq iqFullDisChgBatVt; //Fully discharged battery terminal voltage
#pragma DATA_SECTION("sysparamfile")
struct TestFun3Var sSmartLoadCfg;
#else
#pragma DATA_SECTION(PLL_CtrlParam,"sysparamfile")
struct PIC_IQparam PLL_CtrlParam;
#pragma DATA_SECTION(iqIcCurCtrlParam,"sysparamfile")
struct PIC_IQparam iqIcCurCtrlParam;
#pragma DATA_SECTION(iqVdcCtrlParam,"sysparamfile")
struct PIC_IQparam iqVdcCtrlParam;
#pragma DATA_SECTION(iqIacCtrlParam,"sysparamfile")
struct PIC_IQparam iqIacCtrlParam;
#pragma DATA_SECTION(iqBatVdcCtrlParam,"sysparamfile")
struct PIC_IQparam iqBatVdcCtrlParam;
#pragma DATA_SECTION(IaOffset,"sysparamfile")
int IaOffset;	//The offset of AD converted value for phase-a current.
#pragma DATA_SECTION(IbOffset,"sysparamfile")
int IbOffset;	//The offset of AD converted value for phase-b current.
#pragma DATA_SECTION(IcOffset,"sysparamfile")
int IcOffset;	//The offset of AD converted value for phase-a current.
#pragma DATA_SECTION(VdcOffset,"sysparamfile")
int VdcOffset;  //The offset of AD converted value for dc-link voltage
#pragma DATA_SECTION(IdcOffset,"sysparamfile")
int IdcOffset;  //The offset of AD converted value for dc-link current
#pragma DATA_SECTION(CvtTmpOffset,"sysparamfile")
int CvtTmpOffset;  //The feedback gain of converter temperture
#pragma DATA_SECTION(IaFbGain,"sysparamfile")
float IaFbGain;	//The feedback gain of AD converted value for phase-a current.
#pragma DATA_SECTION(IbFbGain,"sysparamfile")
float IbFbGain;	//The feedback gain of AD converted value for phase-b current.
#pragma DATA_SECTION(IcFbGain,"sysparamfile")
float IcFbGain;	//The feedback gain of AD converted value for phase-a current.
#pragma DATA_SECTION(VdcFbGain,"sysparamfile")
float VdcFbGain;  //The feedback gain of AD converted value for dc-link voltage
#pragma DATA_SECTION(IdcFbGain,"sysparamfile")
float IdcFbGain;  //The feedback gain of AD converted value for dc-link voltage
#pragma DATA_SECTION(CvtTmpFbGain,"sysparamfile")
float CvtTmpFbGain;  //The feedback gain of converter temperture
#pragma DATA_SECTION(ADSetGain,"sysparamfile")
float ADSetGain;  //The gain of ADC setting
#pragma DATA_SECTION(iqVacFbGain,"sysparamfile")
_iq iqVacFbGain;  //The feedback gain of Vac detection
#pragma DATA_SECTION(iqVacOffset,"sysparamfile")
int iqVacOffset;	//The offset of Vac detecting circuit
#pragma DATA_SECTION(iqKsogi,"sysparamfile")
_iq iqKsogi;	//Gain of SOGI PLL
#pragma DATA_SECTION(iqWfreqHalfPrd,"sysparamfile")
_iq iqWfreqHalfPrd; //Parameter for SOGI
#pragma DATA_SECTION(iqMaxDCVoltage,"sysparamfile")
_iq iqMaxDCVoltage; //Maximum DC link voltage
#pragma DATA_SECTION(iqVdcSet,"sysparamfile")
_iq iqVdcSet; //Set DC link voltage
#pragma DATA_SECTION(iqMinDCVoltage,"sysparamfile")
_iq iqMinDCVoltage; //Minimum DC link voltage
#pragma DATA_SECTION(iqMaxACCurrent,"sysparamfile")
_iq iqMaxACCurrent; //Maximum AC current
#pragma DATA_SECTION(iqKWoL,"sysparamfile")
_iq iqKWoL; //Maximum AC current
#pragma DATA_SECTION(iqKVac2Vdc,"sysparamfile")
_iq iqKVac2Vdc; //Maximum AC current
#pragma DATA_SECTION(iqNotchCoef,"sysparamfile")
_iq iqNotchCoef[5]; //Notch Coefficients
#pragma DATA_SECTION(BatTestCfg, "sysparamfile")
struct OpCycle BatTestCfg; //Data structure for battery test
#pragma DATA_SECTION(iqCFreqDev,"sysparamfile")
_iq iqCFreqDev; //Critical frequency deviation
#pragma DATA_SECTION(iqFullChgBatOCV,"sysparamfile")
_iq iqFullChgBatOCV; //Fully charged battery OCV
#pragma DATA_SECTION(iqFullDisChgBatOCV,"sysparamfile")
_iq iqFullDisChgBatOCV; //Fully discharged battery OCV
#pragma DATA_SECTION(iqFullChgBatVt,"sysparamfile")
_iq iqFullChgBatVt; //Fully charged battery terminal voltage
#pragma DATA_SECTION(iqFullDisChgBatVt,"sysparamfile")
_iq iqFullDisChgBatVt; //Fully discharged battery terminal voltage
#pragma DATA_SECTION(sSmartLoadCfg,"sysparamfile")
struct TestFun3Var sSmartLoadCfg;
#endif

int TCnt=0, Delay = 0; //Delay in unit of 100 us
int ADCCnt = 0, ADCbuf[1024];
int IABuf, IBBuf, ICBuf, IDCBuf, VDCBuf, TmpBuf;
int ADCa5, ADCb5, ADCa6, ADCb6, ADCa7, ADCb7;
float IA, IB, IC, IDC, VdcSet, CvtTmp;
float IcSet;
int T1PINT_Cnt = 0;

//Variable for SOGI
_iq iqIA, iqIB, iqIC;
_iq iqIcSet, iqIacSet, iqIm, iqIac; //iqIm is the ac current magnitude generated by the dc-link voltage controller
_iq iqVDC, iqVAC, iqVacSet;   //iqVAC: the sampled AC line voltage
_iq iqRecDuty;
_iq iqTheta, iqPhShift;		//iqPhShift is the variable to set the current lag phase angle compared to input line voltage.
_iq iqValpha, iqVbeta, iqVacEval, iqVq, iqVd; //iqVacEval: evaluated magnitude of the ac line voltage.
_iq iqWfreq; //Angular frequency in per unit with the base value equal to 60*2*pi
_iq iqIntg1State[2], iqIntg2State[2]; //two buffers for two integers in SOGI
_iq iqIntg3State[2];	//One buffer for integer in PLL
//Variable for PR controller
_iq iqPrCtrlState[4]; //States of PR current controller
_iq16 iq16PwmPrd; //PwmPeriod in IQ16 format
_iq iqLdVdc;	//iqLdVdc is the per-unit voltage of DC load voltage


//Variable for voltage feedforward compensation
_iq iqDelta, iqVacv;	//iqDelta: angle voltage phasors across the inductor; iqVacv: the voltage for the compensation
//_iq iqDelta2, iqVacv2;	//iqDelta2: angle voltage phasors across the inductor; iqVacv2: the voltage for the compensation
//Notch filter for processing Vdc
_iq iqNotch[4], iqVdcFd;


int  OpCount = 0, EndCount = 0;

char NewSample = 0; 
int nTime = 0;

struct PIC_IQstate iqVdcCtrlState;
struct PIC_IQstate PLLCtrlState;
struct PIC_IQstate iqIcCurCtrlState;
struct PIC_IQstate iqBatVdcCtrlState;
union CtrlWord SysCtrlWord;
union StateWord BatteryState;

//Variable for battery test
long TimeCnt;
int  MeanCnt;
float LowIc, HighIc;
_iq iqMeanIc, iqMeanVc, iqSamples[4];
//Additional variables for battery test function 2

struct TestFun2Var BatTestVar;

struct Samples1 iqSamplesS;


//Variable for average line frequency evaluation
int LFSampleCnt = 0;
_iq iqAveLFreq, iqMeanLF[8];

//Variables for system calibration
int SysCalcnt = 0, ParmOffset=0;
long CalSum1, CalSum2, CalSum3;

//Global time counting variables
unsigned long iGTimeCnt;		//global time counter in 0.1 ms (dependent on the control period), the maximum time counting is 2.4855 days, or 59.65 hours
unsigned long iGTimeIns1, iGTimeIns2; //initial and final time instance for an event.

interrupt void adc_isr(void)
{
	static int i=0;
#if EnT1PINT
	GpioDataRegs.GPBDAT.bit.GPIOB0=0; //clear GPIOB0 to monitoring the sampling instances (zj, 6/24/2015)
#else
	GpioDataRegs.GPBTOGGLE.bit.GPIOB0=1; //toggle GPIOB0 to monitoring the sampling instances (zj, 6/24/2015)
#endif

	/*
	AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 2; //ADCINA/B2: Power converter current Ia and Ib
	AdcRegs.ADCCHSELSEQ1.bit.CONV01 = 3; //ADCINA/B3: Power converter current Ic and temperature
	AdcRegs.ADCCHSELSEQ1.bit.CONV02 = 4; //ADCINA/B4: DC link current and voltage
	 */
 	EALLOW;                     
     SysCtrlRegs.WDKEY = 0x55; 
     //SysCtrlRegs.WDKEY = 0xAA; 
 	EDIS;

	i++;
	if (i >=SwFrq2SampFrq )
	{
		i=0;
		Voltage_A = AdcRegs.ADCRESULT0 >> 4;
		Voltage_B = AdcRegs.ADCRESULT1 >> 4;
		IABuf = Voltage_A;
		IBBuf = Voltage_B;
		//AINA3/B3
		Voltage_A = AdcRegs.ADCRESULT2 >> 4;
		Voltage_B = AdcRegs.ADCRESULT3 >> 4;
		ICBuf = Voltage_A;
		TmpBuf = Voltage_B;
		//AINA4/B4
		Voltage_A = AdcRegs.ADCRESULT4 >> 4;
		Voltage_B = AdcRegs.ADCRESULT5 >> 4;
		IDCBuf = Voltage_A;
		VDCBuf = Voltage_B;
		//AINA5/B5
		Voltage_A = AdcRegs.ADCRESULT6 >> 4;
		Voltage_B = AdcRegs.ADCRESULT7 >> 4;
		ADCa5 = Voltage_A;
		ADCb5 = Voltage_B;
		//AINA6/B6
		Voltage_A = AdcRegs.ADCRESULT8 >> 4;
		Voltage_B = AdcRegs.ADCRESULT9 >> 4;
		ADCa6 = Voltage_A;
		ADCb6 = Voltage_B;
		//AINA6/B6
		Voltage_A = AdcRegs.ADCRESULT10 >> 4;
		Voltage_B = AdcRegs.ADCRESULT11 >> 4;
		ADCa7 = Voltage_A;
		ADCb7 = Voltage_B;
		CtrlFun();
	}

	AdcRegs.ADCTRL2.bit.RST_SEQ1 = 1;
	AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

#if EnT1PINT
interrupt void T1PINT_isr(void)
{
	GpioDataRegs.GPBDAT.bit.GPIOB0=1; //Set GPIOB0 to monitoring the sampling instances (zj, 6/24/2015)
	T1PINT_Cnt++;
	EvaRegs.EVAIFRA.bit.T1PINT = 1; //Reset the interrupt flag. It is necessary to clear the flag for successive interrupt responses.
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;
}
#endif

void SysCtrlIni(void)
{
	int i, iTemp;
	//Initialization of the system control
	SysCtrlWord.all = 0;
	//PI Controller initialization
	IcSet = 0;

	//Initialize the global time variables
	iGTimeCnt = 0;
	iGTimeIns1 = 0;
	iGTimeIns2 = 0;

	PLLCtrlState.ei = 0;
	PLLCtrlState.eip1 = 0;
	PLLCtrlState.ui = _IQ(1.0);  //Initial output of the PLL controller is set as unit per unit

#if SOGI_PLL2
//PLL for the 2nd test SOGI PLL
	PLLCtrlState2.ei = 0;
	PLLCtrlState2.eip1 = 0;
	PLLCtrlState2.ui = _IQ(1.0);  //Initial output of the PLL controller is set as unit per unit
#endif

	iqIcCurCtrlState.ei = 0;
	iqIcCurCtrlState.eip1 = 0;
	iqIcCurCtrlState.ui = 0;

	iqVdcCtrlState.ei = 0;
	iqVdcCtrlState.eip1 = 0;
	iqVdcCtrlState.ui = 0;

	iqBatVdcCtrlState.ei = 0;
	iqBatVdcCtrlState.eip1 = 0;
	iqBatVdcCtrlState.ui = 0;

	for(i=0; i<4; i++) iqPrCtrlState[i] = 0;
	VdcSet = 48;		//VdcSet should be Set directly with iq varaible
	iqRecDuty = 0;


	for (i=0; i<4; i++) iqNotch[i] = 0;
	for (i=0; i<8; i++) iqMeanLF[i] = 0;

	iqPhShift = 0;		//Phase lag of current related to AC voltage
	//read the phase output to be activated
	BatteryState.all = 0;
	//Initialize the SysCtrlWord based on the setting of dip switching setting GPIOB8:14
	iTemp = GpioDataRegs.GPBDAT.all >>8;
	iTemp &= 0x007F;
	SysCtrlWord.all |= iTemp;
	iTemp &= 0x0038;	//Keep the test mode information in iTemp Bit3:5
//	if (GpioDataRegs.GPBDAT.bit.GPIOB14 == 1) SysCtrlWord.bit.PwmTest = 1;
//	if (GpioDataRegs.GPBDAT.bit.GPIOB8 == 1) SysCtrlWord.bit.PhaseAEn = 1;
//	if (GpioDataRegs.GPBDAT.bit.GPIOB9 == 1) SysCtrlWord.bit.PhaseBEn = 1;
//	SysCtrlWord.bit.SysCal = 1; //System initialization after system power on
	if (SysCtrlWord.bit.TestMode == 1) fPWMGenInit(); //PWM test mode
	else if (SysCtrlWord.bit.TestMode == 2) fBatTestInit();
	else if (SysCtrlWord.bit.TestMode == 3) fSmartLoadInit(); //Initialization of smart load test
	else if (SysCtrlWord.bit.PhaseCEn == 1)
	{
		iqIcSet = 0;
		EnablePhaseC();		//enable PhaseC control
	}
}

void IQ_PI_REG(struct PIC_IQparam CtrlParam, struct PIC_IQstate *p)
{
	//IQ point proportional-integral controller in incremental algorithm
	_iq tmp;
	tmp = p->eip1 - p->ei;
	p->ui += _IQmpy(CtrlParam.Kp, tmp) + _IQmpy(CtrlParam.Ki, p->eip1);
//	CtrlState.ui += CtrlParam.Kp*(CtrlState.eip1 - CtrlState.ei)+CtrlParam.Ki*CtrlState.eip1;
	p->ei = p->eip1;
//	CtrlState.ei = CtrlState.eip1;
	if (p->ui > CtrlParam.LMax) p->ui = CtrlParam.LMax;
	else if (p->ui < CtrlParam.LMin) p->ui = CtrlParam.LMin;
}

void CtrlFun(void)	//Periodic control function
{
	_iq iqtmp;
//	static float Tmp1ave=0, Tmp2ave=0, Pwrave=0;
//	int Errnp1, temp;

	GpioDataRegs.GPBDAT.bit.GPIOB7=1; //Test
//	PieCtrlRegs.PIEACK.all=PIEACK_GROUP1; //Interrupt acknowledge

//	CpuTimer0.InterruptCount++;

	fVacMonitor();				//Monitor the ac input line voltage and process the minimum voltage error

//	PieCtrlRegs.PIEACK.all=PIEACK_GROUP1; //Interrupt acknowledge
	//System power enable and disable
	//Button S1 (GPIOD1) for enable: pressing the button -> read as zero.
	//Button S2 (GPIOD6) for disable: pressing the button -> read as zero.
	if (SysCtrlWord.bit.SysError == 0)
	{
		if (SysCtrlWord.bit.PEnable == 0)
		{
			if (GpioDataRegs.GPDDAT.bit.GPIOD1 == 0)
			{//if an enable input arrived, enable the power stage and set it in enabled mode (OpMode = 1).
				SysCtrlWord.bit.PEnable = 1;
				if (SysCtrlWord.bit.TestMode != 1) //If not in the PWM test mode
				{
					if (SysCtrlWord.bit.PhaseCEn == 1 || BatteryState.bit.BatTestState == 1) EnablePhaseC2();
				}
				GpioDataRegs.GPADAT.bit.GPIOA11=0; //Enable the power circuit
				GpioDataRegs.GPADAT.bit.GPIOA12=1; //Close the relay
				//EnableActiveRectifier();
				SysCtrlWord.bit.OpMode = 1;		//enable power stage
			}
			else if (SysCtrlWord.bit.OpMode ==1)
			{//if a command to set PEnable = 0 while system power stage is still in enabled mode, then set the system power stage as disabled mode.
				SysCtrlWord.bit.OpMode = 0;
				SysCtrlWord.bit.SysCal = 0;
				GpioDataRegs.GPADAT.bit.GPIOA11=1; //Diable the power circuit
				GpioDataRegs.GPADAT.bit.GPIOA12=0; //Open the relay
				if (SysCtrlWord.bit.TestMode != 1) DisableActiveRectifier();  //If not in the PWM test mode
			}
		}
		else if (GpioDataRegs.GPDDAT.bit.GPIOD6 == 0)
		{//if a disable input arrives, disable the power stage and set disabled-mode
			SysCtrlWord.bit.PEnable = 0;
			GpioDataRegs.GPADAT.bit.GPIOA11=1; //Dissable the power circuit
			GpioDataRegs.GPADAT.bit.GPIOA12=0; //Open the relay
			if (SysCtrlWord.bit.TestMode != 1) DisableActiveRectifier(); //If not in the PWM test mode
			SysCtrlWord.bit.OpMode = 0; //disable the power stage
			SysCtrlWord.bit.SysCal = 0;
		}
		else if (SysCtrlWord.bit.OpMode == 0)
		{//If PEnabe = 1 while Power stage is in diabled mode, enable the power stage and set the enabled-mode
			SysCtrlWord.bit.OpMode = 1;
			if (SysCtrlWord.bit.TestMode != 1) //If not in the PWM test mode
			{
				if (SysCtrlWord.bit.PhaseCEn == 1 || BatteryState.bit.BatTestState == 1) EnablePhaseC2();
			}
			GpioDataRegs.GPADAT.bit.GPIOA11=0; //Enable the power circuit
			GpioDataRegs.GPADAT.bit.GPIOA12=1; //Close the relay
			//EnableActiveRectifier();
		}
	}

	AdcData();	//Processing the ADC values
	if (SysCtrlWord.bit.TestMode != 1)  //If not in the PWM test mode
	{
		if (SysCtrlWord.bit.PEnable == 0) 	AdcCal();  //If the system is in disable mode, execute ADC calibration.
	}

//System error checking
	//Check if there is overvoltage in DC-link circuit
	if (iqVDC > iqMaxDCVoltage)
	{
		//DC-link over voltage error, disable the power stage, disable the system and set the system error
		SysCtrlWord.bit.PEnable = 0;
		GpioDataRegs.GPADAT.bit.GPIOA11=1; //Dissable the power circuit
		DisableActiveRectifier();
		SysCtrlWord.bit.OpMode = 0; //disable the power stage
		SysCtrlWord.bit.SysError = 1;
		SysCtrlWord.bit.OverVdc = 1;
	}
	iqtmp = _IQabs(iqIB);
	if (iqtmp> iqMaxACCurrent)
	{
		//Input over current error, disable the power stage, disable the system and set the system error
		SysCtrlWord.bit.PEnable = 0;
		GpioDataRegs.GPADAT.bit.GPIOA11=1; //Dissable the power circuit
		DisableActiveRectifier();
		SysCtrlWord.bit.OpMode = 0; //disable the power stage
		SysCtrlWord.bit.SysError = 1;
		SysCtrlWord.bit.OverCurrent = 1;
	}
//	IcSet = (float)ADCa6 * ADSetGain;
	if (SysCtrlWord.bit.TestMode == 1)
	{	//PWM Test Mode
		if (SysCtrlWord.bit.PEnable == 1)
		{
			GpioDataRegs.GPBDAT.bit.GPIOB1=1; //Set GPIOB0 to monitoring the sampling instances (zj, 6/24/2015)
			if (SysCtrlWord.bit.ActivRecEn == 1) fPWMGen(2);  //three-phase AC inverter in sine PWM
			else if (SysCtrlWord.bit.BatSuplEn == 1) fPWMGen(3); //three-phase AC inverter in SV PWM
			else fPWMGen(1);	//single phase AC inverter
			GpioDataRegs.GPBDAT.bit.GPIOB1=0; //Set GPIOB0 to monitoring the sampling instances (zj, 6/24/2015)
		}
	}
	else
	{	//System operation mode
		GpioDataRegs.GPBDAT.bit.GPIOB1=1; //Set GPIOB0 to monitoring the sampling instances (zj, 6/24/2015)
		SOGIFun();
	#if SOGI_PLL2
		SOGIFun2();		//Function test for the SOGI-PLL with different AC voltage input
	#endif
		GpioDataRegs.GPBDAT.bit.GPIOB1=0; //Set GPIOB0 to monitoring the sampling instances (zj, 6/24/2015)

		iqVdcFd = NotchFilter(iqVDC);		//Notch filter to process the dc-link voltage signal for the feedback control

		if (SysCtrlWord.bit.PEnable == 1)
		{
			if (SysCtrlWord.bit.TestMode == 3) SmartLoadCtrl();  //Smart load control

			if (SysCtrlWord.bit.ActivRecEn == 1)
			{
			//Control of active rectifier
			//	iqPICOut = RectifierCtrl();
			//	PWM_Update(iqPICOut);
				iqRecDuty = RectifierCtrl();
				PWM_Update2(iqRecDuty);
			}

			else if (SysCtrlWord.bit.BatSuplEn == 1)
			{//Battery supply dc-link voltage controller
				iqBatVdcCtrlState.eip1 = iqVdcSet - iqVDC;	//Not using notch filter for the voltage feedback
				IQ_PI_REG(iqBatVdcCtrlParam, &iqBatVdcCtrlState);
				iqIcSet = - iqBatVdcCtrlState.ui;
			}
			if (SysCtrlWord.bit.TestMode == 2) BatTestFun2();  //In the battery test operation, call BatTestFun2();
			if (SysCtrlWord.bit.PhaseCEn == 1 || BatteryState.bit.BatTestState == 1)
			{
			//Ht current control
				CurIcCtrl();	//Current control without voltage feed forward compensation
			}
			else if (iqIcCurCtrlState.ui > 0)
			{
				iqIcCurCtrlState.ui = 0;
				EvaRegs.CMPR3= 0;
			}
		}
		LnFrqEva();		//Evaluate the averrage line frequency in per unit
	}
	if (SysCtrlWord.bit.TestMode == 3) BatteryStateEval();	//Evaluate the battery state
	DataSample();
	iqSamplesS.sSysState = SysCtrlWord.all;		//Save the system control state for test
	iqSamplesS.sBatteryState = BatteryState.all;	//Save the battery state for test
	iGTimeCnt++;		//increment global time counter
	GpioDataRegs.GPBDAT.bit.GPIOB7=0; //test
}

/*
interrupt void cpu_timer0_isr(void)	//CPU timer for control period setting
{
//Interrupt ISR for CPU Timer0 is not used anymore for the control because it can not be syncrhonized with the data sampling of ADC.
//TMS320F28xx: the CPU can process only one interrupt ISR. Before the end of one ISR, no any other interrupt can be responsed.
//It is because there is only one backup register for status register which can be used during one interrupt response.
}
*/

void CurIcCtrl(void)
{
	_iq iqPICOut;	//PICOut in iq format
	_iq16 iq16tmp1, iq16tmp2;
	int PwmDuty;	//for the test, it should be a local variable
	//Ht current control
		//Currentt control without voltage feed forward compensation
		iqIcCurCtrlState.eip1 = iqIcSet - iqIC;
		IQ_PI_REG(iqIcCurCtrlParam, &iqIcCurCtrlState);
		iqPICOut = iqIcCurCtrlState.ui;
		iq16tmp1 = _IQtoIQ16(iqPICOut);
		iq16tmp2 = _IQ16mpy(iq16tmp1, iq16PwmPrd);
		PwmDuty = _IQ16int(iq16tmp2);
		if (PwmDuty <= MNDuty) PwmDuty = 0;
		EvaRegs.CMPR3= PwmDuty;	//Update the duty cycle
}

void CurIcCtrl2(void)
{
	_iq iqPICOut, iqtmp;	//PICOut in iq format
	_iq16 iq16tmp1, iq16tmp2;
	int PwmDuty;	//for the test, it should be a local variable
	//Ht current control
	//Current controller with voltage feed forward compensation.
	//The function is suitable for the load with a battery.
	//The detected voltage of the battery is used for the feed forward compensation of the current controller.
		iqIcCurCtrlState.eip1 = iqIcSet - iqIC;
		IQ_PI_REG(iqIcCurCtrlParam, &iqIcCurCtrlState);
		iqPICOut = iqIcCurCtrlState.ui;
		iqtmp = _IQmpy(iqLdVdc, vBaseVoltRatio1);	//Convert the per-unit DC load voltage from load voltage base to dc-link voltage base.
		iqPICOut += _IQdiv(iqtmp, iqVDC);	//add the voltage compensation to the duty-cycle output
		if (iqPICOut > viqMXDuty2) iqPICOut = viqMXDuty2;
		else if (iqPICOut < 0) iqPICOut = 0;	//Limit the duty cycle between 0 to 0.96 in IQ24 format
		iq16tmp1 = _IQtoIQ16(iqPICOut);
		iq16tmp2 = _IQ16mpy(iq16tmp1, iq16PwmPrd);
		PwmDuty = _IQ16int(iq16tmp2);
		if (PwmDuty <= MNDuty) PwmDuty = 0;
		EvaRegs.CMPR3= PwmDuty;	//Update the duty cycle
}

void AdcData(void)
{//Evaluation of variables sampled with ADC
	//evaluate phase A current

//	IA = (float)(IaOffset - IABuf) / IaFbGain;
	iqIA = (long)(IaOffset - IABuf)*vGainPCurrent;
	//evaluate phase B current

//	IB = (float)(IbOffset - IBBuf) / IbFbGain;
	iqIB = (long)(IbOffset - IBBuf)*vGainPCurrent;
	//evaluate phase C current

//	IC = (float)(IcOffset - ICBuf) / IcFbGain;
	iqIC = (long)(IcOffset - ICBuf)*vGainPCurrent;
	//evaluate dc-link current

//	IDC = (float)(IDCBuf - IdcOffset) / IdcFbGain;
	//evaluate dc-link voltage

//	VDC = (float)(VDCBuf - VdcOffset) / VdcFbGain;
	iqVDC = (long)(VDCBuf - VdcOffset)*vGainDCVoltage;

	//Evaluate the current setting

	//Evaluate detected Vac
//	iqVAC = (long)(ADCa5v-ADCb5-iqVacOffset) *vGainACVoltage;
	iqVAC = (long)(ADCa5-iqVacOffset) *vGainACVoltage;

	//Evaluate detected Vdc of dc load
//	iqLdVdc = (long)(ADCa6-ADCb6) *vGainDCVoltage;
	if (ADCb6 == 0) iqLdVdc = 0;
	else
		{
			iqLdVdc = vBatVolOffset;
			iqLdVdc += (long)(ADCb6)*vGainDCVoltage
		}
	//	iqLdVdc = (long)(ADCb6)*vGainDCVoltage + vBatVolOffset;

	//Evaluate the temperature
	//CvtTmp = (float)(ADCa7 - CvtTmpOffset)/CvtTmpFbGain;
	//ADCa7/b7: battery dc current
//	iqIC2 = (long)(ADCa7 - ADCb7 - IcOffset2)*vGainPCurrent2; This is a battery current detected with the external current detecting circuit. It is not used now.
}

void AdcCal(void)
{ //Calibration of ADC conversion
	//int SysCalcnt = 0, ParmOffset=0;
	//long CalSum1;
	int Offset;
	if (SysCtrlWord.bit.SysCal == 0)
	{
		//Initialization of the system calibration
		SysCalcnt = 0;
		CalSum1 = 0;
		CalSum2 = 0;
		CalSum3 = 0;
		SysCtrlWord.bit.SysCal = 1;
	}
	else if (SysCalcnt < SampleNum)
	{
		CalSum1 += IBBuf;
		CalSum2 += ICBuf;
//		CalSum3 += ADCa5-ADCb5;
		CalSum3 += ADCa5;
		SysCalcnt++;
	}
	else
	{
		Offset = CalSum1/SampleNum;
		CalSum1 = 0;
		IbOffset = Offset;
		Offset = CalSum2/SampleNum;
		CalSum2 = 0;
		IcOffset = Offset;
		Offset = CalSum3/SampleNum;
		CalSum3 = 0;
		iqVacOffset = Offset;
		SysCalcnt = 0;
	}
}


void DataSample(void)
{
	int i, j, m, *p;
	if (NewSample == 1)
	{
		//Data saved in little-endian format.
		m = 0;
		for (i=0; i<DataSetCom.DataNum; i++)
		{
			p = (int*) DataSetCom.Addr[i];
			for (j=0; j<DataSetCom.WrdNum[i]; j++)
			{
				ADCbuf[m++] = (char) *p;
				ADCbuf[m++] = (char) *p>>8;
				p++;
			}
		}
		DataSetCom.ByteNum = m;	//Set the number of sampled bytes.
		NewSample = 2;
	}
	else if (NewSample == 5)
	{
		if (CurveCom.tcnt++ >= CurveCom.delay)
		{
			CurveCom.tcnt = 0;
			for (i=0; i<CurveCom.DataNum; i++)
			{
				p = (int*) CurveCom.Addr[i];
				for (j=0; j<CurveCom.WrdNum[i]; j++)
				{
					ADCbuf[ADCCnt++] = *p;
					p++;
				}
			}

			if (ADCCnt >= 1000) NewSample = 6;
		}
	}
}

//Discrete integrator based on bilinear (trapezoida) approximation. time consumption for computation about 9.2 micro seconds
void TrapeInteger(_iq *var, _iq u)
{
	_iq tmp;
	tmp = _IQmpy(iqWfreqHalfPrd, u);
	*var = *(var+1)+tmp;
	*(var+1) = *var+tmp;
	return;
}

//Variable for SOGI
//float iqValpha, iqVbeta, iqWfreq;
//float iqIntg1State[2], iqIntg2State[2];
//int  OpCount = 0, EndCount = 0;
//float Uref=0;

void SOGIFun(void)
{
	_iq tmp1, tmp2;
	if (OpCount == 0)
	{
		iqValpha = 0;
		iqVbeta = 0;
		iqTheta = 0;
		iqIntg1State[0] = iqValpha;
		iqIntg1State[1] = iqValpha;
		iqIntg2State[0] = iqVbeta;
		iqIntg2State[1] = iqVbeta;
		iqIntg3State[0] = iqTheta;
		iqIntg3State[0] = iqTheta;
		OpCount++;
	}
//	else if (OpCount <= EndCount)
//	{
//	tmp1 = iqWfreq*(iqKsogi*(VAC-iqValpha)-iqVbeta);
	tmp2 = iqVAC-iqValpha;
	tmp1 = _IQmpy(iqKsogi, tmp2);
	tmp2 = tmp1 - iqVbeta;
	tmp1 = _IQmpy(tmp2, iqWfreq);
	TrapeInteger(iqIntg1State, tmp1);
	iqValpha = iqIntg1State[0];
	tmp1 = _IQmpy(iqWfreq, iqValpha);
	TrapeInteger(iqIntg2State, tmp1);
	iqVbeta = iqIntg2State[0];
	iqVacEval = _IQmag(iqValpha, iqVbeta); //iqVacEval = sqrt(iqValpha^2+iqVbeta^2); the command takes about 107 cycles
	//Park's transformation
	tmp1 = _IQsin(iqTheta);
	tmp2 = _IQcos(iqTheta);
	iqVq = _IQmpy(iqVbeta, tmp2) - _IQmpy(iqValpha, tmp1);
	iqVd = _IQmpy(iqValpha, tmp2) + _IQmpy(iqVbeta, tmp1);
	PLLCtrlState.eip1 = 0 - iqVq;
	IQ_PI_REG(PLL_CtrlParam, &PLLCtrlState);
	iqWfreq = PLLCtrlState.ui;
	TrapeInteger(iqIntg3State, PLLCtrlState.ui);
	iqTheta = iqIntg3State[0];
	if (iqTheta > vIQTwoPi)
	{
		iqTheta -= vIQTwoPi;
		iqIntg3State[0] -= vIQTwoPi;
		iqIntg3State[1] -= vIQTwoPi;
	}
	else if (iqTheta < 0)
	{
		iqTheta += vIQTwoPi;
		iqIntg3State[0] += vIQTwoPi;
		iqIntg3State[1] += vIQTwoPi;
	}

//	OpCount++;
//	}
	return;
}

#if SOGI_PLL2
void SOGIFun2(void)
{
	_iq tmp1, tmp2;
	if (OpCount2 == 0)
	{
		Valpha2 = 0;
		Vbeta2 = 0;
		Theta2 = 0;
		Intg1State2[0] = Valpha2;
		Intg1State2[1] = Valpha2;
		Intg2State2[0] = Vbeta2;
		Intg2State2[1] = Vbeta2;
		Intg3State2[0] = Theta2;
		Intg3State2[0] = Theta2;
		OpCount2++;
	}
	tmp2 = iqVAC2-Valpha2;
	tmp1 = _IQmpy(iqKsogi, tmp2);
	tmp2 = tmp1 - Vbeta2;
	tmp1 = _IQmpy(tmp2, Wfreq2);
	TrapeInteger(Intg1State2, tmp1);
	Valpha2 = Intg1State2[0];
	tmp1 = _IQmpy(Wfreq2, Valpha2);
	TrapeInteger(Intg2State2, tmp1);
	Vbeta2 = Intg2State2[0];
	VacEval2 = _IQmag(Valpha2, Vbeta2); //sqrt() will take about 34 microseconds. We may realize it in main function instead of interrupts.
	//Park's transformation
	tmp1 = _IQsin(Theta2);
	tmp2 = _IQcos(Theta2);
	Vq2 = _IQmpy(Vbeta2, tmp2) - _IQmpy(Valpha2, tmp1);
	Vd2 = _IQmpy(Valpha2, tmp2) + _IQmpy(Vbeta2, tmp1);
	PLLCtrlState2.eip1 = 0 - Vq2;
	IQ_PI_REG(PLL_CtrlParam, &PLLCtrlState2);
	Wfreq2 = PLLCtrlState2.ui;
	TrapeInteger(Intg3State2, PLLCtrlState2.ui);
	Theta2 = Intg3State2[0];
	if (Theta2 > vIQTwoPi)
	{
		Theta2 -= vIQTwoPi;
		Intg3State2[0] -= vIQTwoPi;
		Intg3State2[1] -= vIQTwoPi;
	}
	else if (Theta2 < 0)
	{
		Theta2 += vIQTwoPi;
		Intg3State2[0] += vIQTwoPi;
		Intg3State2[1] += vIQTwoPi;
	}
	return;
}
#endif

_iq RectifierCtrl(void)
	{
		_iq tmp1, tmp2, tmp3;
		_iq iqPwmDuty;
	//Voltage controller
//		iqVdcCtrlState.eip1 = iqVdcSet - iqVDC;
		iqVdcCtrlState.eip1 = iqVdcSet - iqVdcFd;	//Using notch filter for the voltage feedback
		IQ_PI_REG(iqVdcCtrlParam, &iqVdcCtrlState);
		iqIm = iqVdcCtrlState.ui;
		tmp1 = -_IQcos(iqTheta - iqPhShift);
		iqIacSet = _IQmpy(iqIm, tmp1);	//Find the ac current setting value
		//Find the ac current
		iqIac = iqIB;
	//Current PR controller
		tmp1 = iqIac - iqIacSet;	//Find the error of the current setting, note the positive current corresponding to the power flow from line to dc-link.
		tmp2 = _IQmpy(tmp1, iqIacCtrlParam.Kp);	//Proportional output
		tmp3 = _IQmpy(tmp1, iqIacCtrlParam.Ki);
		tmp3 -=iqPrCtrlState[2];
		//First integeral
		tmp1 = _IQmpy(iqWfreq, tmp3);
		tmp3 = _IQmpy(iqWfreqHalfPrd, tmp1);
		iqPrCtrlState[0] = iqPrCtrlState[1] + tmp3;
		iqPrCtrlState[1] = iqPrCtrlState[0] + tmp3;
		//Second integral
		tmp1 = _IQmpy(iqWfreq, iqPrCtrlState[0]);
		tmp3 = _IQmpy(iqWfreqHalfPrd, tmp1);
		iqPrCtrlState[2] = iqPrCtrlState[3] + tmp3;
		iqPrCtrlState[3] = iqPrCtrlState[2] + tmp3;
		tmp1 = iqPrCtrlState[0];	//Integral output
	//Limit the output
		tmp3 = tmp1 + tmp2;
	//Voltage compensation
#if VoltComp
		iqVacv = FdFrdVoltCmp();
		tmp3 += iqVacv;
#endif

		if (tmp3 > iqIacCtrlParam.LMax) tmp3 = iqIacCtrlParam.LMax;
		else if (tmp3 < iqIacCtrlParam.LMin) tmp3 = iqIacCtrlParam.LMin;
	//Find Duty cycle
		iqPwmDuty = _IQdiv(tmp3, iqVDC);
		if (iqPwmDuty >= viqMXDuty) iqPwmDuty = viqMXDuty;
		else if (iqPwmDuty <= -viqMXDuty) iqPwmDuty = -viqMXDuty;
	//Limit state variable
		if (iqPrCtrlState[1] > viqOne) iqPrCtrlState[1] = viqOne;
		else if (iqPrCtrlState[1] < -viqOne) iqPrCtrlState[1] = -viqOne;
		if (iqPrCtrlState[3] > viqOne) iqPrCtrlState[3] = viqOne;
		else if (iqPrCtrlState[3] < -viqOne) iqPrCtrlState[3] = -viqOne;

		return iqPwmDuty;
	}

_iq CurSetConv(float ISet)
	{
		float tmp;
		_iq out;
		if (IcSet > ILineMax) IcSet = ILineMax;
		if (IcSet < - ILineMax) IcSet = - ILineMax;
		tmp = ISet/vBasePCurrent;
		out = _IQ(tmp);
		if (VdcSet > 50) VdcSet = 50;
		else if (VdcSet < 24) VdcSet = 24;
		tmp = VdcSet/vBaseDCVoltage;
		iqVdcSet = _IQ(tmp);
		return out;
	}

#if VoltComp == 1
_iq FdFrdVoltCmp(void)		//Evaluate the feed forward voltage compensation
	{
		_iq tmp1, tmp2, tmp3;
		tmp1 = _IQmpy(iqKWoL, iqIm);
		iqDelta = _IQatan2(tmp1, iqVacEval);
		tmp2 = -_IQcos(iqTheta - iqDelta);	//tmp2 = -cos(iqTheta - Delta)
		tmp3 = _IQmpy(iqKVac2Vdc, iqVacEval);
		tmp1 = _IQcos(iqDelta);
		tmp1 = _IQdiv(tmp3, tmp1);
		tmp3 = _IQmpy(tmp2, tmp1);
		return tmp3;
	}
#elif VoltComp == 2
_iq FdFrdVoltCmp(void)		//Evaluate the feed forward voltage compensation
	{
		_iq tmp1, tmp2, tmp3, tmp4;
		tmp1 = _IQmpy(iqKWoL, iqIm);	//tmp1=wLIm
		tmp2 = _IQmpy(tmp1, tmp1)+_IQmpy(iqVacEval, iqVacEval);
		tmp3 = _IQsin(iqPhShift);
		tmp4 = _IQmpy(tmp3, iqVacEval);	//tmp4 = Um*sin(iqPhiShift)
		tmp3 = _IQmpy2(tmp4);
		tmp2 -= _IQmpy(tmp3, tmp1);
		tmp3 = _IQsqrt(tmp2);	//tmp3 = Uvm
		tmp2 = tmp1 - tmp4;
		tmp2 = _IQdiv(tmp2, tmp3);
		tmp1 = _IQasin(tmp2);
		iqDelta = iqPhShift + tmp1;
		tmp2 = -_IQcos(iqTheta - iqDelta);
		tmp4 = _IQmpy(tmp2, tmp3);
		tmp3 = _IQmpy(iqKVac2Vdc, tmp4);
		return tmp3;
	}
#endif

_iq NotchFilter(_iq u)
	{
		_iq tmp;
		tmp = _IQmpy(iqNotchCoef[2], u);
		tmp += _IQmpy(iqNotchCoef[3], iqNotch[2])+_IQmpy(iqNotchCoef[4], iqNotch[3]);
		tmp = tmp - _IQmpy(iqNotchCoef[0], iqNotch[0])-_IQmpy(iqNotchCoef[1], iqNotch[1]);
		iqNotch[1] = iqNotch[0];
		iqNotch[0] = tmp;
		iqNotch[3] = iqNotch[2];
		iqNotch[2] = u;
		return tmp;
	}

void LnFrqEva(void)
{
	//Find average line frequency in per unit with total 512 samples (about 3 cycle in 60 Hz)
	int index;
	_iq tmp;
	index = LFSampleCnt / 64;	//integer division
	iqMeanLF[index] += iqWfreq;
	LFSampleCnt++;
	if (LFSampleCnt == 512)
	{
		LFSampleCnt = 0;
		tmp = 0;
		for (index = 0; index < 8; index++)
		{
			tmp += _IQdiv64(iqMeanLF[index]);
			iqMeanLF[index] = 0;
		}
		iqAveLFreq = _IQdiv8(tmp);
	}
	return;
}

void DisableActiveRectifier(void)
	{//Stop the rectifier
		int temp;
		temp = EvaRegs.ACTRA.all;
		temp &=0xFF00;	//PWM3 and PWM1 forced low;
		temp |=0x00CC;	//PWM4 and PWM2forced high
		EvaRegs.ACTRA.all = temp;	//Update the output action register
		return;
	}

void EnableActiveRectifier(void)
	{//Enable the rectifier, this function may not be used because of initial compare values are not available now yet.
		int temp;
		temp = EvaRegs.ACTRA.all;
		temp &=0xFF00;	//PWM3 and PWM1 active low;
		temp |=0x0055;	//PWM4 and PWM2 active low
		EvaRegs.ACTRA.all = temp;	//Update the output action register
		return;
	}

void DisablePhaseC(void)
	{//Stop the phase C switching power pole
		int temp;
		temp = EvaRegs.ACTRA.all;
		temp &=0xF0FF;	//PWM5 forced low;
		temp |=0x0C00;	//PWM6 forced high
		EvaRegs.ACTRA.all = temp;	//Update the output action register
		return;
	}

void EnablePhaseC(void)
	{//Enable the phase C switching power pole
		int temp;
		EvaRegs.CMPR3= 0;
		temp = EvaRegs.ACTRA.all;
		temp &=0xF0FF;
		temp |=0x0500;	//PWM5 and PWM6 active low.
		EvaRegs.ACTRA.all = temp;	//Update the output action register
		return;
	}

void EnablePhaseC2(void)
	{//Enable the phase C switching power pole with initial voltage setting
		int temp, PwmDuty;
		_iq iqPICOut, iqtmp;	//PICOut in iq format
		_iq16 iq16tmp1, iq16tmp2;
		//The function is suitable for the load with a battery.
		//The detected voltage of the battery is used for the feed forward compensation of the current controller.
		iqtmp = _IQmpy(iqLdVdc, vBaseVoltRatio1);	//Convert the per-unit DC load voltage from load voltage base to dc-link voltage base.
		iqPICOut = _IQdiv(iqtmp, iqVDC);	//add the voltage compensation to the duty-cycle output
		if (iqPICOut > viqMXDuty2) iqPICOut = viqMXDuty2;
		else if (iqPICOut < 0) iqPICOut = 0;	//Limit the duty cycle between 0 to 0.96 in IQ24 format
		iqIcCurCtrlState.ui = iqPICOut;		//Initializae the current controller for phase C switching power pole
		iq16tmp1 = _IQtoIQ16(iqPICOut);
		iq16tmp2 = _IQ16mpy(iq16tmp1, iq16PwmPrd);
		PwmDuty = _IQ16int(iq16tmp2);
		if (PwmDuty <= MNDuty) PwmDuty = 0;
		EvaRegs.CMPR3= PwmDuty;	//Set the initial compare value for PWM generation based on the detected load voltage
//		EvaRegs.CMPR3= 0;
		temp = EvaRegs.ACTRA.all;
		temp &=0xF0FF;
		temp |=0x0500;	//PWM5 and PWM6 active low.
		EvaRegs.ACTRA.all = temp;	//Update the output action register
		return;
	}

void DisRecEnaPhaseC2(void)
{//Disable the singal-phase rectifier and Enable the phase C switching power pole with initial voltage setting
		int temp, PwmDuty;
		_iq iqPICOut, iqtmp;	//PICOut in iq format
		_iq16 iq16tmp1, iq16tmp2;
		//The function is suitable for the load with a battery.
		//The detected voltage of the battery is used for the feed forward compensation of the current controller.
		iqtmp = _IQmpy(iqLdVdc, vBaseVoltRatio1);	//Convert the per-unit DC load voltage from load voltage base to dc-link voltage base.
		iqPICOut = _IQdiv(iqtmp, iqVDC);	//add the voltage compensation to the duty-cycle output
		if (iqPICOut > viqMXDuty2) iqPICOut = viqMXDuty2;
		else if (iqPICOut < 0) iqPICOut = 0;	//Limit the duty cycle between 0 to 0.96 in IQ24 format
		iqIcCurCtrlState.ui = iqPICOut;		//Initializae the current controller for phase C switching power pole
		iq16tmp1 = _IQtoIQ16(iqPICOut);
		iq16tmp2 = _IQ16mpy(iq16tmp1, iq16PwmPrd);
		PwmDuty = _IQ16int(iq16tmp2);
		if (PwmDuty <= MNDuty) PwmDuty = 0;
		EvaRegs.CMPR3= PwmDuty;	//Set the initial compare value for PWM generation based on the detected load voltage
//		EvaRegs.CMPR3= 0;
		temp = EvaRegs.ACTRA.all;
		temp &=0xF000;
		temp |=0x05CC;	//PWM5 and PWM6 active low, //PWM4 and PWM2forced high, PWM3 and PWM1 forced low;
		EvaRegs.ACTRA.all = temp;	//Update the output action register
		return;
}

void BatTestFun1(void)
{
	_iq tmp;
	if (SysCtrlWord.bit.TestReset == 1)
	{
		//Start a new test
		TimeCnt = 0;
		IcSet = LowIc;
		//Initialize battery current controller
		iqVdcSet = 0; //This line should be removed. iqVdcSet should be set based on VdcSet
		iqIcCurCtrlState.ei = 0;
		iqIcCurCtrlState.ui = 0;
		EnablePhaseC();
		SysCtrlWord.bit.TestReset = 0;
		BatteryState.bit.BatTestState = 1;	//Set the battery in operating mode
	}
	else if (BatteryState.bit.BatTestState == 1)
	{
		if (TimeCnt == FiveMin) IcSet = HighIc;
		else if (TimeCnt == TenMin)
		{
			IcSet = 0;
			DisablePhaseC();
			BatteryState.bit.BatTestState = 0;	//Set battery in open-circuit mode
		}
	}

	iqMeanIc += iqIC;
	iqMeanVc +=iqLdVdc;
	MeanCnt++;
	if (MeanCnt >=256)
	{
		MeanCnt = 0;
		iqSamples[0] = TimeCnt/10000;	//take the number of seconds
		tmp = _IQdiv16(iqMeanIc);
		iqSamples[1] = _IQdiv16(tmp);
		tmp = _IQdiv16(iqMeanVc);
		iqSamples[2] = _IQdiv16(tmp);
		iqSamples[3] = iqAveLFreq;
		iqMeanIc = 0;
		iqMeanVc = 0;
	}
	TimeCnt++;
	return;
}

void BatTestFun2(void)
{
	_iq tmp, tmp2;
	_iq21 tmp3;
	_iq23 tmp4;
	if (SysCtrlWord.bit.TestReset == 1)
	{
		//reset and start a new test
		TimeCnt = 0;
		//A protest function for wrongly set parameters
		if (BatTestCfg.OpPeriod <=10) BatTestCfg.OpPeriod = 10;
		if (BatTestCfg.OpTime <= 5) BatTestCfg.OpTime = 5;
		if (BatTestCfg.OpTime >= BatTestCfg.OpPeriod) BatTestCfg.OpTime = BatTestCfg.OpPeriod - 5;
		BatTestVar.CycleCnt = 0;
		BatTestVar.iqVccpk = 0;		//Peak voltage of completed cycle is reset
		BatTestVar.CycleStart = 20000; //a 20000 unit delay
		BatTestVar.CycleEnd = BatTestVar.CycleStart + BatTestCfg.OpPeriod;
		BatTestVar.SwTime = BatTestVar.CycleStart + BatTestCfg.OpTime;
		SysCtrlWord.bit.TestReset = 0;	//Battery test reset is completed.
		BatteryState.bit.Discharged = 0;
		BatteryState.bit.FullCharged = 0;
		BatteryState.bit.BatTest = 1;
		IcSet = 0;
		DisablePhaseC();
		BatteryState.bit.BatTestState = 0;	//Set battery in open-circuit mode
	}
	if (BatteryState.bit.BatTest == 1)
	{
		if (TimeCnt == BatTestVar.CycleStart)
		{
			IcSet = BatTestCfg.Current;
			if (IcSet >= 0) BatteryState.bit.BatTestMode = 0; //For charging mode
			else BatteryState.bit.BatTestMode = 1; //Discharging mode
			//Initialize battery current controller
	//		iqVdcSet = 0;
			BatTestVar.iqVcpk = 0;
			iqIcCurCtrlState.ei = 0;
			iqIcCurCtrlState.ui = 0;
			EnablePhaseC();
			BatteryState.bit.BatTestState = 1;	//Set the battery in operating mode
		}
		if (TimeCnt == BatTestVar.SwTime)
		{
			IcSet = 0;
			DisablePhaseC();
			BatteryState.bit.BatTestState = 0;	//Set battery in open-circuit mode
		}
		if (TimeCnt == BatTestVar.CycleEnd)
		{
			if (BatteryState.bit.BatTestMode == 0)
			{
				if (BatTestVar.iqVccpk < BatTestVar.iqVcpk) BatTestVar.iqVccpk = BatTestVar.iqVcpk; //Update the peak voltage of completed cycle
				else
				{
					if (BatTestVar.iqVccpk > 0)
					{ //If the voltage peak falls to 99% of the peak value, the battery is considered as fully charged. The test is completed.
						tmp = _IQdiv(BatTestVar.iqVcpk, BatTestVar.iqVccpk);
						if (tmp < iqValue1)
						{//
							BatteryState.bit.BatTest = 0;
							BatteryState.bit.FullCharged = 1;
						}
					}
				}
			}
			BatTestVar.CycleCnt++;
			if (BatTestVar.CycleCnt >= BatTestCfg.OpCycle)
			{ //Battery test is completed
				BatteryState.bit.BatTest = 0;
			}
			else
			{ //Prepare for the next test cycle
				BatTestVar.CycleStart = TimeCnt+1;	//
				BatTestVar.CycleEnd = BatTestVar.CycleStart + BatTestCfg.OpPeriod;
				BatTestVar.SwTime = BatTestVar.CycleStart + BatTestCfg.OpTime;
			}
		}
	}
	else if (BatteryState.bit.BatTestState == 1)
	{//If the battery test is stopped by instruction, but the Battery test is still under the processing, execute the following instructions to end the test operation.
		IcSet = 0;
		DisablePhaseC();
		BatteryState.bit.BatTestState = 0;	//Set battery in open-circuit mode
	}

//Evaluation of average state values
	iqMeanIc += iqIC;
	iqMeanVc +=iqLdVdc;
	MeanCnt++;
	if (MeanCnt >=256)
	{
		MeanCnt = 0;
		iqSamplesS.iqSTime = TimeCnt/10000;	//take the number of seconds
		tmp = _IQdiv16(iqMeanIc);
		iqSamplesS.iqSIc = _IQdiv16(tmp);	//Save the average current value
		tmp = _IQdiv16(iqMeanVc);
		iqSamplesS.iqSVc = _IQdiv16(tmp);	//Save the average voltage value
		iqSamplesS.iqSLFreq = iqAveLFreq;		//Save the average frequency value
		iqMeanIc = 0;
		iqMeanVc = 0;
		tmp2 = _IQmpy(iqSamplesS.iqSIc, viqTimeStep);
		tmp3 = _IQtoIQ21(tmp2);
		iqQTime += tmp3;
		tmp2 = _IQmpy(iqSamplesS.iqSVc, iqSamplesS.iqSIc);
		tmp2 = _IQmpy(tmp2, viqTimeStep);
		tmp4 = _IQtoIQ23(tmp2);
		iqWTime += tmp4;

		//Evaluation if the battery is fully charged or fully discharged during the test operation
		if (BatteryState.bit.BatTestState == 1)
		{
			if (BatteryState.bit.BatTestMode == 0)
			{ //Charging mode
				if (BatTestVar.iqVcpk < iqSamplesS.iqSVc) BatTestVar.iqVcpk = iqSamplesS.iqSVc;
				else
				{//Within a cycle, if the voltage drops to 99% of the peak value, the battery is assumed to be fully charged. The test is completed.
					tmp = _IQdiv(iqSamplesS.iqSVc, BatTestVar.iqVcpk);
					if (tmp < iqValue1)
					{
						IcSet = 0;
						DisablePhaseC();
						BatteryState.bit.BatTestState = 0;	//Set battery in open-circuit mode
						BatteryState.bit.BatTest = 0;
						BatteryState.bit.FullCharged = 1;
					}
				}
			}
			else //In discharging mode
			{
				tmp = iqSamplesS.iqSVc/BatTestCfg.Ncell;
				if (tmp <= BatVoltIqUnit)
				{//If the battery voltage drops to 1 V per cell, the battery is fully discharged. The test is completed.
					IcSet = 0;
					DisablePhaseC();
					BatteryState.bit.BatTestState = 0;	//Set battery in open-circuit mode
					BatteryState.bit.BatTest = 0;
					BatteryState.bit.Discharged = 1;
				}
			}
		}
	}
	TimeCnt++;
	return;
}

void fBatTestInit(void)
{
	//Battery test mode
	//Set parameters for battery test
	LowIc = -0.15;
	HighIc = -2;	//Negative for discharging, positive for charging
	TimeCnt = OneHour;
	MeanCnt = 0;
	iqMeanIc = 0;
	iqMeanVc = 0;
	BatteryState.bit.BatTest = 1; //Set the battery test active
	return;
}

void fVacMonitor(void)				//Monitor the ac input line voltage and process the minimum voltage error
{
	long tmp;

	if (SysCtrlWord.bit.LowVac == 0)
	{
		if (iqVacEval < viqMiniVac)
		{
			SysCtrlWord.bit.LowVac = 1;
			if (SysCtrlWord.bit.PEnable == 1)
			{
				if (SysCtrlWord.bit.TestMode != 3) SysCtrlWord.bit.PEnable = 0; //disable the power stage
				iGTimeIns1 = iGTimeIns2;
			}
		}
	}
	else
	{
		if (iqVacEval > viqMiniVac)
		{
			if (iGTimeIns1 == iGTimeIns2) iGTimeIns1 = iGTimeCnt;
			tmp = iGTimeCnt - iGTimeIns1;
			if (tmp < 0) tmp =-tmp;
			if (tmp > viDelayTime) SysCtrlWord.bit.LowVac = 0;
		}
		else iGTimeIns1 = iGTimeIns2;
	}
	return;
}
//===========================================================================
// End of SourceCode.
//===========================================================================


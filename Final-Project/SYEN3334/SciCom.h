//###########################################################################
//
// FILE:	A_SciCom.h for STM32F4xx
//
// TITLE:	SCI Communication
//
//###########################################################################
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  v1.0 | 12Dec2016 | J.Z. |Migrate from version for TMS320F2812
//###########################################################################

extern struct DataSet DataSetCom;
extern struct CurveData CurveCom;
extern void CmdDecode(void); //command translation
extern void SLComInit(void);
extern void SLCom(void);
extern void USART6_Configure(void);
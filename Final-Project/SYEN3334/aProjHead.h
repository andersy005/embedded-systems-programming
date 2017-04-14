//########################################################################################
//
// FILE:	aProjHead.h for STM32F4xx
//
// TITLE:	Header file of a project
//
//########################################################################################
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|============================================================
//  v1.0| Dec.12,2016 | J.Z. | version developed from a_control.c
//########################################################################################


struct FourByte{
	uint16_t WordL;
	uint16_t WordH;
};

union Float2Byte {
	float	Float;
	struct 	FourByte Word;
};

struct DataSet
{
	char DataNum;
	long Addr[10];
	char WrdNum[10];
	int ByteNum;
};

struct CurveData
{
	char DataNum;
	long Addr[2];
	char WrdNum[2];
	char delay;
	char tcnt;
};

struct TRDataBlk
{
	char Mode;
	long Addr;
	int Length;
};

struct SLDataBlk
{
        int32_t RSetVal; //Setting value
        int32_t RCtrlWd; //Control word
        int32_t SAddr;  //Beginning address of the data structure
        int16_t SNum;   //Number of units for sending data back to Simulink
};

struct SLDataSent
{
        float data1;
        float data2;
        float data3;
        float data4;
};
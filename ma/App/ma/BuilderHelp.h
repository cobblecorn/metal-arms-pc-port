//////////////////////////////////////////////////////////////////////////////////////
// BuilderHelp.h
//
// What: assists in the implemention of a builder
//       by providing a more automated way getting data from the CSV 
//       into the member variables of a builder
//
//
//	How: make a table that looks something like:
//
//		static InterpretTableDataSpec _aItableDataSpec[] = 
//		{
//		     Tag								Address of Vbl To Be set							DataType		pMinMaxSpec				BuilderHelp_EnumTagSpec			uNumEnumTags in Spec		Group Bool									Internal Use/NULL	Internal Use/NULL
//			"XA_ALARM",							(u32) &CAlarmSysBuilder::s_bIsAlarm,				BDT_BOOL,		NULL,						NULL,						0,							(u32) &CAlarmSysBuilder::s_bIsAlarm,			NULL,				NULL,
//
//			then pass a pointer to the table into the Builder Helper, and your builders member vbls will be filled in if
//          the user props say to do so.
//
// Author: Pat M
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 02/6/03 Pat M       Move code to this file from AIBuilder.cpp
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _BUILDERHELP_H_
#define _BUILDERHELP_H_ 1

enum e_BuilderDataTypes
{
	 BDT_U32 = 0,
	 BDT_F32,
	 BDT_ENUM,
	 BDT_BOOL,
	 BDT_STRING,
	 BDT_ENUMS2BITS,
	 BDT_S32,
};

struct BuilderHelp_EnumTagSpec
{
	char* pszEnumTag;
	s32 nValue;
};

struct BuilderHelp_U32MinMaxSpec
{
	u32 uMin;
	u32 uMax;
	BOOL bClamp;
};

struct BuilderHelp_F32MinMaxSpec
{
	f32 fMin;
	f32 fMax;
	BOOL bClamp;
};

struct BuilderHelp_S32MinMaxSpec
{
	s32 nMin;
	s32 nMax;
	BOOL bClamp;
};

#define NO_TRACKGROUPEDBOOLOFFSET (0xffff)

struct InterpretTableDataSpec
{
	char* pszTag;													   //The Tag, or table name as it should appear in the fgamedata
	u16 uDataOffset;												   //address (relative to builder object) at which the parsed data should be stored (should be address of a vbl of type uDataType)
	u8 uDataType;													   //type of data expected
	u8 uNumEnumTags;												   //size of array pointed to by paBuilderHelp_EnumTagSpec
	void* pMinMaxSpec;												   //some specs on how to clamp or range the data
	BuilderHelp_EnumTagSpec* paBuilderHelp_EnumTagSpec;				   //for datatype = enum, list of possible enums

	u16 uTrackGroupBoolOffset;

	union															   //internal use data
	{																   //internal use data
		s32*  pS32;
		u32*  pU32;													   
		f32*  pF32;													   
		u32*  pEnum;												   
		BOOL* pBool;												   
		void* pData;												   
		cchar** ppszData;											   
	} data;															   

	BOOL* pTrackGroupBool;											   //is part of a group of tags that should be tracked.  If so, address of bool to set if any in group are used.
};

void BuilderHelp_SetDefaults(void* pBuilder, InterpretTableDataSpec *paItableDataSpec, u32 uNumItableFields);
void BuilderHelp_InterpretTable(InterpretTableDataSpec *paItableDataSpec, u32 uNumItableFields, BOOL *pbDidInterpret);

//confusing, but helpfull funcs
void BuilderHelp_SetU8IfValid(u8* puVal, u32 uSetting);			 //if uSetting is not 0xffffffff then puVal will be assigned it
void BuilderHelp_SetU16IfValid(u16* puVal, u32 uSetting);		 //if uSetting is not 0xffffffff then puVal will be assigned it
void BuilderHelp_SetS8IfValid(s8* pnVal, u32 uSetting);			 //if uSetting is not 0xffffffff then pnVal will be assigned it
void BuilderHelp_SetF32IfValid(f32* pfVal, f32 fDefault);		 //if uSetting is not -1.0f then pfVal will be assigned it
void BuilderHelp_SetU32IfValid(u32* puVal, u32 uDefault);		 //if uSetting is not 0xffffffff then puVal will be assigned it

void BuilderHelp_SetSafeDefaultF32(f32* pfVal, f32 fDefault);			 //if *pfVal is not -1.0f  then it will be assigned fDefault
void BuilderHelp_SetSafeDefaultU32(u32* puVal, u32 uDefault);			 //if *pfVal is not 0xffffffff	 then it will be assigned uDefault


extern BuilderHelp_U32MinMaxSpec BuilderHelp_ZeroToHunWithClamp;
extern BuilderHelp_U32MinMaxSpec BuilderHelp_U8;
extern BuilderHelp_F32MinMaxSpec BuilderHelp_PosF32MMS;
extern BuilderHelp_F32MinMaxSpec BuilderHelp_PosUnitF32MMS;
extern BuilderHelp_F32MinMaxSpec BuilderHelp_NegOneToPosUnitF32MMS;
extern BuilderHelp_S32MinMaxSpec BuilderHelp_S32;
extern BuilderHelp_U32MinMaxSpec BuilderHelp_U32;
extern BuilderHelp_U32MinMaxSpec BuilderHelp_U16;


#if FANG_PRODUCTION_BUILD 
	#define BUILDER_ERROR_STRING(s) "b"
#else
	#define BUILDER_ERROR_STRING(s) s
#endif



#endif

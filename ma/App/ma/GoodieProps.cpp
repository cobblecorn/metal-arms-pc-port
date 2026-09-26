//////////////////////////////////////////////////////////////////////////////////////
// GoodieProps.cpp - Goodie properties class for Mettle Arms.
//
// Author: Justin Link
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
// 05/23/02	Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "GoodieProps.h"
#include "fclib.h"
#include "fmath.h"

CGoodieProps::CGoodieProps()
{
	m_afSpeed[0] = 1.0f;
	m_afSpeed[1] = 1.0f;

	m_fDispersion = 0.0f;
}



BOOL CGoodieProps::InterpretTable(FGameDataTableHandle_t hTableHandle, CGoodieProps **ppGoodieProps)
{
	cchar *pszTableName = fgamedata_GetTableName(hTableHandle);
	FGameData_VarType_e eFieldType;
	const void *pField;

	if(fclib_stricmp(pszTableName, "gp_dispersion") == 0)
	{
		CreateGoodieProps(ppGoodieProps);
		pField = fgamedata_GetPtrToFieldData(hTableHandle, 0, eFieldType);
		if(eFieldType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CGoodieProps::InterpretTable() : Error in field for 'gp_dispersion' table.  Wrong data type.\n");
			return(TRUE);
		}

		(*ppGoodieProps)->m_fDispersion = FMATH_DEG2RAD(*((const f32 *)(pField)));
		return(TRUE);
	}
	else if(fclib_stricmp(pszTableName, "gp_speed1") == 0)
	{
		CreateGoodieProps(ppGoodieProps);
		pField = fgamedata_GetPtrToFieldData(hTableHandle, 0, eFieldType);
		if(eFieldType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CGoodieProps::InterpretTable() : Error in field for 'gp_speed1' table.  Wrong data type.\n");
			return(TRUE);
		}

		(*ppGoodieProps)->m_afSpeed[0] = *((const f32 *)(pField));
		return(TRUE);
	}
	else if(fclib_stricmp(pszTableName, "gp_speed2") == 0)
	{
		CreateGoodieProps(ppGoodieProps);
		pField = fgamedata_GetPtrToFieldData(hTableHandle, 0, eFieldType);
		if(eFieldType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CGoodieProps::InterpretTable() : Error in field for 'gp_speed2' table.  Wrong data type.\n");
			return(TRUE);
		}

		(*ppGoodieProps)->m_afSpeed[1] = *((const f32 *)(pField));
		return(TRUE);
	}

	return(FALSE);
}



void CGoodieProps::CreateGoodieProps(CGoodieProps **ppGoodieProps)
{
	if(*ppGoodieProps == NULL)
	{
		*ppGoodieProps = fnew CGoodieProps;

		if(*ppGoodieProps == NULL)
		{
			DEVPRINTF("CGoodieProps::CreateGoodieProps() : Out of memory.\n");
		}
	}
}

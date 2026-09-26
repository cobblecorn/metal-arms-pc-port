#include "fang.h"
#include "fgamedata.h"
#include "fclib.h"
#include "BuilderHelp.h"
#include "Entity.h"

void BuilderHelp_SetDefaults(void* pBuilder, InterpretTableDataSpec *paItableDataSpec, u32 uNumItableFields)
{
	u32 i;
	for (i = 0; i <	uNumItableFields; i++)
	{
		paItableDataSpec[i].data.pData = (void*) ((u32)pBuilder + paItableDataSpec[i].uDataOffset);

		if (paItableDataSpec[i].uTrackGroupBoolOffset != NO_TRACKGROUPEDBOOLOFFSET)
		{
		   paItableDataSpec[i].pTrackGroupBool = (BOOL*) ((u32)pBuilder + (paItableDataSpec[i].uTrackGroupBoolOffset));
		   *(paItableDataSpec[i].pTrackGroupBool) = FALSE;
		}
		else
		{
			paItableDataSpec[i].pTrackGroupBool = NULL;
		}

		//if using U32 or F32, or S32, specify a minmaxspec
		FASSERT(!((paItableDataSpec[i].uDataType == BDT_S32) || (paItableDataSpec[i].uDataType == BDT_U32) || (paItableDataSpec[i].uDataType == BDT_F32)) || paItableDataSpec[i].pMinMaxSpec);  //if your gonna use U32 or F32, or S32, specify a minmaxspec
	}
}



void BuilderHelp_InterpretTable(InterpretTableDataSpec *paItableDataSpec, u32 uNumItableFields, BOOL *pbDidInterpret)
{
	FGameDataTableHandle_t hCurTable = CEntityParser::m_hTable;
	FGameData_VarType_e eVarType;
	BOOL bDidInterpret = FALSE;		
	u32 i,j;

	for (i = 0; (i < uNumItableFields) && !bDidInterpret; i++)
	{
		switch (paItableDataSpec[i].uDataType)
		{
				case BDT_U32:
				{
					if ( !fclib_stricmp( CEntityParser::m_pszTableName, paItableDataSpec[i].pszTag ) )
					{
						CEntityParser::Interpret_U32( paItableDataSpec[i].data.pU32, ((BuilderHelp_U32MinMaxSpec*) paItableDataSpec[i].pMinMaxSpec)->uMin, ((BuilderHelp_U32MinMaxSpec*) paItableDataSpec[i].pMinMaxSpec)->uMax, ((BuilderHelp_U32MinMaxSpec*) paItableDataSpec[i].pMinMaxSpec)->bClamp);
						bDidInterpret = TRUE;
					}

				}
				break;
			case BDT_F32:
				{
					if ( !fclib_stricmp( CEntityParser::m_pszTableName, paItableDataSpec[i].pszTag ) )
					{
						CEntityParser::Interpret_F32( paItableDataSpec[i].data.pF32, ((BuilderHelp_F32MinMaxSpec*) paItableDataSpec[i].pMinMaxSpec)->fMin, ((BuilderHelp_F32MinMaxSpec*) paItableDataSpec[i].pMinMaxSpec)->fMax, ((BuilderHelp_U32MinMaxSpec*) paItableDataSpec[i].pMinMaxSpec)->bClamp);
						bDidInterpret =  TRUE;
					}
				}
				break;
			case BDT_S32:
				{
					if ( !fclib_stricmp( CEntityParser::m_pszTableName, paItableDataSpec[i].pszTag ) )
					{
						CEntityParser::Interpret_S32( paItableDataSpec[i].data.pS32, ((BuilderHelp_S32MinMaxSpec*) paItableDataSpec[i].pMinMaxSpec)->nMin, ((BuilderHelp_S32MinMaxSpec*) paItableDataSpec[i].pMinMaxSpec)->nMax, ((BuilderHelp_S32MinMaxSpec*) paItableDataSpec[i].pMinMaxSpec)->bClamp);
						bDidInterpret =  TRUE;
					}
				}
				break;
			case BDT_BOOL:
				{
					if ( !fclib_stricmp( CEntityParser::m_pszTableName, paItableDataSpec[i].pszTag ) )
					{	
						cchar *pszValue;
						CEntityParser::Interpret_String(&pszValue);

						if(!fclib_stricmp(pszValue, "TRUE") || !fclib_stricmp(pszValue, "ON") || !fclib_stricmp(pszValue, "YES") || !fclib_stricmp(pszValue, "1"))
						{
							*(paItableDataSpec[i].data.pBool) = TRUE;
						}
						else if(!fclib_stricmp(pszValue, "FALSE") || !fclib_stricmp(pszValue, "OFF") || !fclib_stricmp(pszValue, "NO") || !fclib_stricmp(pszValue, "0"))
						{
							*(paItableDataSpec[i].data.pBool) = FALSE;
						}
						else
						{
							DEVPRINTF("CAIBuilder::InterpretTable() : Invalid value of '%s' for '%s' table. Use (ON, OFF).\n", pszValue, paItableDataSpec[i].pszTag);
						}
						bDidInterpret = TRUE;
					}
				}
				break;
			case BDT_ENUM:
				{
					if ( !fclib_stricmp( CEntityParser::m_pszTableName, paItableDataSpec[i].pszTag ) )
					{
						cchar *pszValue;
						BOOL bValidEnum = FALSE;
						CEntityParser::Interpret_String(&pszValue);
						FASSERT(paItableDataSpec[i].uNumEnumTags > 0);
						for (j = 0; j < (u32)(paItableDataSpec[i].uNumEnumTags-1) && !bValidEnum; j++)
						{
							if(!fclib_stricmp(pszValue, paItableDataSpec[i].paBuilderHelp_EnumTagSpec[j].pszEnumTag))
							{
								*(paItableDataSpec[i].data.pEnum) = paItableDataSpec[i].paBuilderHelp_EnumTagSpec[j].nValue;
								bValidEnum = TRUE;
							}
						}

						if (!bValidEnum)
						{
							DEVPRINTF(paItableDataSpec[i].paBuilderHelp_EnumTagSpec[paItableDataSpec[i].uNumEnumTags-1].pszEnumTag, pszValue);
						}
						bDidInterpret = TRUE;
					}
				}
				break;
			case BDT_STRING:
				{
					if ( !fclib_stricmp( CEntityParser::m_pszTableName, paItableDataSpec[i].pszTag ) )
					{
						CEntityParser::Interpret_String( ( paItableDataSpec[i].data.ppszData) );
						bDidInterpret = TRUE;
					}
				}
				break;
			case BDT_ENUMS2BITS:
				{
					if ( !fclib_stricmp( CEntityParser::m_pszTableName, paItableDataSpec[i].pszTag ) )
					{
						cchar *pszCurField;
						u32 uNumEntries = CEntityParser::m_nFieldCount;
						for(u32 uCurEntry = 0; uCurEntry < uNumEntries; ++uCurEntry)
						{
							pszCurField = (cchar *)(fgamedata_GetPtrToFieldData(hCurTable, uCurEntry, eVarType));

							BOOL bClearBit = FALSE;
							if (pszCurField[0] =='!')
							{
								bClearBit = TRUE;
								pszCurField++;
							}
							BOOL bValidEnum = FALSE;
							FASSERT(paItableDataSpec[i].uNumEnumTags > 0);
							for (j = 0; j < (u32)(paItableDataSpec[i].uNumEnumTags-1) && !bValidEnum; j++)
							{
								if (!fclib_stricmp(pszCurField, paItableDataSpec[i].paBuilderHelp_EnumTagSpec[j].pszEnumTag))
								{
									if (*(paItableDataSpec[i].data.pU32) == 0xffffffff)
									{
										*(paItableDataSpec[i].data.pU32) = 0;
									}

									if (bClearBit)
									{
										*(paItableDataSpec[i].data.pU32) &= ~paItableDataSpec[i].paBuilderHelp_EnumTagSpec[j].nValue;
									}
									else
									{
										*(paItableDataSpec[i].data.pU32) |= paItableDataSpec[i].paBuilderHelp_EnumTagSpec[j].nValue;
									}
									bValidEnum = TRUE;
								}
							}
							
							if (!bValidEnum)
							{
								DEVPRINTF(paItableDataSpec[i].paBuilderHelp_EnumTagSpec[paItableDataSpec[i].uNumEnumTags-1].pszEnumTag, pszCurField);
							}
						}
						bDidInterpret = TRUE;
					}
				}
				break;
		}
	}


	if (bDidInterpret)
	{
		FASSERT(i> 0 && i <= uNumItableFields);

		if (paItableDataSpec[i-1].pTrackGroupBool)
		{
			*(paItableDataSpec[i-1].pTrackGroupBool) = TRUE;
		}
	}

	if (pbDidInterpret)
	{
		*pbDidInterpret = bDidInterpret;
	}
}

void BuilderHelp_SetU8IfValid(u8* puVal, u32 uSetting)
{
	if (uSetting != 0xffffffff && uSetting < 256)
	{
		*puVal = (u8) uSetting;
	}
}

void BuilderHelp_SetU16IfValid(u16* puVal, u32 uSetting)
{
	if (uSetting != 0xffffffff && uSetting <= 0xffff)
	{
		*puVal = uSetting;
	}
}

void BuilderHelp_SetS8IfValid(s8* puVal, u32 uSetting)
{
	if (uSetting != 0xffffffff && uSetting <= 0xff)
	{
		*puVal = (s8) uSetting;
	}
}


void BuilderHelp_SetF32IfValid(f32* pfVal, f32 fSetting)
{
	if (fSetting < 0.0f)
	{
		*pfVal = fSetting;
	}
}

void BuilderHelp_SetU32IfValid(u32* puVal, u32 uSetting)
{
	if (uSetting == 0xffffffff)
	{
		*puVal = uSetting;
	}
}

void BuilderHelp_SetSafeDefaultF32(f32* pfVal, f32 fDefault)
{
	if (*pfVal < 0.0f)
	{
		*pfVal = fDefault;
	}
}

void BuilderHelp_SetSafeDefaultU32(u32* puVal, u32 uDefault)
{
	if (*puVal ==0xffffffff)
	{
		*puVal = uDefault;
	}
}


BuilderHelp_U32MinMaxSpec BuilderHelp_ZeroToHunWithClamp = 
{
	0, 100, TRUE,
};

BuilderHelp_U32MinMaxSpec BuilderHelp_U8 = 
{
	0, 255, TRUE,
};

BuilderHelp_F32MinMaxSpec BuilderHelp_PosF32MMS = 
{
	0.0f, 10000.0f, TRUE,
};

BuilderHelp_F32MinMaxSpec BuilderHelp_PosUnitF32MMS = 
{
	0.0f, 1.0f, TRUE,
};

BuilderHelp_F32MinMaxSpec BuilderHelp_NegOneToPosUnitF32MMS = 
{
	-1.0f, 10000.0f, TRUE,
};

BuilderHelp_S32MinMaxSpec BuilderHelp_S32 = 
{
	-0x7fffffff, 0x7fffffff, TRUE,
};

BuilderHelp_U32MinMaxSpec BuilderHelp_U32 = 
{
	0, 0xffffffff, TRUE,
};

BuilderHelp_U32MinMaxSpec BuilderHelp_U16 = 
{
	0, 0xffff, TRUE,
};

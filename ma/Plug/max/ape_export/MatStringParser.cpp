//////////////////////////////////////////////////////////////////////////////////////
// MatStringParser.cpp - 
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 02/08/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "MatStringParser.h"
#include "fmath.h"

#define _AUTO_ID		128

typedef struct {
	u32 nErrorMask;
	cchar *pszErrorString;
} _ErrorCodes_t;

enum {
	MAT_STRING_ERROR_FLAG_SORT			= 0x00000001,
	MAT_STRING_ERROR_FLAG_ORDER			= 0x00000002,
	MAT_STRING_ERROR_FLAG_SHADER		= 0x00000004,
	MAT_STRING_ERROR_FLAG_MOTIF			= 0x00000008,
	MAT_STRING_ERROR_FLAG_ANIM			= 0x00000010,
	MAT_STRING_ERROR_FLAG_SCROLL		= 0x00000020,
	MAT_STRING_ERROR_FLAG_Z				= 0x00000040,
	MAT_STRING_ERROR_FLAG_ID			= 0x00000080,
	MAT_STRING_ERROR_FLAG_NOCOLL		= 0x00000100,
	MAT_STRING_ERROR_FLAG_COLL_ID		= 0x00000200,
	MAT_STRING_ERROR_FLAG_COLL			= 0x00000400,
	MAT_STRING_ERROR_FLAG_NODRAW		= 0x00000800,
	MAT_STRING_ERROR_FLAG_SURF			= 0x00001000,
	MAT_STRING_ERROR_FLAG_REACT			= 0x00002000,
	
	MAT_STRING_ERROR_FLAG_UNKNOWN_CMD	= 0x80000000,
	MAT_STRING_ERROR_FLAG_NONE			= 0x00000000
}; 

static const _ErrorCodes_t _aErrorCodes[] = {
	MAT_STRING_ERROR_FLAG_SORT,			"*sort error",
	MAT_STRING_ERROR_FLAG_ORDER,		"*order error",
	MAT_STRING_ERROR_FLAG_SHADER,		"*shader error",
	MAT_STRING_ERROR_FLAG_MOTIF,		"*motif error",
	MAT_STRING_ERROR_FLAG_ANIM,			"*anim error",
	MAT_STRING_ERROR_FLAG_SCROLL,		"*scroll error",
	MAT_STRING_ERROR_FLAG_Z,			"*z error",
	MAT_STRING_ERROR_FLAG_ID,			"*id error",
	MAT_STRING_ERROR_FLAG_NOCOLL,		"*nocoll error",	
	MAT_STRING_ERROR_FLAG_COLL_ID,		"*collid error",
	MAT_STRING_ERROR_FLAG_COLL,			"*coll error",
	MAT_STRING_ERROR_FLAG_NODRAW,		"*nodraw error",
	MAT_STRING_ERROR_FLAG_SURF,			"*surf error",
	MAT_STRING_ERROR_FLAG_REACT,		"*react error",

	MAT_STRING_ERROR_FLAG_UNKNOWN_CMD,	"unknown * command",
	MAT_STRING_ERROR_FLAG_NONE,			"",	
};

static BOOL _GetParameterString( cchar *pszCmd, CStr &rsParam ) {
	char *psz1stParenthesis, *psz2ndParenthesis, *pszNextCmd;
	int nLen;
	
	psz1stParenthesis = strchr( pszCmd, '(' );
	if( !psz1stParenthesis ) {
		// all paramerters must be enclosed in ()
		return FALSE;
	}
	psz2ndParenthesis = strchr( pszCmd, ')' );
	if( !psz2ndParenthesis ) {
		// all paramerters must be enclosed in ()
		return FALSE;	
	}

	// make sure that the ) is before the next *
	pszNextCmd = strchr( &pszCmd[1], '*' );
	if( pszNextCmd ) {
		if( (u32)psz2ndParenthesis > (u32)pszNextCmd ) {
			// these parenthesis are the next command's
			return FALSE;
		}
	}

	nLen = (u32)psz2ndParenthesis - (u32)psz1stParenthesis;
	nLen -= 1;
	if( nLen < 0 ) {
		// nothing is contained between the ()
		return FALSE;
	}
	// move psz1stParenthesis one char to the 1st parameter character
	psz1stParenthesis++;
	rsParam = psz1stParenthesis;
	rsParam.Resize( nLen );

	return TRUE;
}

/////////////////////////
// MATERIAL STRING PARSER
/////////////////////////

CMatStringParser::CMatStringParser() {
	ResetToDefaults();
}

CMatStringParser::~CMatStringParser() {
}

void CMatStringParser::Parse( cchar *pszMaterialString, BOOL bResetToDefaults/*=TRUE*/ ) {
	CStr sLowerMatString, sParam;
	char *pszData, *pszCmd;
	int nParam, nParam2, nParam3, nParam4, nParam5, nParam6, nParam7, nParam8, nParam9, nParam10, nParam11, nParam12;
	f32 fParam, fParam2, fParam3, fParam4;
	u32 nNumValidStarCommands = 0;

	if( bResetToDefaults ) {
		ResetToDefaults();
	}

	if( !pszMaterialString ) {
		return;
	}
	m_sOrigMatString = pszMaterialString;
	sLowerMatString = m_sOrigMatString;
	sLowerMatString.toLower();
	pszData = sLowerMatString.data();
	// look for *id command (we must look for this command 1st, cause *anim & *scroll use this value)
	pszCmd = strstr( pszData, "*id" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		// see if there is an id param
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%d", &nParam ) == 1 ) {
				if( nParam > 127 ) {
					m_nErrorMask |= MAT_STRING_ERROR_FLAG_ID;
				} else {
					m_ApeCommands.nID = nParam;
				}
			} else {
				m_nErrorMask |= MAT_STRING_ERROR_FLAG_ID;
			}
		} else {
			m_nErrorMask |= MAT_STRING_ERROR_FLAG_ID;
		}

		// erase the 'id' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
	}
	// look for *collid command
	pszCmd = strstr( pszData, "*collid" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		// see if there is an collid param
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%d", &nParam ) == 1 ) {
				FMATH_CLAMP( nParam, 1, 63 );
				m_ApeCommands.nCollID = nParam;				
			} else {
				m_nErrorMask |= MAT_STRING_ERROR_FLAG_COLL_ID;
			}
		} else {
			m_nErrorMask |= MAT_STRING_ERROR_FLAG_COLL_ID;
		}

		// erase the 'collid' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
		pszCmd[5] = '_';
		pszCmd[6] = '_';
	}
	// look for *sort command
	pszCmd = strstr( pszData, "*sort" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		// no params
		m_ApeCommands.bSort = TRUE;	

		// erase the 'sort' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';		
	}
	// look for *order command
	pszCmd = strstr( pszData, "*order" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		m_ApeCommands.nOrderNum = 1;
		// see if there is a depth param
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%d", &nParam ) == 1 ) {
				FMATH_CLAMP( nParam, 1, 100 );
				m_ApeCommands.nOrderNum = nParam;
			}
		}

		// erase the 'order' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
		pszCmd[5] = '_';		
	}
	// look for *shader command
	pszCmd = strstr( pszData, "*shader" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		// there must be shader type
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%d", &nParam ) == 1 ) {
				FMATH_CLAMPMIN( nParam, 0 );
				m_ApeCommands.nShaderNum = nParam;
			} else {
				m_nErrorMask |= MAT_STRING_ERROR_FLAG_SHADER;
			}
		} else {
			m_nErrorMask |= MAT_STRING_ERROR_FLAG_SHADER;
		}

		// erase the 'shader' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
		pszCmd[5] = '_';
		pszCmd[6] = '_';
	}
	// look for *motif command
	pszCmd = strstr( pszData, "*motif" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		// there must be 6 parameters (EmotifID,UseEColor,DmotifID,UseDColor,SmotifID,UseSColor)
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%d,%d,%d,%d,%d,%d", &nParam, &nParam2, &nParam3, &nParam4, &nParam5, &nParam6 ) == 6 ) {
				if( nParam >= 0 && nParam3 >= 0 && nParam5 >= 0 ) {
					m_ApeCommands.nEmissiveMotifID = nParam;
					m_ApeCommands.nDiffuseMotifID = nParam3;
					m_ApeCommands.nSpecularMotifID = nParam5;
					m_ApeCommands.bUseEmissiveColor = (nParam2) ? TRUE : FALSE;
					m_ApeCommands.bUseDiffuseColor = (nParam4) ? TRUE : FALSE;
					m_ApeCommands.bUseSpecularColor = (nParam6) ? TRUE : FALSE;				
				} else {
					m_nErrorMask |= MAT_STRING_ERROR_FLAG_MOTIF;
				}
			} else {
				m_nErrorMask |= MAT_STRING_ERROR_FLAG_MOTIF;
			}
		} else {
			m_nErrorMask |= MAT_STRING_ERROR_FLAG_MOTIF;
		}

		// erase the 'motif' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
		pszCmd[5] = '_';		
	}
	// look for *anim command
	pszCmd = strstr( pszData, "*anim" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		// there must be 2 parameters (NumFrames, FramesPerSec)
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%d,%f", &nParam, &fParam ) == 2 ) {
				if( nParam > 0 && fParam > 0.0f ) {
					m_ApeCommands.nNumTexFrames = nParam;
					m_ApeCommands.fFramesPerSecs = fParam;
					// see if we need to assign an auto id
					if( m_ApeCommands.nID == 255 ) {
						m_ApeCommands.nID = _AUTO_ID;
					}
				} else {
					m_nErrorMask |= MAT_STRING_ERROR_FLAG_ANIM;	
				}
			} else {
				m_nErrorMask |= MAT_STRING_ERROR_FLAG_ANIM;
			}
		} else {
			m_nErrorMask |= MAT_STRING_ERROR_FLAG_ANIM;
		}

		// erase the 'anim' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';		
	}
	// look for *rotate command
	pszCmd = strstr( pszData, "*rotate" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		// there must be 3 parameters ( degrees per second, u, v )
		if( _GetParameterString( pszCmd, sParam ) ) 
		{
			if ( sscanf( sParam.data(), "%f,%f,%f", &fParam, &fParam2, &fParam3 ) == 3 )
			{
				if ( fParam2 > 1.f || fParam2 < 0.f || fParam3 > 1.f || fParam3 < 0.f )
				{
					m_nErrorMask |= MAT_STRING_ERROR_FLAG_SCROLL;
				}
				else if( fParam != 0.0f ) 
				{
					m_ApeCommands.fDeltaUVRotationPerSec = fParam;
					m_ApeCommands.vRotateUVAround.Set( fParam2, fParam3 );

					// if we set the scroll command then see if we need to assign an auto id
					if( m_ApeCommands.nID == 255 ) 
					{
						// assign an auto id to this layer
						m_ApeCommands.nID = _AUTO_ID;
					}
				}
			} 
			else 
			{
				m_nErrorMask |= MAT_STRING_ERROR_FLAG_SCROLL;
			}
		} else {
			m_nErrorMask |= MAT_STRING_ERROR_FLAG_SCROLL;
		}

		// erase the 'scroll' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
		pszCmd[5] = '_';
		pszCmd[6] = '_';
	}
	// look for *scroll command
	pszCmd = strstr( pszData, "*scroll" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		// there must be 4 parameters (deltaU, USecs, deltaV, VSecs, deltaRotSec )
		if( _GetParameterString( pszCmd, sParam ) ) 
		{
			if ( sscanf( sParam.data(), "%f,%f,%f,%f", &fParam, &fParam2, &fParam3, &fParam4 ) == 4 )
			{
				if( fParam != 0.0f && fParam2 > 0.0f ) 
				{
					m_ApeCommands.fDeltaUPerSec = fParam/fParam2;
				}
				if( fParam3 != 0.0f && fParam4 > 0.0f ) 
				{
					m_ApeCommands.fDeltaVPerSec = fParam3/fParam4;
				}

				// if we set the scroll command then see if we need to assign an auto id
				if( m_ApeCommands.fDeltaUPerSec != 0.f || m_ApeCommands.fDeltaVPerSec != 0.f ) 
				{
					if( m_ApeCommands.nID == 255 ) 
					{
						// assign an auto id to this layer
						m_ApeCommands.nID = _AUTO_ID;
					}
				}
			} 
			else 
			{
				m_nErrorMask |= MAT_STRING_ERROR_FLAG_SCROLL;
			}
		} else {
			m_nErrorMask |= MAT_STRING_ERROR_FLAG_SCROLL;
		}

		// erase the 'scroll' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
		pszCmd[5] = '_';
		pszCmd[6] = '_';
	}
	// look for *z command
	pszCmd = strstr( pszData, "*z" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		m_ApeCommands.nZTugValue = 1;	
		// see if there is a depth param
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%d", &nParam ) == 1 ) {
				FMATH_CLAMP( nParam, 1, 1000 );
				m_ApeCommands.nZTugValue = nParam;
			}				
		}

		// erase the 'z' to avoid conflicting with other commands
		pszCmd[1] = '_';		
	}
	// look for *nocoll command
	pszCmd = strstr( pszData, "*nocoll" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		// no params
		m_ApeCommands.bNoColl = TRUE;

		// erase the 'nocoll' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
		pszCmd[5] = '_';
		pszCmd[6] = '_';
	}
	// look for the *coll command
	pszCmd = strstr( pszData, "*coll" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		// there must be 8 parameters (player,NPCs,Line of Sight,Thin Projectiles,Thick Projectitles,Camera,Objects,Walkable)
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &nParam, &nParam2, &nParam3, &nParam4, 
																  &nParam5, &nParam6, &nParam7, &nParam8, &nParam9, 
																  &nParam10, &nParam11, &nParam12 ) == 12 ) {
				if( nParam ) {
					m_ApeCommands.nCollMask |= APE_MAT_COLL_FLAGS_COLL_WITH_PLAYER;
				} else {
					m_ApeCommands.nCollMask &= ~APE_MAT_COLL_FLAGS_COLL_WITH_PLAYER;
				}
				if( nParam2 ) {
					m_ApeCommands.nCollMask |= APE_MAT_COLL_FLAGS_COLL_WITH_NPCS;
				} else {
					m_ApeCommands.nCollMask &= ~APE_MAT_COLL_FLAGS_COLL_WITH_NPCS;
				}
				if( nParam3 ) {
					m_ApeCommands.nCollMask |= APE_MAT_COLL_FLAGS_OBSTRUCT_LINE_OF_SIGHT;
				} else {
					m_ApeCommands.nCollMask &= ~APE_MAT_COLL_FLAGS_OBSTRUCT_LINE_OF_SIGHT;
				}
				if( nParam4 ) {
					m_ApeCommands.nCollMask |= APE_MAT_COLL_FLAGS_COLL_WITH_THIN_PROJECTILES;
				} else {
					m_ApeCommands.nCollMask &= ~APE_MAT_COLL_FLAGS_COLL_WITH_THIN_PROJECTILES;
				}
				if( nParam5 ) {
					m_ApeCommands.nCollMask |= APE_MAT_COLL_FLAGS_COLL_WITH_THICK_PROJECTILTES;
				} else {
					m_ApeCommands.nCollMask &= ~APE_MAT_COLL_FLAGS_COLL_WITH_THICK_PROJECTILTES;
				}
				if( nParam6 ) {
					m_ApeCommands.nCollMask |= APE_MAT_COLL_FLAGS_COLL_WITH_CAMERA;
				} else {
					m_ApeCommands.nCollMask &= ~APE_MAT_COLL_FLAGS_COLL_WITH_CAMERA;
				}
				if( nParam7 ) {
					m_ApeCommands.nCollMask |= APE_MAT_COLL_FLAGS_COLL_WITH_OBJECTS;
				} else {
					m_ApeCommands.nCollMask &= ~APE_MAT_COLL_FLAGS_COLL_WITH_OBJECTS;
				}
				if( nParam8 ) {
					m_ApeCommands.nCollMask |= APE_MAT_COLL_FLAGS_WALKABLE;
				} else {
					m_ApeCommands.nCollMask &= ~APE_MAT_COLL_FLAGS_WALKABLE;
				}				
				if( nParam9 ) {
					m_ApeCommands.nCollMask |= APE_MAT_COLL_FLAGS_OBSTRUCT_SPLASH_DAMAGE;
				} else {
					m_ApeCommands.nCollMask &= ~APE_MAT_COLL_FLAGS_OBSTRUCT_SPLASH_DAMAGE;
				}
				if( nParam10 ) {
					m_ApeCommands.nCollMask |= APE_MAT_COLL_FLAGS_COLLIDE_WITH_DEBRIS;
				} else {
					m_ApeCommands.nCollMask &= ~APE_MAT_COLL_FLAGS_COLLIDE_WITH_DEBRIS;
				}				
				if( nParam11 ) {
					m_ApeCommands.nCollMask |= APE_MAT_COLL_FLAGS_COLLIDE_WITH_VEHICLES;
				} else {
					m_ApeCommands.nCollMask &= ~APE_MAT_COLL_FLAGS_COLLIDE_WITH_VEHICLES;
				}
				if( nParam12 ) {
					m_ApeCommands.nCollMask |= APE_MAT_COLL_FLAGS_HOVER_COLLIDABLE;
				} else {
					m_ApeCommands.nCollMask &= ~APE_MAT_COLL_FLAGS_HOVER_COLLIDABLE;
				}
			} else {
				m_nErrorMask |= MAT_STRING_ERROR_FLAG_COLL;
			}
		} else {
			m_nErrorMask |= MAT_STRING_ERROR_FLAG_COLL;
		}

		// erase the 'coll' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
	}
	// look for *noascroll command
	pszCmd = strstr( pszData, "*noascroll" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		m_nMatFlags |= APE_LAYER_FLAGS_NO_ALPHA_SCROLL;
		// erase the 'noascroll' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
		pszCmd[5] = '_';
		pszCmd[6] = '_';
		pszCmd[7] = '_';
		pszCmd[8] = '_';
	}
	// look for *tint command
	pszCmd = strstr( pszData, "*tint" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		m_ApeCommands.nFlags |= APE_MAT_FLAGS_APPLY_TINT;
		// erase the 'tint' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';		
	}
	// look for *writez command
	pszCmd = strstr( pszData, "*writez" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		m_ApeCommands.nFlags |= APE_MAT_FLAGS_ZWRITE_ON;
		// erase the command to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';		
		pszCmd[5] = '_';
		pszCmd[6] = '_';
	}
	// look for *notint command
	pszCmd = strstr( pszData, "*nomeshtint" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		m_ApeCommands.nFlags |= APE_MAT_FLAGS_DO_NOT_TINT;
		// erase the command to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';		
		pszCmd[5] = '_';
		pszCmd[6] = '_';
	}
	// look for *bumptile command
	pszCmd = strstr( pszData, "*bumptile" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%f", &fParam ) == 1 ) {
				m_ApeCommands.fBumpMapTileFactor = fParam;
			}
		}
		// erase the command to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';		
		pszCmd[5] = '_';		
		pszCmd[6] = '_';
		pszCmd[7] = '_';		
		pszCmd[8] = '_';		
	}
	// look for *detailtile command
	pszCmd = strstr( pszData, "*detailtile" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%f", &fParam ) == 1 ) {
				m_ApeCommands.fDetailMapTileFactor = fParam;
			}
		}
		// erase the command to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';		
		pszCmd[5] = '_';		
		pszCmd[6] = '_';
		pszCmd[7] = '_';		
		pszCmd[8] = '_';		
		pszCmd[8] = '_';		
		pszCmd[9] = '_';		
	}
	// look for *light command
	pszCmd = strstr( pszData, "*light" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%f,%f,%f,%f", &fParam, &fParam2, &fParam3, &fParam4 ) == 4 ) {
				FMATH_CLAMP( fParam, 0.f, 255.f );
				FMATH_CLAMP( fParam2, 0.f, 255.f );
				FMATH_CLAMP( fParam3, 0.f, 255.f );
				FMATH_CLAMP( fParam4, 0.f, 255.f );
				m_ApeCommands.LightRGBI.Set( fParam / 255.f, fParam2 / 255.f, fParam3 / 255.f, fParam4 / 255.f );
			}
		}
		// erase the command to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';		
		pszCmd[5] = '_';		
	}
	// look for *nodraw command
	pszCmd = strstr( pszData, "*nodraw" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		// no params
		m_ApeCommands.nFlags |= APE_MAT_FLAGS_NO_DRAW;

		// erase the 'nodraw' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
		pszCmd[5] = '_';
		pszCmd[6] = '_';
	}
	// look for *notinlm command
	pszCmd = strstr( pszData, "*notinlm" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		// no params
		m_ApeCommands.nFlags |= APE_MAT_FLAGS_DO_NOT_LM;

		// erase the 'notinlm' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
		pszCmd[5] = '_';
		pszCmd[6] = '_';
		pszCmd[7] = '_';
	}
	// look for *nolmblock command
	pszCmd = strstr( pszData, "*nolmblock" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		// no params
		m_ApeCommands.nFlags |= APE_MAT_FLAGS_DO_NOT_BLOCK_LM;

		// erase the 'nolmblock' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
		pszCmd[5] = '_';
		pszCmd[6] = '_';
		pszCmd[7] = '_';
		pszCmd[8] = '_';
		pszCmd[9] = '_';
	}
	// look for *nolmuse command
	pszCmd = strstr( pszData, "*nolmuse" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		// no params
		m_ApeCommands.nFlags |= APE_MAT_FLAGS_NO_LM_USE;

		// erase the 'nolmuse' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
		pszCmd[5] = '_';
		pszCmd[6] = '_';
		pszCmd[7] = '_';
	}
	// look for *vertrad command
	pszCmd = strstr( pszData, "*vertrad" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		// no params
		m_ApeCommands.nFlags |= APE_MAT_FLAGS_VERT_RADIOSITY|APE_MAT_FLAGS_DO_NOT_LM;

		// erase the 'vertrad' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
		pszCmd[5] = '_';
		pszCmd[6] = '_';
		pszCmd[7] = '_';
	}
	// look for *noshadows command
	pszCmd = strstr( pszData, "*noshadows" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		// no params
		m_nMatFlags |= APE_LAYER_FLAGS_DO_NOT_CAST_SHADOWS;

		// erase the 'noshadows' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
		pszCmd[5] = '_';
		pszCmd[6] = '_';
		pszCmd[7] = '_';
		pszCmd[8] = '_';
		pszCmd[9] = '_';
		pszCmd[10] = '_';
	}
	// look for *eangle command
	pszCmd = strstr( pszData, "*eangle" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%d", &nParam ) == 1 ) {
				FMATH_CLAMP( nParam, 0, 180 );
				m_nAffectAngle = nParam;
				m_nMatFlags |= APE_LAYER_FLAGS_ANGULAR_EMISSIVE;
			}
		}
		// erase the 'eangle' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
		pszCmd[5] = '_';
		pszCmd[6] = '_';
	}
	// look for *eangle command
	pszCmd = strstr( pszData, "*tangle" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%d", &nParam ) == 1 ) {
				FMATH_CLAMP( nParam, 0, 180 );
				m_nAffectAngle = nParam;
				m_nMatFlags |= APE_LAYER_FLAGS_ANGULAR_TRANSLUCENCY;
			}
		}
		// erase the 'tangle' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
		pszCmd[5] = '_';
		pszCmd[6] = '_';
	}
	// look for *surf command
	pszCmd = strstr( pszData, "*surf" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		
		// there must be surface type
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%d", &nParam ) == 1 ) {
				FMATH_CLAMP( nParam, 0, 15 );
				m_ApeCommands.nSurfaceType = nParam;
			} else {
				m_nErrorMask |= MAT_STRING_ERROR_FLAG_SURF;
			}
		} else {
			m_nErrorMask |= MAT_STRING_ERROR_FLAG_SURF;
		}

		// erase the 'surf' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
	}
	// look for *react command
	pszCmd = strstr( pszData, "*react" );
	if( pszCmd ) {
		nNumValidStarCommands++;
		
		// there must be reaction type
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%d", &nParam ) == 1 ) {
				FMATH_CLAMP( nParam, 0, 7 );
				m_ApeCommands.nReactType = nParam;
			} else {
				m_nErrorMask |= MAT_STRING_ERROR_FLAG_REACT;
			}
		} else {
			m_nErrorMask |= MAT_STRING_ERROR_FLAG_REACT;
		}

		// erase the 'react' to avoid conflicting with other commands
		pszCmd[1] = '_';
		pszCmd[2] = '_';
		pszCmd[3] = '_';
		pszCmd[4] = '_';
		pszCmd[5] = '_';
	}

	// count the number of star commands in the whole string and
	// see if that number matches our valid number
	u32 nNumStars = 0;
	pszCmd = strchr( pszData, '*' );
	while( pszCmd ) {
		if( pszCmd ) {
			nNumStars++;
			pszCmd++;
		}
		pszCmd = strchr( pszCmd, '*' );		
	}
	if( nNumStars != nNumValidStarCommands ) {
		m_nErrorMask |= MAT_STRING_ERROR_FLAG_UNKNOWN_CMD;
	}
}

void CMatStringParser::ResetToDefaults() {
	
	m_sOrigMatString.Resize( 0 );
	m_nErrorMask = MAT_STRING_ERROR_FLAG_NONE;
	ZeroMemory( &m_ApeCommands, sizeof( ApeCommands_t ) );
	m_nMatFlags = APE_LAYER_FLAGS_NONE;
	m_nAffectAngle = 0;
	m_ApeCommands.bSort = FALSE;
	m_ApeCommands.nOrderNum = 0;
	m_ApeCommands.nShaderNum = -1;
	m_ApeCommands.nEmissiveMotifID = 0;
	m_ApeCommands.nSpecularMotifID = 0;
	m_ApeCommands.nDiffuseMotifID = 0;
	m_ApeCommands.bUseEmissiveColor = TRUE;
	m_ApeCommands.bUseSpecularColor = TRUE;
	m_ApeCommands.bUseDiffuseColor = FALSE;
	m_ApeCommands.nNumTexFrames = 0;
	m_ApeCommands.fFramesPerSecs = 0.0f;
	m_ApeCommands.fDeltaUPerSec = 0.0f;
	m_ApeCommands.fDeltaVPerSec = 0.0f;
	m_ApeCommands.fDeltaUVRotationPerSec = 0.f;
	m_ApeCommands.nZTugValue = 0;
	m_ApeCommands.nID = 255;
	m_ApeCommands.bNoColl = FALSE;
	m_ApeCommands.nCollID = 0;

	// NEW
	m_ApeCommands.nFlags = APE_MAT_FLAGS_NONE;
	m_ApeCommands.nCollMask = APE_MAT_COLL_FLAGS_COLL_WITH_EVERYTHING;
	m_ApeCommands.nReactType = 0;
	m_ApeCommands.nSurfaceType = 0;
	m_ApeCommands.TintRGB.Set( 1.f, 1.f, 1.f );
	m_ApeCommands.LightRGBI.Set( 0.f, 0.f, 0.f, 0.f );
	m_ApeCommands.fBumpMapTileFactor = 1.f;
	m_ApeCommands.fDetailMapTileFactor = 4.f;
}

u32 CMatStringParser::GetNumberOfErrors() {
	return fmath_CountBits( m_nErrorMask );
}

BOOL CMatStringParser::GetErrorString( u32 nErrorIndex, CStr &rsErrorString ) {
	if( nErrorIndex >= fmath_CountBits( m_nErrorMask ) ) {
		return FALSE;
	}
	u32 i, uCurIndex;
	i = uCurIndex = 0;
	while( _aErrorCodes[i].nErrorMask != MAT_STRING_ERROR_FLAG_NONE ) {
		if( m_nErrorMask & _aErrorCodes[i].nErrorMask ) {
			if( uCurIndex == nErrorIndex ) {
				rsErrorString = _aErrorCodes[i].pszErrorString;
				return TRUE;
			}
			uCurIndex++;
		}
		i++;
	}
	return FALSE;
}

///////////////////////
// OBJECT STRING PARSER
///////////////////////

CObjStringParser::CObjStringParser() {
	ResetToDefaults();
}

CObjStringParser::~CObjStringParser() {
}

void CObjStringParser::Parse( cchar *pszObjString ) {
	CStr sLowerString, sParam;
	char *pszData, *pszCmd;
	f32 fParam0, fParam1, fParam2;
	
	ResetToDefaults();

	if( !pszObjString ) {
		return;
	}
	m_sOrigString = pszObjString;
	sLowerString = m_sOrigString;
	sLowerString.toLower();
	pszData = sLowerString.data();
	// look for *postery command
	pszCmd = strstr( pszData, "*postery" );
	if( pszCmd ) {
		m_nCommands |= APE_OB_FLAG_POSTER_Y;	
	}
	// look for *posterx command
	pszCmd = strstr( pszData, "*posterx" );
	if( pszCmd ) {
		m_nCommands |= APE_OB_FLAG_POSTER_X;	
	}
	// look for *posterz command
	pszCmd = strstr( pszData, "*posterz" );
	if( pszCmd ) {
		m_nCommands |= APE_OB_FLAG_POSTER_Z;	
	}
	// look for *nocoll command
	pszCmd = strstr( pszData, "*nocoll" );
	if( pszCmd ) {
		m_nCommands |= APE_OB_FLAG_NO_COLL;	
	}
	// look for *nofog command
	pszCmd = strstr( pszData, "*nofog" );
	if( pszCmd ) {
		// This is a legacy command that has no function
//		m_nCommands |= APE_OB_FLAG_NO_FOG;	
	}
	// look for *nolight command
	pszCmd = strstr( pszData, "*nolight" );
	if( pszCmd ) {
		m_nCommands |= APE_OB_FLAG_NO_LIGHT;	
	}
	// look for *culldist command
	pszCmd = strstr( pszData, "*culldist" );
	if( pszCmd ) {
		// there must be value specified
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%f", &fParam0 ) == 1 ) {
				FMATH_CLAMPMIN( fParam0, 1.0f );
				m_fCullDist = fParam0;
			}
		}
	}
	pszCmd = strstr( pszData, "*tint" );
	if( pszCmd ) {
		// there must be value specified
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%f,%f,%f", &fParam0, &fParam1, &fParam2 ) == 3 ) {
				FMATH_CLAMP( fParam0, 0.f, 255.f );
				FMATH_CLAMP( fParam1, 0.f, 255.f );
				FMATH_CLAMP( fParam2, 0.f, 255.f );
				m_TintRGB.Set( fParam0 / 255.f, fParam1 / 255.f, fParam2 / 255.f );
				m_nCommands |= APE_OB_FLAG_TINT;
			}
		}
	}
	// look for the *sort command
	pszCmd = strstr( pszData, "*sort" );
	if( pszCmd ) {
		// This is a legacy command that has no function
//		m_nCommands |= APE_OB_FLAG_SORT_AFTER_TERRAIN;
	}
	// look for the *nodraw command
	pszCmd = strstr( pszData, "*nodraw" );
	if( pszCmd ) {
		m_nCommands |= APE_OB_FLAG_NO_DRAW;
	}
	// look for the *acceptlm command
	pszCmd = strstr( pszData, "*acceptlm" );
	if( pszCmd ) {
		m_nCommands |= APE_OB_FLAG_LM;
	}
	// look for the *acceptlm command
	pszCmd = strstr( pszData, "*vertrad" );
	if( pszCmd ) {
		m_nCommands |= APE_OB_FLAG_VERT_RADIOSITY;
	}
	// look for the *acceptshadows command
	pszCmd = strstr( pszData, "*acceptshadows" );
	if( pszCmd ) {
		m_nCommands |= APE_OB_FLAG_ACCEPT_SHADOWS;
	}
	// look for the *castshadows command
	pszCmd = strstr( pszData, "*castshadows" );
	if( pszCmd ) {
		m_nCommands |= APE_OB_FLAG_CAST_SHADOWS;
	}
	// look for the *dynamic command
	pszCmd = strstr( pszData, "*dynamic" );
	if( pszCmd ) {
		m_nCommands &= ~(APE_OB_FLAG_STATIC|APE_OB_FLAG_LM|APE_OB_FLAG_VERT_RADIOSITY);
	}
	pszCmd = strstr( pszData, "*nolmuse" );
	if( pszCmd ) {
		m_nCommands |= APE_OB_FLAG_NO_LM_USE;
		m_nCommands &= ~(APE_OB_FLAG_LM|APE_OB_FLAG_VERT_RADIOSITY);
	}
	// look for the *lightperpixel command
	pszCmd = strstr( pszData, "*lightperpixel" );
	if( pszCmd ) {
		m_nCommands |= APE_OB_FLAG_PER_PIXEL;
	}
}

void CObjStringParser::ResetToDefaults() {
	m_sOrigString.Resize( 0 );
	m_nCommands = APE_OB_FLAG_STATIC;// go ahead and set this for all objects
	m_fCullDist = 0.0f;
	m_TintRGB.Set( 0.f, 0.f, 0.f );
}

//////////////////////
// LIGHT STRING PARSER
//////////////////////

CLightStringParser::CLightStringParser( BOOL bWorldFile ) {
	m_bWorldFile = bWorldFile;
	ResetToDefaults();
}

CLightStringParser::~CLightStringParser() {
}

void CLightStringParser::Parse( cchar *pszObjString ) {
	CStr sLowerString, sParam;
	char *pszData, *pszCmd;
	int nParam1, nParam2;
	f32 fParam;
	
	ResetToDefaults();

	if( !pszObjString ) {
		return;
	}
	m_sOrigString = pszObjString;
	sLowerString = m_sOrigString;
	sLowerString.toLower();
	pszData = sLowerString.data();
	// look for *self command
	pszCmd = strstr( pszData, "*self" );
	if( pszCmd ) {
		m_nCommands |= APE_LIGHT_FLAG_LIGHT_SELF;	
	}

	pszCmd = strstr( pszData, "*castshadows" );
	if( pszCmd ) {
		m_nCommands |= APE_LIGHT_FLAG_CAST_SHADOWS;	
	}

	pszCmd = strstr( pszData, "*scalecorona" );
	if( pszCmd ) {
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%f", &fParam ) == 1 ) {
				m_fCoronaScale = fParam;
			}
		}
		else
		{
			m_szCoronaTexture[0] = 0;
		}
	}

	pszCmd = strstr( pszData, "*fadingcorona" );
	if( pszCmd ) {
		m_nCommands |= APE_LIGHT_FLAG_CORONA|APE_LIGHT_FLAG_CORONA_PROXFADE;	
		if( _GetParameterString( pszCmd, sParam ) ) {
			strcpy( m_szCoronaTexture, sParam.data() );
		}
		else
		{
			m_szCoronaTexture[0] = 0;
		}
	}

	pszCmd = strstr( pszData, "*corona" );
	if( pszCmd ) {
		m_nCommands |= APE_LIGHT_FLAG_CORONA;	
		if( _GetParameterString( pszCmd, sParam ) ) {
			strcpy( m_szCoronaTexture, sParam.data() );
		}
		else
		{
			m_szCoronaTexture[0] = 0;
		}
	}

	pszCmd = strstr( pszData, "*perpixel" );
	if( pszCmd ) {
		m_nCommands |= APE_LIGHT_FLAG_PER_PIXEL;	
		if( _GetParameterString( pszCmd, sParam ) ) {
			strcpy( m_szPerPixelTexture, sParam.data() );
		}
		else
		{
			m_szPerPixelTexture[0] = 0;
		}
	}

	pszCmd = strstr( pszData, "*onlyppmesh" );
	if( pszCmd ) {
        // in order to light a mesh per-pixel, the mesh must be flagged to receive per pixel lighting
		m_nCommands |= APE_LIGHT_FLAG_MESH_MUST_BE_PER_PIXEL;	
	}

	// Look for lighting mode flags
	pszCmd = strstr( pszData, "*onlydynamic" );
	if( pszCmd ) {
		m_nCommands |= APE_LIGHT_FLAG_DYNAMIC_ONLY;	
	}

	pszCmd = strstr( pszData, "*lm" );
	if( pszCmd ) {
		m_nCommands &= ~APE_LIGHT_FLAG_DYNAMIC_ONLY;
		m_nCommands |= APE_LIGHT_FLAG_LIGHTMAP_LIGHT;
	}

	pszCmd = strstr( pszData, "*uniquelm" );
	if( pszCmd ) {
		m_nCommands &= ~APE_LIGHT_FLAG_DYNAMIC_ONLY;
		m_nCommands |= APE_LIGHT_FLAG_UNIQUE_LIGHTMAP | APE_LIGHT_FLAG_LIGHTMAP_LIGHT;
	}

	pszCmd = strstr( pszData, "*onlylm" );
	if( pszCmd ) {
		m_nCommands &= ~APE_LIGHT_FLAG_DYNAMIC_ONLY;
		m_nCommands |= APE_LIGHT_FLAG_LIGHTMAP_ONLY_LIGHT|APE_LIGHT_FLAG_LIGHTMAP_LIGHT;
	}

	if( !m_bWorldFile ) {
		// look for *noterrain command
		pszCmd = strstr( pszData, "*noterrain" );
		if( pszCmd ) {
			m_nCommands |= APE_LIGHT_FLAG_OBJ_DONT_LIGHT_TERRAIN;	
		}
	}

	// look for light id
	pszCmd = strstr( pszData, "*id" );
	if( pszCmd ) {
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%d", &nParam1 ) == 1 ) {
				// make sure that we have a valid light id
				if ( nParam1 >= 0 && nParam1 < 0xffff )	{
					m_nLightID = nParam1;
				}
			}
		}
	}

	// look for *motif command
	pszCmd = strstr( pszData, "*motif" );
	if( pszCmd ) {
		// there must be 2 parameters (motifID,UseColor) specified
		if( _GetParameterString( pszCmd, sParam ) ) {
			if( sscanf( sParam.data(), "%d,%d", &nParam1, &nParam2 ) == 2 ) {
				// make sure that we have a valid motif id
				if( nParam1 < 0 ) {
					nParam1 = 0;
				}
				m_nMotifID = nParam1;
				if( !nParam2 ) {
					// don't use the rgb color
					m_nCommands |= APE_LIGHT_FLAG_DONT_USE_RGB;
				}
			}
		}
	}	
}

void CLightStringParser::ResetToDefaults() {
	m_sOrigString.Resize( 0 );
	m_nCommands = APE_LIGHT_FLAG_DYNAMIC_ONLY;// go ahead and set this for all lights
	m_nMotifID = 0;
	m_nLightID = 0xffff;
	m_fCoronaScale = 1.f;
	m_szCoronaTexture[0] = 0;
	m_szPerPixelTexture[0] = 0;
}


///////////////////////
// PORTAL STRING PARSER
///////////////////////

CPortalStringParser::CPortalStringParser() {
	ResetToDefaults();
}

CPortalStringParser::~CPortalStringParser() {
}

void CPortalStringParser::Parse( cchar *pszObjString ) {
	CStr sLowerString, sParam;
	char *pszData, *pszCmd;
	
	ResetToDefaults();

	if( !pszObjString ) {
		return;
	}
	m_sOrigString = pszObjString;
	sLowerString = m_sOrigString;
	sLowerString.toLower();
	pszData = sLowerString.data();

	// look for *mirror command
	pszCmd = strstr( pszData, "*mirror" );
	if( pszCmd ) {
		m_nCommands |= APE_PORTAL_FLAG_MIRROR;	
	}

	// look for *sound command
	pszCmd = strstr( pszData, "*sound" );
	if( pszCmd ) {
		m_nCommands |= APE_PORTAL_FLAG_SOUND_ONLY;	
	}

	// look for *1way command
	pszCmd = strstr( pszData, "*1way" );
	if( pszCmd ) {
		m_nCommands |= APE_PORTAL_FLAG_ONE_WAY;	
	}
	// look for *oneway command
	pszCmd = strstr( pszData, "*oneway" );
	if( pszCmd ) {
		m_nCommands |= APE_PORTAL_FLAG_ONE_WAY;	
	}

	// look for *anti command
	pszCmd = strstr( pszData, "*anti" );
	if( pszCmd ) {
		m_nCommands |= APE_PORTAL_FLAG_ANTI;	
	}	
}

void CPortalStringParser::ResetToDefaults() {
	m_sOrigString.Resize( 0 );
	m_nCommands = APE_PORTAL_FLAG_NONE;// go ahead and set this for all objects
}


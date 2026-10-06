//////////////////////////////////////////////////////////////////////////////////////
// ProTrack.cpp - 
//
// Author: Pat MacKellar 
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
// 05/07/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "ProTrack.h"
#include "ftimer.h"
#include "flinklist.h"
#include "fboxfilter.h"
#include "fclib.h"
#include "fmath.h"
#include "ftext.h"
#include "gamepad.h"
#include "gameloop.h"
#if PROTRACK_ENABLE == 1

const s32 kPROTRACK_NAME_LEN=64;

class CProfile;
void *flinklist_Find( FLinkRoot_t *pLinkRoot, void *pStructure );
typedef BOOL (*IterateFunc)(void* pStructure);
void flinklist_IterateUntil( FLinkRoot_t *pLinkRoot, IterateFunc pFunc);
CProfile* ProfileListFind(FLinkRoot_t* pRoot, cchar* pszName);
void _ProTrack_TreeFree(CProfile* pProfile);

class CProTrackCfg
{
	public:
	CProTrackCfg(void);
	~CProTrackCfg(void);

	enum
	{
		SAMPLEMODE_SINGLE_FRAME = 0,
		SAMPLEMODE_AVG_SHORT,
		SAMPLEMODE_AVG_LONG,
		SAMPLEMODE_AVG_RUN,
		NUM_SAMPLEMODES,
	};
	u16 m_nSampleMode;

	enum
	{
		DISPLAYMODE_TREE = 0,
		DISPLAYMODE_SUMS,
		NUM_DISPLAYMODES,
	};
	u16 m_nDisplayMode;
};

cchar* _aszSampleModes[] =
{
	"SAMP_FRAME",
	"SAMP_AVG_5",
	"SAMP_AVG_30",
	"SAMP_AVG_RUN",
};

cchar* _aszDisplayModes[] = 
{
	"DISP_TREE",
	"DISP_SUMS",
};


FCLASS_NOALIGN_PREFIX class CProfile
{
public:
	BOOL Allocate(void);
	void Init(cchar* pszName, CProfile* pParent);

	inline FLinkRoot_t	*GetChildren(void)		{ return &m_ChildList;};
	void Begin(void);
	void End(void);
	void BeginFrame(void);
	void Reset(void);
	s32 CountDepth(void);

	FLink_t	m_Link;
	CFTimer m_Timer;
	CProfile* m_pParent;
	CProfile* m_pGlobalProfile;
	FLinkRoot_t	m_ChildList;
	char m_aszName[kPROTRACK_NAME_LEN];
	u32	m_uNameCRC;
	f32 m_fTimeThisFrame;
	BOOL m_bChildExpand;
	FBoxFilterHandle_t m_hBoxFilterShort;
	FBoxFilterHandle_t m_hBoxFilterLong;
	u32 m_nNumRunningSamples;
	f32 m_fAvgRun;
	f32 m_fTime;

private:
	CProfile(void) {};
public:
	enum
	{
		DEFAULT_PROFILEBANK_SIZE = 500
	};

	static BOOL InitSystem(s32 nProfileAllocBankCount = DEFAULT_PROFILEBANK_SIZE);
	static void UninitSystem(void);
	static FLinkRoot_t s_GlobalProfileList;
	static CProfile *s_pProTrackRoot;
	static CProfile *s_pCurrent;
	static CProTrackCfg s_ProTrackCfg;
	static CProfile* BankGet(cchar* pszName, CProfile* pParent);
	static void BankRecycle(CProfile* pProfile);

	
	static CProfile *s_aAllocBank;
	static s32 *s_anAllocBankStatus;
	static s32 s_nBankAllocCount;


	FCLASS_STACKMEM_NOALIGN( CProfile );
} FCLASS_NOALIGN_SUFFIX;

CProfile *CProfile::s_pProTrackRoot = NULL;
CProfile *CProfile::s_pCurrent = NULL;
CProTrackCfg CProfile::s_ProTrackCfg;
FLinkRoot_t CProfile::s_GlobalProfileList;
CProfile *CProfile::s_aAllocBank = NULL;
s32 *CProfile::s_anAllocBankStatus = NULL;
s32 CProfile::s_nBankAllocCount = 0;

BOOL CProfile::InitSystem(s32 nProfileAllocBankCount)
{
	s32 i;
	FResFrame_t ResFrame = fres_GetFrame();

	if (s_pProTrackRoot)
	{
		UninitSystem();
	}

	s_aAllocBank = (CProfile *) fres_AllocAndZero(sizeof(CProfile)*nProfileAllocBankCount);
	if (!s_aAllocBank)
	{
		goto _ExitProtrack_InitSysWithError;
	}
	s_anAllocBankStatus = (s32 *) fres_AllocAndZero(sizeof(s32)*nProfileAllocBankCount);
	if (!s_aAllocBank)
	{
		goto _ExitProtrack_InitSysWithError;
	}
	s_nBankAllocCount = nProfileAllocBankCount; 
	for (i= 0; i < s_nBankAllocCount; i++)
	{
		if (!s_aAllocBank[i].Allocate())
		{
		goto _ExitProtrack_InitSysWithError;
		}
	}

	flinklist_InitRoot(&s_GlobalProfileList, (s32)FANG_OFFSETOF( CProfile, m_Link));
	s_pProTrackRoot = BankGet("Root", NULL);

	return TRUE;

_ExitProtrack_InitSysWithError:

	s_pProTrackRoot = NULL;
	s_aAllocBank = NULL;
	s_anAllocBankStatus = NULL;
	s_nBankAllocCount = NULL;
	return FALSE;
}


void CProfile::UninitSystem(void)
{
	_ProTrack_TreeFree(s_pProTrackRoot);
	s_pProTrackRoot = NULL;
	//free some memory or something
	while (CProfile* pProfile = (CProfile*) flinklist_RemoveHead(&s_GlobalProfileList))
	{
		BankRecycle(pProfile);
	}
	s_pCurrent = NULL;


	//Could assert that all have been returned to bank?
	for (s32 i= 0; i < s_nBankAllocCount; i++)
	{
		FASSERT(s_anAllocBankStatus[i] == 0);// 1 would mean it wasn't returned to the bank.
	}
	s_aAllocBank = NULL;
	s_anAllocBankStatus = NULL;
}


CProfile* CProfile::BankGet(cchar* pszName, CProfile* pParent)
{
	for (s32 i= 0; i < s_nBankAllocCount; i++)
	{
		if (s_anAllocBankStatus[i] == 0)
		{
			s_anAllocBankStatus[i] = 1;
			CProfile* pProfile = s_aAllocBank + i;
			pProfile->Init(pszName,pParent);
			return (s_aAllocBank + i);
		}
	}
	return NULL;
}


void CProfile::BankRecycle(CProfile* pProfile)
{
	s32 i = pProfile - s_aAllocBank;

	FASSERT(i >=0 && i < s_nBankAllocCount);
	FASSERT(s_anAllocBankStatus[i] == 1 );	//better be checked out
	s_anAllocBankStatus[i] = 0;
}


BOOL CProfile::Allocate(void)
{
	m_hBoxFilterShort = fboxfilter_Create_f32( 5 );		 //these are now frames, wish the were in seconds or something...
	m_hBoxFilterLong =  fboxfilter_Create_f32( 30 );
	return (m_hBoxFilterShort && m_hBoxFilterLong);
}


void CProfile::Init(cchar* pszName, CProfile* pParent)
{
	m_pGlobalProfile = NULL;
	m_fTimeThisFrame = 0.0f;
	m_fTime = 0.0f;
	m_pParent = pParent;
	m_uNameCRC = 0;
	m_bChildExpand = 1;

	s32 nNameLen = fclib_strlen(pszName);
	FASSERT(pszName && nNameLen < kPROTRACK_NAME_LEN-1);
	fclib_strcpy(m_aszName, pszName);
	m_uNameCRC = fmath_Crc32(0, (u8*) pszName, nNameLen);

	m_Timer.Reset();

	flinklist_InitRoot(GetChildren(), (s32)FANG_OFFSETOF( CProfile, m_Link ));

	if (m_pParent)
	{
		flinklist_AddTail(m_pParent->GetChildren(), this);
	}

	m_pGlobalProfile = ProfileListFind(&s_GlobalProfileList, m_aszName);

	m_fAvgRun = 0.0f;
	m_nNumRunningSamples = 0;
}


s32 CProfile::CountDepth(void)
{
	CProfile* pTmp = this;
	s32 nCount = 0;
	while (pTmp->m_pParent)
	{
		nCount++;
		pTmp = pTmp->m_pParent;
	}
	return nCount;
}


void CProfile::Reset(void)
{
	m_fTime = 0.0f;
	m_fTimeThisFrame = 0.0f;
	fboxfilter_Reset_f32(m_hBoxFilterShort);
	fboxfilter_Reset_f32(m_hBoxFilterLong);
	m_fAvgRun = 0.0f;
	m_nNumRunningSamples = 0;
}


void CProfile::BeginFrame(void)
{
	if (CProfile::s_ProTrackCfg.m_nSampleMode == CProTrackCfg::SAMPLEMODE_SINGLE_FRAME)
	{
		m_fTime = m_fTimeThisFrame;
	}
	else if (CProfile::s_ProTrackCfg.m_nSampleMode == CProTrackCfg::SAMPLEMODE_AVG_SHORT)
	{
		fboxfilter_Add_f32( m_hBoxFilterShort, m_fTimeThisFrame);
		fboxfilter_Get_f32(m_hBoxFilterShort, NULL, &m_fTime, NULL, NULL);
	}
	else if (CProfile::s_ProTrackCfg.m_nSampleMode == CProTrackCfg::SAMPLEMODE_AVG_LONG)
	{
		fboxfilter_Add_f32( m_hBoxFilterLong, m_fTimeThisFrame);
		fboxfilter_Get_f32(m_hBoxFilterLong, NULL, &m_fTime, NULL, NULL);
	}
	else if (CProfile::s_ProTrackCfg.m_nSampleMode == CProTrackCfg::SAMPLEMODE_AVG_RUN)
	{
		f32 fSum = m_fAvgRun*m_nNumRunningSamples;
		fSum+=m_fTimeThisFrame;
		m_nNumRunningSamples++;
		if (m_nNumRunningSamples<=0)
			m_nNumRunningSamples = 1;
		m_fAvgRun = fSum/m_nNumRunningSamples;
		m_fTime = m_fAvgRun;
	}
	m_fTimeThisFrame = 0.0f;
}


void CProfile::Begin(void)
{
	m_Timer.Reset();
}


void CProfile::End(void)
{
	f32 fTimeThisBlock = m_Timer.SampleSeconds(FALSE)*1000.0f;
	m_fTimeThisFrame += fTimeThisBlock;

	//add this time to my global profile
	if (m_pGlobalProfile)
	{
		m_pGlobalProfile->m_fTimeThisFrame += fTimeThisBlock;
	}
}



CProTrackCfg::CProTrackCfg(void) 
: m_nSampleMode(SAMPLEMODE_SINGLE_FRAME),
  m_nDisplayMode(DISPLAYMODE_TREE)
{

}


CProTrackCfg::~CProTrackCfg(void)
{
}










//
//
//
//
//
//	
static f32 _fMenuX = 0.1f;
static f32 _fMenuY = 0.5f;
static f32 _fMenuIndentSize = 0.04f;
static f32 _fMenuLineHeight = 0.02f;
static s32 _nMenuDisplayCurLine = 0;
static s32 _nMenuDisplayCurItem = 0;
static s32 _nMenuDisplayTopLine = 0;
static s32 _nMenuDisplayNumLines = 7;
static s32 _nMenuDisplayTotalLines = 0;
static s32 _nMenuLineCounter = 0;				  //don't change this it is used while running throught the list
CProfile* _pMenuCurProfile = NULL;
static s32 _bProTrackMenuOn = 0;
#if FANG_PLATFORM_WIN
	static s32 _nProTrackControllerId = 0;
#else 
	static s32 _nProTrackControllerId = 1;
#endif
 

BOOL PROTRACK_INITSYSTEM(void)
{
	BOOL rVal = CProfile::InitSystem();
	_nMenuDisplayCurItem = 0;
	_nMenuDisplayTopLine = 0;
	_nMenuDisplayTotalLines = 0;
	return rVal;
}


void PROTRACK_UNINITSYSTEM(void)
{
	CProfile::UninitSystem();
}


void PROTRACK_BEGINBLOCK(cchar *pszName)
{
	if (!CProfile::s_pProTrackRoot)
		return;

	CProfile* pProfile = NULL;

	if (!CProfile::s_pCurrent)
	{
		CProfile::s_pCurrent = CProfile::s_pProTrackRoot;
	}

	if (!CProfile::s_pCurrent ||
		!(pProfile = ProfileListFind(CProfile::s_pCurrent->GetChildren(), pszName)))
	{
		FASSERT(pProfile == NULL);
		pProfile = CProfile::BankGet(pszName, CProfile::s_pCurrent);

		if (!pProfile->m_pGlobalProfile)
		{
			pProfile->m_pGlobalProfile = CProfile::BankGet(pszName, NULL);
			FASSERT(pProfile->m_pGlobalProfile);
			flinklist_AddTail(&CProfile::s_GlobalProfileList, pProfile->m_pGlobalProfile);
		}
	}

	FASSERT(pProfile);
	CProfile::s_pCurrent = pProfile;

	CProfile::s_pCurrent->Begin();
}


void PROTRACK_ENDBLOCK(void)
{
	if (!CProfile::s_pProTrackRoot)
		return;
	CProfile::s_pCurrent->End();
	CProfile::s_pCurrent = CProfile::s_pCurrent->m_pParent;
}

enum
{
	CONTINUE_SEARCH = 0,
	STOP_DEPTH_RECURSION = 1,
	QUIT_SEARCH = 2
};
typedef s32 (*TreeCallBackFunc)(CProfile* pProfile);


s32 _ProTrack_TreeRecurse(CProfile* pProfile, TreeCallBackFunc pFunc)
{
	s32 nControl = CONTINUE_SEARCH;
	if (pProfile)
	{
		s32 rVal = (*pFunc)(pProfile);

		switch (rVal)
		{
			case CONTINUE_SEARCH:
				{
					CProfile* pChild = (CProfile*) flinklist_GetHead(pProfile->GetChildren());
					while (pChild)
					{
						if (_ProTrack_TreeRecurse(pChild, pFunc) == QUIT_SEARCH)
						{
							return QUIT_SEARCH;
						}

						pChild = (CProfile*) flinklist_GetNext(pProfile->GetChildren(), pChild);
					}
				}
				break;
			case STOP_DEPTH_RECURSION:
				break;
			case QUIT_SEARCH:
				nControl = QUIT_SEARCH;
				break;


		}
	}
	return nControl;
}


void _ProTrack_TreeFree(CProfile* pProfile)
{
	if (pProfile)
	{
		while (CProfile* pChild = (CProfile*) flinklist_RemoveHead(pProfile->GetChildren()))
		{
			_ProTrack_TreeFree(pChild);
		}

		CProfile::BankRecycle(pProfile); pProfile = NULL;
	}
}

s32 _ProfileBeginFrameCallBack(CProfile* pProfile)
{
	FASSERT(pProfile);
	pProfile->BeginFrame();
	return CONTINUE_SEARCH;
}



s32 _ProfileResetCallBack(CProfile* pProfile)
{
	FASSERT(pProfile);
	pProfile->Reset();
	return CONTINUE_SEARCH;
}



CProfile* _pLastDrawnParentProfile = NULL;
s32 _ProfileDisplayTreeItem(CProfile* pProfile)
{
	FASSERT(pProfile);

	if (pProfile)
	{
		_pLastDrawnParentProfile  = pProfile->m_pParent;

		s32 nIndentLevel = pProfile->CountDepth() - 1;
		if (nIndentLevel == -1)
		{	//ROOT
			ftext_DebugPrintf(	_fMenuX,
							_fMenuY,
							"~w1PROTRACK: %s, %s",
							_aszDisplayModes[CProfile::s_ProTrackCfg.m_nDisplayMode],
							_aszSampleModes[CProfile::s_ProTrackCfg.m_nSampleMode]);
		}
		else
		{
			_nMenuDisplayTotalLines++;
			if (_nMenuDisplayCurLine < _nMenuDisplayTopLine || 
				_nMenuDisplayCurLine >= (_nMenuDisplayTopLine+_nMenuDisplayNumLines))
			{
				_nMenuDisplayCurLine++;

				if (!pProfile->m_bChildExpand)
				{
					return STOP_DEPTH_RECURSION;
				}

				return CONTINUE_SEARCH;
			}

			_nMenuDisplayCurLine++;

			char szFormat[35] = "~w1~c99999999%2.1f >%s";
			if (pProfile->GetChildren()->nCount ==0)
			{
				szFormat[19] = ' ';
			}
			else if (pProfile->m_bChildExpand)
			{
				szFormat[19] = '+';
			}

			if (_nMenuDisplayCurItem == _nMenuDisplayCurLine-1)
			{
				_pMenuCurProfile = pProfile;
				szFormat[5] = '0';
				szFormat[6] = '0';
				szFormat[9] = '0';
				szFormat[10] = '0';
			}

			ftext_DebugPrintf(	_fMenuX+nIndentLevel*_fMenuIndentSize,
							_fMenuY+(1+_nMenuLineCounter)*_fMenuLineHeight,
							szFormat,
							pProfile->m_fTime,
							pProfile->m_aszName);
			_nMenuLineCounter++;
		}
	}

	if (!pProfile->m_bChildExpand)
	{
		return STOP_DEPTH_RECURSION;
	}
	return CONTINUE_SEARCH;
}


void _ProfileDisplayListItem(CProfile* pProfile)
{
	FASSERT(pProfile);

	_nMenuDisplayTotalLines++;
	if (_nMenuDisplayCurLine < _nMenuDisplayTopLine || 
		_nMenuDisplayCurLine >= (_nMenuDisplayTopLine+_nMenuDisplayNumLines))
	{
		_nMenuDisplayCurLine++;
		return;
	}

	_nMenuDisplayCurLine++;

	if (pProfile)
	{
		char szFormat[35] = "~w1~c99999999%2.1f %s";
		if (_nMenuDisplayCurItem == _nMenuDisplayCurLine-1)
		{
			_pMenuCurProfile = pProfile;
			szFormat[5] = '0';
			szFormat[6] = '0';
			szFormat[9] = '0';
			szFormat[10] = '0';
		}
		ftext_DebugPrintf(	_fMenuX,
						_fMenuY+(1+_nMenuLineCounter)*_fMenuLineHeight,
						szFormat,
						pProfile->m_fTime,
						pProfile->m_aszName);
		_nMenuLineCounter++;
	}
}


void _BeginFrameAll(void)
{
	_ProTrack_TreeRecurse(CProfile::s_pProTrackRoot, _ProfileBeginFrameCallBack);

	//reset everyone int the global profile array
	CProfile* pProfile = (CProfile*) flinklist_GetHead(&CProfile::s_GlobalProfileList);
	while (pProfile)
	{
		pProfile->BeginFrame();

		pProfile = (CProfile*) flinklist_GetNext(&CProfile::s_GlobalProfileList, pProfile);
	}

}


void _ResetAll(void)
{
	_ProTrack_TreeRecurse(CProfile::s_pProTrackRoot, _ProfileResetCallBack);

	//reset everyone int the global profile array
	CProfile* pProfile = (CProfile*) flinklist_GetHead(&CProfile::s_GlobalProfileList);
	while (pProfile)
	{
		pProfile->Reset();

		pProfile = (CProfile*) flinklist_GetNext(&CProfile::s_GlobalProfileList, pProfile);
	}

}

BOOL PROTRACK_ISMENUON(void)
{
	return _bProTrackMenuOn;
}


void PROTRACK_WORK(void)
{
#if FANG_WINGC
	// The debug port shares gameplay controls on PC. Only enable the legacy
	// fire/fire/jump shortcut when the player explicitly enables debug UI.
	if (!Gameloop_bDrawDebugInfo) {
		_bProTrackMenuOn = FALSE;
		return;
	}
#endif
	if (!CProfile::s_pProTrackRoot)
		return;
	
	// this can change depending on which ports the players are on
	_nProTrackControllerId = Gamepad_nDebugPortIndex;

	BOOL bFire1Button = ( Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_FIRE_PRIMARY]->uLatches & FPAD_LATCH_ON );
	BOOL bFire2Button = ( Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_FIRE_SECONDARY]->uLatches & FPAD_LATCH_ON );
	BOOL bSelectFire2Button = ( Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_JUMP]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK );

	if( bFire1Button && bFire2Button && bSelectFire2Button )
	{
		if (_bProTrackMenuOn==FALSE)
		{	//turn on
			 _bProTrackMenuOn=TRUE;
			_ResetAll();
		}
		else
		{
			 _bProTrackMenuOn=FALSE;
		}
	}

	if (!_bProTrackMenuOn)
	{
		return;
	}

	//reset everyone in the global tree
	_BeginFrameAll();
  
	if( Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_SELECT_SECONDARY]->uLatches &
		GAMEPAD_BUTTON_1ST_PRESS_MASK )
	{
		CProfile::s_ProTrackCfg.m_nSampleMode = (CProfile::s_ProTrackCfg.m_nSampleMode+1)%CProTrackCfg::NUM_SAMPLEMODES;
		_ResetAll();
	}
	else if( Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_ACTION]->uLatches &
		GAMEPAD_BUTTON_1ST_PRESS_MASK )
	{
		CProfile::s_ProTrackCfg.m_nDisplayMode = (CProfile::s_ProTrackCfg.m_nDisplayMode+1)%CProTrackCfg::NUM_DISPLAYMODES;
		_ResetAll();
	}
	else if (Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_SELECT_PRIMARY]->uLatches &  GAMEPAD_BUTTON_1ST_PRESS_MASK )
	{
		if (CProfile::s_ProTrackCfg.m_nDisplayMode == CProTrackCfg::DISPLAYMODE_TREE && _pMenuCurProfile)
		{
			_pMenuCurProfile->m_bChildExpand ^= 1;	//toggle the expansion level of the current item
		}
	}
	else if ((Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY) &&
		      (Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT]->fCurrentState > 0.0f))
	{
		if (CProfile::s_ProTrackCfg.m_nDisplayMode == CProTrackCfg::DISPLAYMODE_TREE && _pMenuCurProfile)
		{
			_pMenuCurProfile->m_bChildExpand = 1;	//set the expansion level of the current item
		}
	}
	else if ((Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY) &&
		      (Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT]->fCurrentState < 0.0f))
	{
		if (CProfile::s_ProTrackCfg.m_nDisplayMode == CProTrackCfg::DISPLAYMODE_TREE && _pMenuCurProfile)
		{
			_pMenuCurProfile->m_bChildExpand = 0;	//set the expansion level of the current item
		}
	}
	else if( ((Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_LOOK_UP_DOWN]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY) &&
		     (Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_LOOK_UP_DOWN]->fCurrentState > 0.0f)) ||
			 ((Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY) &&
		     (Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN]->fCurrentState > 0.0f)))
	{	// up
		_nMenuDisplayCurItem--;
		FMATH_CLAMP(_nMenuDisplayCurItem, 0, (_nMenuDisplayTotalLines-1));
		if (_nMenuDisplayCurItem < _nMenuDisplayTopLine)
		{
			_nMenuDisplayTopLine = _nMenuDisplayCurItem;
		}
	}
	else if( ((Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_LOOK_UP_DOWN]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY) &&
		     (Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_LOOK_UP_DOWN]->fCurrentState < 0.0f)) ||
			 (Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY) &&
		     (Gamepad_aapSample[_nProTrackControllerId][GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN]->fCurrentState < 0.0f))
	{	// down
		_nMenuDisplayCurItem++;
		FMATH_CLAMP(_nMenuDisplayCurItem, 0, (_nMenuDisplayTotalLines-1));
		if (_nMenuDisplayCurItem >= _nMenuDisplayTopLine+_nMenuDisplayNumLines)
		{									   
			_nMenuDisplayTopLine += (_nMenuDisplayCurItem - (_nMenuDisplayTopLine+(_nMenuDisplayNumLines-1)));
		}
	}

}


void PROTRACK_DRAW(void)
{
	if (!CProfile::s_pProTrackRoot)
		return;

	if (!_bProTrackMenuOn)
	{
		return;
	}

	_pLastDrawnParentProfile = NULL;
	_nMenuLineCounter = 0;
	_nMenuDisplayCurLine = 0;
	_nMenuDisplayTotalLines = 0;
	if (CProfile::s_ProTrackCfg.m_nDisplayMode == CProTrackCfg::DISPLAYMODE_TREE)
	{
		_ProTrack_TreeRecurse(CProfile::s_pProTrackRoot, _ProfileDisplayTreeItem);
	}
	else if (CProfile::s_ProTrackCfg.m_nDisplayMode == CProTrackCfg::DISPLAYMODE_SUMS)
	{
		ftext_DebugPrintf(	_fMenuX,
						_fMenuY,
						"~w1PROTRACK: %s, %s",
						_aszDisplayModes[CProfile::s_ProTrackCfg.m_nDisplayMode],
						_aszSampleModes[CProfile::s_ProTrackCfg.m_nSampleMode]);

		CProfile* pProfile = (CProfile*) flinklist_GetHead(&CProfile::s_GlobalProfileList);
		while (pProfile)
		{
			_ProfileDisplayListItem(pProfile);
			pProfile = (CProfile*) flinklist_GetNext(&CProfile::s_GlobalProfileList, pProfile);
		}
	}
}
















void *flinklist_Find( FLinkRoot_t *pLinkRoot, void *pStructure )
{
	FLink_t* pTmp = pLinkRoot->pHeadLink;

	while ( pTmp  && flinklist_GetStructurePointer(pLinkRoot, pTmp) != pStructure)
	{
		pTmp = pTmp->pNextLink;	
	}
	
	if (pTmp)
	{  //found it!
		FASSERT(flinklist_GetStructurePointer(pLinkRoot, pTmp) == pStructure);
		return pStructure;
	}
	return NULL;
}


void flinklist_IterateUntil( FLinkRoot_t *pLinkRoot, IterateFunc pFunc)
{
	FLink_t* pTmp = pLinkRoot->pHeadLink;

	while ( pTmp && !pFunc(flinklist_GetStructurePointer(pLinkRoot, pTmp)))
	{
		pTmp = pTmp->pNextLink;	
	}
}


CProfile* ProfileListFind(FLinkRoot_t* pLinkRoot, cchar* pszName)
{
	FLink_t* pTmp = pLinkRoot->pHeadLink;

	u32 uThisCRC = fmath_Crc32(0, (u8*) pszName, fclib_strlen(pszName));

	while ( pTmp  && ((CProfile*) flinklist_GetStructurePointer(pLinkRoot, pTmp))->m_uNameCRC != uThisCRC)
	{
		pTmp = pTmp->pNextLink;	
	}

	if (pTmp)
	{  //found it!
		return (CProfile*) flinklist_GetStructurePointer(pLinkRoot, pTmp);
	}

	return NULL;
}

#endif //PROTRACK_ENABLE

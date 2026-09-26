#ifndef _AITACTICS_H_
#define _AITACTICS_H_

class CGroundCombat;

BOOL _GANoneInit(u32 uControl, void* pParam1, void* pParam2);
BOOL _GANoneWork(u32 uControl, void* pParam1, void* pParam2);
BOOL _GANoneUninit(u32 uControl, void* pParam1, void* pParam2);

void _GAStandGroundSetParams(	CGroundCombat* pT,
								u8 uDodgeTimeMin,					  //no-damage time based local dodge initiation frequency
								u8 uDodgeTimeMax,					  	
								u8 uDodgeOutTimeMin,				  //no-damage time based local dodge hold time
								u8 uDodgeOutTimeMax,				  
								u8 uDamageDodgeTimeMin,				  //minimum time between damage dodges
								u8 uEnemyRelAttackPtTimeMin,
								u8 uEnemyRelAttackPtTimeMax,
								u8 uInitialStandGroundState);
BOOL _GAStandGroundInit(u32 uControl, void* pParam1, void* pParam2);
BOOL _GAStandGroundWork(u32 uControl, void* pParam1, void* pParam2);
BOOL _GAStandGroundUninit(u32 uControl, void* pParam1, void* pParam2);

void _GARangeSetParams(	CGroundCombat* pT,
						u8 uDodgeTimeMin,									  //no-damage time based local dodge initiation frequency
						u8 uDodgeTimeMax,										
						u8 uDodgeOutTimeMin,								  //no-damage time based local dodge hold time
						u8 uDodgeOutTimeMax,
						u8 uDamageDodgeTimeMin,								  //minimum time between damage dodges
						f32 fRangeMin,
						f32 fRangeMax, 
						u8 uEnemyRelAttackPtSearchUsesage,					  //see CGroundCombat::ERAPT_***
						f32 fEraptLosIsLostWaitTime);						  //how long to wait before automatically starting an ERAPT_SMART
BOOL _GARangeInit(u32 uControl, void* pParam1, void* pParam2);
BOOL _GARangeWork(u32 uControl, void* pParam1, void* pParam2);
BOOL _GARangeUninit(u32 uControl, void* pParam1, void* pParam2);

BOOL _GARange3DInit(u32 uControl, void* pParam1, void* pParam2);
BOOL _GARange3DWork(u32 uControl, void* pParam1, void* pParam2);
BOOL _GARange3DUninit(u32 uControl, void* pParam1, void* pParam2);


BOOL _GACircleInit(u32 uControl, void* pParam1, void* pParam2);
BOOL _GACircleWork(u32 uControl, void* pParam1, void* pParam2);
BOOL _GACircleUninit(u32 uControl, void* pParam1, void* pParam2);

void _GAPeekABooSetParams(CGroundCombat* pT,
						  u8 m_uRepeatTimeMin,			//timer for when to come out of cover!
						  u8 m_uRepeatTimeMax,
						  u8 m_uAttackFromCoverTimeMin,		//timer for how long to stay out of cover under normal conditions
						  u8 m_uAttackFromCoverTimeMax);
BOOL _GAPeekABooInit(u32 uControl, void* pParam1, void* pParam2);
BOOL _GAPeekABooWork(u32 uControl, void* pParam1, void* pParam2);
BOOL _GAPeekABooUninit(u32 uControl, void* pParam1, void* pParam2);
void _GAPeekABooReset(CGroundCombat* pT);

void _GASearchSetParams(CGroundCombat* pT);
BOOL _GASearchInit(u32 uControl, void* pParam1, void* pParam2);
BOOL _GASearchWork(u32 uControl, void* pParam1, void* pParam2);
BOOL _GASearchUninit(u32 uControl, void* pParam1, void* pParam2);


extern u16 _kuInvestigateMarkFailureTime;
extern f32 _kfInvestigateMarkMinDist;

#endif
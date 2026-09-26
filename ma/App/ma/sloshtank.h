//////////////////////////////////////////////////////////////////////////////////////
// SloshTank.cpp - Slosh's backpack code
//
// Author: Michael Scholz
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.

// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 01.03.03 Scholz		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _SLOSHTANK_H_
#define _SLOSHTANK_H_ 1

#include "fang.h"
#include "fdraw.h"
#include "fgamedata.h"
#include "ftex.h"

#define _SLOSHTANK_NUM_BASEVERTS 7
#define _SLOSHTANK_NUM_CAPVERTS	 7
#define _SLOSHTANK_NUM_VERTS	14
#define _SLOSHTANK_NUM_FACES	19
#define _SLOSHTANK_NUM_CARDFACES 4
#define _SLOSHTANK_NUM_CARDVERTS 6
#define _SLOSHTANK_NUM_TEXTURES  2

// Tank refilling state:
typedef enum 
{
	// Common:
	TR_STATE_ACTIVE,
	TR_STATE_REFILLING,
	TR_STATE_COUNT
} WRState_e;

FCLASS_ALIGN_PREFIX class CSloshTank
{

//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef struct
	{
		f32 fNumTanks;						// ,1,# 	Number of tanks in the game (at a time)
		f32 fRefillRate;						//,0.5,#	Tank refill rate
		f32 fAlphaFluid;					// ,1.0,# 	Vertex alpha of fluid
		f32 fAlphaFluidSurface;				// ,1.0,# 	Vertex alpha of fluid surface
		f32 fAlphaFluidCard;				// ,1.0,# 	Vertex alpha of fluid card
		f32 fVerletPendentLength;			// ,2.5,#	Verlet pendent length
		f32 fVerletPendentGravityX;			// ,0,#		Verlet Pendent gravity X
		f32 fVerletPendentGravityY;			// ,-200,#	Verlet Pendent gravity Y
		f32 fVerletPendentGravityZ;			// ,0,#		Verlet Pendent gravity Z
		f32 fVerletDampeningConstant;		// ,0.3,#	Verlet Dampening constant
		f32 fTextureVelocityX;				// ,0.005,#Texture Velocity X
		f32 fTextureVelocityY;				// ,0.005,#Texture Velocity Y
		f32 fTextureTravelMaxX;				// ,.25,#	Texture Travel Max X
		f32 fTextureTravelMaxY;				// ,.25,#	Texture Travel Max X
		f32 fTextureChangeProbabilityX;		// odds of texture change dir
		f32 fTextureChangeProbabilityY;		// odds of texture change dir
	}_SloshTankProps_t;

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BOOL			 InitSystem( void );
	static void			 UninitSystem( void );
	static FINLINE	BOOL IsSystemInitialized( void ) { return m_bSystemInitialized; };
	
	static void Work( void );
	static void DrawAll( void );
	static CSloshTank* GetTank(void);
	static void PutTankBack(CSloshTank*);
	
	void UpdateTank(f32 fTankPercent);
	const f32* GetTankPercentPtr(void) { return &m_fTankFillPercentHeight;}
//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:
	CSloshTank();

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static  BOOL _InitTextures( void );

			void _Draw( void );
			BOOL _IsOffscreen( void );

	friend class CBotSlosh; // we actually belong to slosh, he can do anything with us
//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static const FGameDataMap_t m_aGameDataMap[];
	static const FGameData_TableEntry_t m_aSloshTankPropVocab[];
	static _SloshTankProps_t m_aSloshTankDefaultProps[1];

	static BOOL			m_bSystemInitialized;				// TRUE if the system has been initialized
	static BOOL			m_bSystemActive;
	static CSloshTank*	m_aTanks;
	static cchar*		m_apszTexNames[_SLOSHTANK_NUM_TEXTURES];
	static CFTexInst	m_aTextures[_SLOSHTANK_NUM_TEXTURES];
	static FDrawVtx_t	m_aTankVertsMS[_SLOSHTANK_NUM_VERTS];
	
	_SloshTankProps_t m_SloshTankProps;
	
	const CFMtx43A*		m_pmtxFluid_WS;						// Points to the attached fluid canister
	CFVec3A		m_vPendentVerletCurr;			// Current verlet point
	CFVec3A		m_vPendentVerletPrev;			// Previous verlet point
	f32			m_fTankHeight;					// Rest distance between the attach pt and the pendent
	f32			m_fTankFillPercentHeight;		// Rest distance between the attach pt and the pendent
	f32			m_fSOffset;
	f32			m_fSDir;
	f32			m_fTOffset;
	f32			m_fTDir;
	BOOL		m_bActive;						// should this do work/be drawn
	WRState_e   m_eTankState;

	FCLASS_STACKMEM_ALIGN( CSloshTank );
} FCLASS_NOALIGN_SUFFIX;

#endif
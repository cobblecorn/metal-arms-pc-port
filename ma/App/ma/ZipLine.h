//////////////////////////////////////////////////////////////////////////////////////
// ZipLine.h - Zip line entity
//
// Author: Nathan Miller
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
// 09/30/02 Miller      Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _ZIPLINE_H_
#define _ZIPLINE_H_ 1

#include "fang.h"
#include "entity.h"
#include "fworld.h"
#include "fparticle.h"
#include "fdraw.h"

class CEZipLine;
class CFSoundGroup;

FCLASS_ALIGN_PREFIX class CZipLineHook {
public:

	FINLINE CZipLineHook() { }

	CFVec3A m_Vel_WS;								// Current world space velocity
	CFVec3A m_Pos_WS;								// Current world space position of the hook
	CFVec3A m_Pos_Draw_WS;							// Where the zip line hook will be drawn, NOT the same as m_Pos_WS
	CFVec3A m_UnitVecToNext;						// Unit vector to next hook toward end2

	f32 m_fSpeed_WS;								// Current world space speed
	f32 m_fUnitDist12;								// Unit distance along the cable from end1 to end2
	f32 m_fUnitLoadMass;							// Unit mass the hook is carrying
	f32 m_fUnitFriction;							// Unit friction of the hook
	f32 m_fInputBrakeAmount;						// Friction from the user holding the handbreak. (1.0f = full, 0.0f = none)
	f32 m_fGripSlowDownTotal;						// Amount to slow down, accumulates over time (0=none, 1=full)
	f32 m_fSparkIntensity;							// Intensity of the sparks, monitored by the emitter (0=low, 1=high)					
	f32 m_fYawSnapOffset;							// Offset to properly snap bot hand to zipline

	BOOL8 m_bAttached;								// TRUE when this hook is attached
	BOOL8 m_bUsedSlot;								// TRUE when this hook is using a cable's slot
	BOOL8 m_bTakingSlack;							// TRUE when we're dropping to take slack
	BOOL8 m_bBrakeStartThisFrame;					// TRUE if we started to brake this frame

	CEZipLine *m_pCable;							// The cable we're attached to

	FParticle_EmitterHandle_t m_hSparkEmitter;		// Emitter for the sparks coming from the bot's hand

	CFAudioEmitter *m_pZipEmitter;					// On zipline audio emitter
	CFAudioEmitter *m_pAudioEmitterBrake;			// Braking on the zipline


	FCLASS_STACKMEM_ALIGN( CZipLineHook );
} FCLASS_ALIGN_SUFFIX;

FCLASS_ALIGN_PREFIX class CEZipLine : public CEntity {

public:
	CEZipLine();
	virtual ~CEZipLine();

	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static BOOL IsSystemInitialized( void ) { return m_bSystemInitialized; }
	static void Destroy( void );

	static CEZipLine *GetZipCollision( const CFSphere &Sphere );

	static void DrawAll( void );
	static void WorkAll( void );

	FINLINE void Enable( BOOL bEnable );
	FINLINE BOOL IsEnabled( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nFlags & FLAG_ENABLE); }

	FINLINE void SetVisibility( BOOL bVisibility );
	FINLINE BOOL IsVisible( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nFlags & FLAG_VISIBLE); }

	CZipLineHook *AttachHook( const CFVec3A *pInitialPos_WS, const CFVec3A* pVelocity_WS, CBot *pBot );
	void DetachHook( CZipLineHook *pCableHook, CBot *pBot, BOOL bPlayDetachSound=TRUE );

	BOOL SphereIntersect( const CFSphere &Sphere);

	CFSoundGroup *GetAttachSound( void )	{ return m_pAttachSound; }
	CFSoundGroup *GetDetachSound( void )	{ return m_pDetachSound; }
	CFSoundGroup *GetZippingSound( void )	{ return m_pZippingSound; }
	CFSoundGroup *GetBrakeSound( void )		{ return m_pBrakeSound; }
	
protected:
	ENTITY_CLASS_HIERARCHY_BITDEF

	virtual BOOL ClassHierarchyLoadSharedResources( void );
	virtual void ClassHierarchyUnloadSharedResources( void );

	virtual void ClassHierarchyDestroy( void );
	virtual BOOL ClassHierarchyBuild( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );

	void ClassHierarchyAddToWorld( void );
	void ClassHierarchyRemoveFromWorld( void );

public:
	enum { MAX_HOOK_COUNT = 4 };					// The maximum number of attached hooks per cable

	CFVec3A m_End2_WS;								// The other end of the cable
	CFVec3A m_UnitVecToFirstHook;					// Unit vector from end1 to first hook (equal to m_Mtx.m_vFront if no hooks)
	CFMtx43A m_Mtx;									// Front is from end1 to end2, Up is world space Y+, Pos is at End1
	CFVec2 m_UnitVec12XZ;							// Unit vector from end1 to end2 in XZ space
	
	CFSphere m_BoundingSphere;

	f32 m_fLength;									// Length of the cable in feet
	f32 m_fOOLengthXZ;								// Inverse length projected onto the XZ plane

	static CEZipLine *_GetHead( void ) { return (CEZipLine *)flinklist_GetHead( &m_LinkRoot ); }
private:
	enum { MAX_VTX_COUNT = MAX_HOOK_COUNT + 2 };	// Maximum number of FDraw vertices

	enum {
		FLAG_CREATED				= 0x00000001,	// This cable has been created
		FLAG_VISIBLE				= 0x00000002,	// Draw this cable
		FLAG_ENABLE					= 0x00000004,	// Enable this cable
		FLAG_OPAQUE					= 0x00000008,	// Draw opaque, regardless of texture and color alpha values

		FLAG_NONE					= 0x00000000
	};

	void _AddZipLine( void ) { flinklist_AddTail( &m_LinkRoot, this ); }
	void _RemoveZipLine( void ) { flinklist_Remove( &m_LinkRoot, this ); }
	static CEZipLine *_GetTail( void ) { return (CEZipLine *)flinklist_GetTail( &m_LinkRoot ); }
	static CEZipLine *_GetNext( CEZipLine *pCable ) { return (CEZipLine *)flinklist_GetNext( &m_LinkRoot, pCable ); }

	void _Init( void );
	void _Destroy( void );
	void _SetEndPoints( const CFVec3A *pEnd1_WS, const CFVec3A *pEnd2_WS );

	void _Draw( void );
	f32 _GetLineWidth( void );
	void _Work( void );
	void _SortHooks( void );
	void _RecomputeHookVecs( void );
	FINLINE BOOL _SphereBelowLine( const CFSphere &Sphere, const CFVec3A &Point, const CFVec3A &Dir );
	void _KillHookEmitter( CZipLineHook *pHook );
	void _KillBrakeAudioEmitter( CZipLineHook *pHook );

	static int _HookSortCallback( const void *pElement1, const void *pElement2 );
	static BOOL _CollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );

	static BOOL m_bSystemInitialized;				// The cable system has been initialized
	static FLinkRoot_t m_LinkRoot;					// Linklist of CCable objects
	static FDrawVtx_t m_aDrawVtx[MAX_VTX_COUNT];	// FDraw vertices from end1 to end2
	static u32 m_nClassClientCount;					// Number of ziplines of this class using the class-wide resources

	static CFSoundGroup *m_pAttachSound;
	static CFSoundGroup *m_pDetachSound;
	static CFSoundGroup *m_pZippingSound;
	static CFSoundGroup *m_pBrakeSound;

	u32 m_nFlags;									// See FLAG_* for info
	FLink_t m_Link;									// Link to other CEZipLine objects

	f32 m_fUnitGive;								// The amount the cable gives to weight (0=none, 1=max)
	f32 m_fTextureRepeatPerFoot;					// The number of times to repeat the texture per foot
	f32 m_fUnitFriction;							// Amount of friction (0=none, 1=max)
	f32 m_fAccel;									// How much to accelerate(1=none, 2=max)

	CFColorRGBA m_ColorRGBA;						// Color bias
	CFTexInst m_TexInst;							// Texture to apply (m_pTexDef is NULL for none)

	u32 m_nAttachedHookCount;						// The number of attached hooks
	CZipLineHook *m_apAttachedHook[MAX_HOOK_COUNT];	// Array of attached hook pointers (NULL=none), sorted from closest to end1 to closest to end2
	CZipLineHook m_aAttachedHook[MAX_HOOK_COUNT];	// Array of attached hook data (m_fSpeed_WS < 0 indicates an unused slot)

	CFWorldTracker *m_pWorldTracker;

	FParticle_DefHandle_t m_hSparkParticleDef;		// Particle def to use for this zipline

	FCLASS_STACKMEM_ALIGN( CEZipLine );
} FCLASS_ALIGN_SUFFIX;


FCLASS_ALIGN_PREFIX class CEZipLineBuilder : public CEntityBuilder
{
public:

	FINLINE CEZipLineBuilder() { }

	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );

protected:
	virtual BOOL InterpretTable(void);
	virtual BOOL PostInterpretFixup(void);

public:
	CFVec3A m_End1_WS;							// One of the endpoints in world space
	CFVec3A m_End2_WS;							// The other endpoint in world space

	f32 m_fUnitGive;							// Amount of tensil give (0=none, 1=max)
	f32 m_fUnitFriction;						// Amount of friction (0=none, 1=max)
	f32 m_fAccel;								// Amount to accelerate on the zipline (1=none, 2=max)

	BOOL m_bVisible;							// TRUE if this cable is to be drawn
	BOOL m_bOpaque;								// Draw opaque, regardless of texture and color alpha values

	CFColorRGBA m_ColorBiasRGBA;				// Color bias
	cchar *m_szTextureName;						// Texture to apply (NULL=none)
	f32 m_fTextureRepeatPerFoot;				// Number of times to repeat the texture per foot

	cchar *m_szParticleName;					// Name of the particle system to use when on the zipline (default is "zipspark")

	FCLASS_STACKMEM_ALIGN(CEZipLineBuilder);

} FCLASS_ALIGN_SUFFIX;

// Returns TRUE if the sphere's position is below the passed line
// Point is a point on the line and Dir is the direction of the line
FINLINE BOOL CEZipLine::_SphereBelowLine( const CFSphere &Sphere, const CFVec3A &Point, const CFVec3A &Dir ) {
	CFVec3A Right;
	CFVec3A Up;

	Right.Cross( Dir, CFVec3A::m_UnitAxisY );

	// Only keep going if the zip line isn't horizontal (it shouldn't be)
	if( Right.MagSq() > 0.0001f ) {
		Up.Cross( Right, Dir );

		// We are below the line, it is okay to attach to it
		if( Up.Dot( Sphere.m_Pos ) + -Up.Dot( Point ) < 0.0f ) {
			return TRUE;
		}
	}

	return FALSE;
}

#endif
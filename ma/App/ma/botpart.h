//////////////////////////////////////////////////////////////////////////////////////
// botpart.h - Bot part module.
//
// Author: Steve Ranck     
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
// 01/02/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _BOTPART_H_
#define _BOTPART_H_ 1

#include "fang.h"
#include "fverlet.h"
#include "fgamedata.h"
#include "bot.h"
#include "fparticle.h"
#include "fsndfx.h"



#define BOTPART_MAX_BONES_PER_PART		16		// Bones contributing to the part
#define BOTPART_MAX_TACKS_PER_PART		16		// Non-collision tacks
#define BOTPART_MAX_CTACKS_PER_PART		16		// Collision tacks (these collide with collision planes)
#define BOTPART_MAX_BTACKS_PER_PART		4		// Blast tacks (these receive blast impulses)
#define BOTPART_MAX_CPLANES_PER_PART	4		// Collision planes
#define BOTPART_MAX_ANCHORS				4		// Anchors
#define BOTPART_MAX_PARTICLES_PER_LIMB	4		// Particles
#define BOTPART_MAX_PARTS_PER_LIMB		10		// Maximum number of parts per limb



class CBot;
class CBotPart;
class CBotPartPool;
class CFDebrisGroup;
class CEParticle;
class CBotZomPart;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSeg - Represents one boned mesh segment of a bot.
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
FCLASS_ALIGN_PREFIX class CBotSeg {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:




	FCLASS_STACKMEM_NOALIGN( CBotSeg );
} FCLASS_NOALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotPartInit - IMPORTANT: If you change the data layout of this class, you *MUST* update m_aGameDataVocab and all bp_*.csv files.
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
FCLASS_NOALIGN_PREFIX class CBotPartInit {
//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	struct Anchor_t {
		u32 nAttachTypeCode;									// 0=anim-driven, 1=rest position, 2=part

		cchar *pszTackName;										// Name of the tack that will be anchored
		cchar *pszAnchorBoneName;								// Name of the tack to anchor to

		cchar *pszAnchorParentBoneName;							// Attach Type 0: Unused
																// Attach Type 1: Name of bone pszAnchorBoneName is attached to ("None" to use its mesh parent bone)
																// Attach Type 2: Name of the part to attach to

		f32 fMinDist;											// Minimum anchor distance
		f32 fMaxDist;											// Maximum anchor distance
	};


	struct CollPlane_t {
		cchar *pszBoneName;										// Name of the bone defining this collision plane
		u32 nAxisCode;											// Which axis defines the normal 0=x, 1=y, 2=z, 3=-x, 4=-y, 5=-z
		u32 nAnchorTypeCode;									// Type of anchor: 0=dynamic, 1=boned
		f32 fFriction;											// Friction on this plane
		f32 fDistFromPlane;										// Distance tacks must be from the plane
		BOOL bPreserveVelocity;									// TRUE if the collision response with the plane should not introduce a velocity change
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static const FGameData_TableEntry_t m_aGameDataVocab[];

	cchar *m_pszLimbName;										// The limb this part is a member of

	cchar *m_apszBoneNameParts[BOTPART_MAX_BONES_PER_PART];		// Names of the bones in this part (first name in the list is the base bone)
	cchar *m_apszBoneNameTacks[BOTPART_MAX_TACKS_PER_PART];		// Names of the bones that are to be considered tacks
	cchar *m_apszBoneNameCTacks[BOTPART_MAX_CTACKS_PER_PART];	// Names of the bones that are to be considered collision tacks
	cchar *m_apszBoneNameBTacks[BOTPART_MAX_BTACKS_PER_PART];	// Names of the bones that are to receive blast impulses

	CollPlane_t m_aCollPlanes[BOTPART_MAX_CPLANES_PER_PART];	// Collision planes
	Anchor_t m_aAnchors[BOTPART_MAX_ANCHORS];					// Anchors

	f32 m_fDimX;												// Approximate part dimension X
	f32 m_fDimY;												// Approximate part dimension Y
	f32 m_fDimZ;												// Approximate part dimension Z

	f32 m_fMass;												// Approximate part mass
	f32 m_fDamping;												// Damping factor

	u32 m_nMeshPartsMask;										// Describes which mesh parts are members of this bot part (matches the .AID file for the bot mesh)

	BOOL m_bCTackRayTest;										// TRUE to perform a ray test from the first CTack to the second CTack




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	BOOL LoadFromGameData( cchar *pszTableName, FGameDataFileHandle_t hGameDataFile );
	BOOL LoadFromGameData( FGameDataTableHandle_t hTable, cchar *pszFileName );


	friend class CBotPart;
	friend class CBotPartPool;


	FCLASS_STACKMEM_NOALIGN( CBotPartInit );
} FCLASS_NOALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotPart
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
FCLASS_ALIGN_PREFIX class CBotPart {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		MODE_DISABLED,											// This part isn't being used
		MODE_DANGLING,											// This part is dangling from a bot
		MODE_DEBRIS,											// This part is loose debris
		MODE_REPAIR,

		MODE_COUNT
	} Mode_e;


	typedef enum {
		ATTACH_TYPE_DYNAMIC_MTX_PALETTE,						// Dynamic anchor attached directly to matrix palette
		ATTACH_TYPE_DYNAMIC_ANIM_OUTPUT,						// Dynamic anchor attached either to matrix palette if anchor bone is not Verleted, or to m_DanglingBoneAnimDrivenPos_WS otherwise
		ATTACH_TYPE_BONED,										// Boned anchor
		ATTACH_TYPE_PART,										// Anchor attached to another part

		ATTACH_TYPE_COUNT
	} AttachType_e;



	enum {
		TACK_SURFACE_FLAG_PRESERVE_VELOCITY		= 0x01,			// Sets the CFVerletTackSurface::FLAG_PRESERVE_VELOCITY flag

		TACK_SURFACE_FLAG_NONE					= 0x00
	};


	struct AnchorDef_t {
		CFVerletTack *pTack;									// The tack that's being anchored to something

		cchar *pszAnchorPartName;								// Used for ATTACH_TYPE_PART only

		u8 nAnchorBoneIndex;									// Bone being anchored to
		u8 nParentBoneIndex;									// Used for ATTACH_TYPE_BONED only (255=attach to Xfm)

		u8 nAttachType;											// See AttachType_e for info

		f32 fMinDist;											// Minimum anchor distance
		f32 fMaxDist;											// Maximum anchor distance
	};


	struct TackSurfaceDef_t {
		u8 nBoneIndex;											// The bone index that describes the orientation of this surface
		u8 nAnchorType;											// CFVerletTackSurface::AnchorType_e
		u8 nPlaneNormalAxis;									// CFVerletTackSurface::PlaneNormal_e
		u8 nFlags;												// See TACK_SURFACE_FLAG_* for info

		f32 fFriction;											// Friction on this plane
		f32 fDistFromPlane;										// Distance tacks must be from the plane
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CFVerlet m_Verlet;											// Verlet physics object for this part

	CFVec3A m_DanglingBoneAnimDrivenPos_WS;						// Position of the base bone before the Verlet system replaces it

	cchar *m_pszLimbName;										// The limb this part is a member of

	Mode_e m_nMode;												// The part's mode

	cchar *m_pszPartName;										// Name of this part
	u32 m_nMeshPartsMask;										// Describes which mesh parts are members of this bot part (matches the .AID file for the bot mesh)

	CBot *m_pDanglingFromBot;									// The bot this part is dangling from (used for MODE_DANGLING, NULL otherwise)
	BOOL8 m_bDanglingBoneCallbackPreviouslyEnabled;				// TRUE if the bone callback for the base bone was previously enabled (used for MODE_DANGLING only)

	u8 m_nBlastTackCount;										// Number of elements in m_pnBlastTackBoneIndexArray
	u8 m_nTackSurfaceCount;										// Number of elements in m_pTackSurfaceDefArray and m_pTackSurfaceArray
	u8 m_nAnchorCount;											// Number of elements in m_apAnchors

	s8 m_nRayTestStartVerletTackIndex;							// If not -1, these are the start and end indices in the m_Verlet
	s8 m_nRayTestEndVerletTackIndex;							//   object of where to cast the ray.

	u8 *m_pnBlastTackIndexArray;								// Array of tack indices that are to receive blast damage (indices are ino the m_Verlet object)
	TackSurfaceDef_t *m_pTackSurfaceDefArray;					// Array of tack surface definition objects (used to dynamically init m_pTackSurfaceArray)
	CFVerletTackSurface *m_pTackSurfaceArray;					// Array of tack surface objects
	CFVerletTackSurface **m_ppTackSurfaceArray;					// Array of tack surface object pointers

	AnchorDef_t *m_pAnchorDefArray;								// Anchor definitions
	
	f32 m_fUnitRepairTimer;										// Timer used while repairing a part




//----------------------------------------------------------------------------------------------------------------------------------
// Public Function:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CBotPart() { m_pTackSurfaceArray=NULL; }
	~CBotPart() { Destroy(); }

	static CBotPart *CreateArrayFromGameData( cchar *pszTableName, FGameDataFileHandle_t hGameDataFile, const FMesh_t *pMesh, u32 nInstanceCount );
	static CBotPart *CreateArrayFromGameData( FGameDataTableHandle_t hTable, cchar *pszFileName, const FMesh_t *pMesh, u32 nInstanceCount );
	static CBotPart *CreateArray( cchar *pszPartName, const CBotPartInit *pBotPartInit, const FMesh_t *pMesh, u32 nInstanceCount );

	BOOL Create( cchar *pszPartName, const CBotPartInit *pBotPartInit, const FMesh_t *pMesh );
	void Destroy( void );
	FINLINE BOOL IsCreated( void ) const { return m_Verlet.IsCreated(); }

	FINLINE Mode_e GetMode( void ) const { FASSERT( IsCreated() ); return m_nMode; }

	void SetMode_Disabled( void );
	void SetMode_Dangling( CBot *pDanglingFromBot );
	void SetMode_Repair( void );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Function:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _InitTackSurfaces( CFWorldMesh *pWorldMesh );
	u32 _FillStringTable( cchar **apszDestNameTable, cchar * const *apszSourceNameTable, u32 nMaxCount, BOOL bAddNullTerminator );

	static void _VerletConstraintCallback( CFVerlet *pVerlet );


	FCLASS_STACKMEM_ALIGN( CBotPart );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotPartPool - Only one of these per bot class. Note that if a bot class implements multiple FMesh_t's (like a red and green
//                grunt) each version will need its own CBotPartPool.
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotPartPool {

//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum {												// NOTE: The order of these must be such that COMPONENT_TYPE_FLAG_* == (1 << CBotPartMgr::ComponentType_e) !!!
		COMPONENT_TYPE_FLAG_EYES		= 0x01,			// Component that controls the bot's vision
		COMPONENT_TYPE_FLAG_PRIMARY		= 0x02,			// Component that controls the bot's primary weapon
		COMPONENT_TYPE_FLAG_SECONDARY	= 0x04,			// Component that controls the bot's secondary weapon
		COMPONENT_TYPE_FLAG_LIGHT		= 0x08,			// Component that controls the bot's spotlight

		COMPONENT_TYPE_FLAG_NONE		= 0x00
	};


	FCLASS_NOALIGN_PREFIX class CBoneDef {
	public:

		u8 m_nBoneGeoDebrisGroupIndex;					// Index into CBot::m_pBotInfo_Gen->apBoneGeoDebrisGroup of which debris group to spawn bone geo debris
		u8 m_nFlamingChunkCount;						// Number of flaming chunks to spawn for this bone
		u8 m_nChunkCount;								// Number of chunks to spawn for this bone
		u8 m_nGutCount;									// Number of guts to spawn for this bone
		u8 m_bBoneGeoOnFire;							// Catch bone geo on fire
		u8 m_nBoneIndex;								// Bone index
		f32 m_fChunkScale;								// Chunk scale multiplier
		f32 m_fGutScale;								// Gut scale multiplier
		FExplosion_GroupHandle_t m_hExplosion;			// Explosion to spawn when this bone dies


		FCLASS_STACKMEM_NOALIGN( CBoneDef );
	} FCLASS_NOALIGN_SUFFIX;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	typedef enum {
		DANGLE_RULE_ZERO_HEALTH,						// When the health reaches zero, dangle the limb
		DANGLE_RULE_BACK_BREAKER,						// If health drops below 0, % chance of dangling limb (100% if from blast)

		DANGLE_RULE_COUNT
	} DangleRule_e;


	typedef enum {
		LEG_TYPE_RIGHT,									// Right leg
		LEG_TYPE_LEFT,									// Left leg
		LEG_TYPE_NONE,									// Not a leg

		LEG_TYPE_COUNT
	} LegType_e;


	struct ParticleInfo_t {
		FParticle_DefHandle_t hParticleDef;				// Particle definition handle
		f32 fDuration;
		f32 fUnitIntensity;								// Initial intensity of particle effect
	};


	struct LimbGameDataInfo_t {							// Note: This data doesn't stick around after RegisterClient()
		cchar *pszLimbName;								// Name of the limb
		cchar *pszArmorProfileIntact;					// Armor profile used when limb is intact
		cchar *pszArmorProfileDangle;					// Armor profile used when limb is dangling
		u32 nComponentTypeFlags;						// See COMPONENT_TYPE_FLAG_* for info

		f32 fInitialHealthMultIntact;					// This gets multiplied by the bot's initial health and stored in the limb at start-up (when the limb's intact)
		f32 fInitialHealthMultDangle;					// This gets multiplied by the bot's initial health and stored in the limb when the limb is set to dangle
		u32 nDangleRuleCode;							// See DangleRule_e

		cchar *pszProxyDamageBone;						// The proxy bone, when it gets hit, should inflict damage onto the limb (NULL=none)
		f32 fProxyDamageBox_NegX_BS;					// The proxy damage box -x dimension, relative to the proxy bone
		f32 fProxyDamageBox_PosX_BS;					// The proxy damage box +x dimension, relative to the proxy bone
		f32 fProxyDamageBox_NegY_BS;					// The proxy damage box -y dimension, relative to the proxy bone
		f32 fProxyDamageBox_PosY_BS;					// The proxy damage box +y dimension, relative to the proxy bone
		f32 fProxyDamageBox_NegZ_BS;					// The proxy damage box -z dimension, relative to the proxy bone
		f32 fProxyDamageBox_PosZ_BS;					// The proxy damage box +z dimension, relative to the proxy bone

		cchar *pszParticleBone;							// The particle bone to attach particles & spawn debris (NULL=none)
		ParticleInfo_t aParticleInfo[BOTPART_MAX_PARTICLES_PER_LIMB];	// Array of particle info structures
		CFDebrisGroup *m_pDebrisGroup;					// Debris group (NULL=none)

		FSndFx_FxHandle_t hDangleSound;					// Sound to make when limb starts to dangle
		FSndFx_FxHandle_t hBlowOffSound;				// Sound to make when limb gets blown into pieces
		FExplosion_GroupHandle_t hBlowOffExplosion;		// Explosion to spawn when limb gets blown into pieces
	};


	struct BoneInfo_t {									// Note: This data doesn't stick around after RegisterClient()
		cchar *pszBoneName;								// The name of the bone
		u32 nBurstCode;									// 0=timed, 1=immediate, 2=impact
		u32 nFlamingChunkCount;							// Number of flaming chunks to spawn for this bone
		u32 nChunkCount;								// Number of chunks to spawn for this bone
		u32 nGutCount;									// Number of guts to spawn for this bone
		BOOL bBoneGeoOnFire;							// Catch bone geo on fire
		f32 fChunkScale;								// Chunk scale multiplier
		f32 fGutScale;									// Gut scale multiplier
		FExplosion_GroupHandle_t hExplosion;			// Explosion to be spawned when this bone dies
	};


	struct PartTypePool_t {
		CBotPart *pPartArray;							// Points to the array of parts in this set
		u8 nLimbType;									// Limb type
	};


	FCLASS_ALIGN_PREFIX class CLimbDef {
	public:

		u8 m_nStartPartType;							// Index of the first part in this limb
		u8 m_nEndPartType;								// Index+1 of the last part in this limb
		s8 m_nProxyBoneIndex;							// The index of the proxy bone (-1=none)
		s8 m_nParticleBoneIndex;						// The index of the particle bone (-1=none)
		u8 m_nParticleDefCount;							// Number of elements in m_pParticleDefArray
		u8 m_nLegType;									// Which leg (see LegType_e for info)
		u8 m_nComponentTypeFlags;						// See COMPONENT_TYPE_FLAG_* for info

		f32 m_fInitialHealthMultIntact;					// This gets multiplied by the bot's initial health and stored in the limb at start-up (when the limb's intact)
		f32 m_fInitialHealthMultDangle;					// This gets multiplied by the bot's initial health and stored in the limb when the limb is set to dangle
		DangleRule_e m_nDangleRule;						// The rule that dictates when we should dangle the part

		CArmorProfile *m_pArmorProfileIntact;			// This limb's armor profile used when the limb is intact
		CArmorProfile *m_pArmorProfileDangle;			// This limb's armor profile used when the limb is dangling

		CFVec3A m_ProxyBoxMinCorner_BS;					// Proxy box minimum x,y,z corner in proxy bone space
		CFVec3A m_ProxyBoxMaxCorner_BS;					// Proxy box minimum x,y,z corner in proxy bone space

		ParticleInfo_t *m_pParticleInfoArray;			// Array of particle info structures
		CFDebrisGroup *m_pDebrisGroup;					// Debris group (NULL=none)

		FSndFx_FxHandle_t m_hDangleSound;				// Sound to make when limb starts to dangle
		FSndFx_FxHandle_t m_hBlowOffSound;				// Sound to make when limb gets blown into pieces

		FExplosion_GroupHandle_t m_hBlowOffExplosion;	// Explosion to be spawned when this limb blows off


		FCLASS_STACKMEM_ALIGN( CLimbDef );
	} FCLASS_ALIGN_SUFFIX;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static const FGameData_TableEntry_t m_aGameDataVocab_LimbGameDataInfo[];	// For LimbGameDataInfo_t
	static const FGameData_TableEntry_t m_aGameDataVocab_BoneInfo[];			// For BoneInfo_t

	static SmokeTrailAttrib_t m_SmokeTrailAttrib;			// Smoke trail attributes for debris
	static FParticle_DefHandle_t m_hDebrisFlame;			// Particle def handle for debris flame
	static FParticle_DefHandle_t m_hDebrisSmoke;			// Particle def handle for debris smoke
	static BOOL m_bSystemInitialized;						// TRUE when InitSystem() has been called

	CBotPartPool **m_ppPartPool;						// Each bot class has a single instance of a CBotPartPool. This is the address of the instance pointer.

	u8 m_nClientCount;									// Number of clients using this pool
	u8 m_nPartsPerPool;									// Number of parts per pool
	u8 m_nPartTypeCount;								// Number of part types (elements in m_pPartTypePoolArray)
	u8 m_nLimbTypeCount;								// Number of limb types (elements in m_pLimbDefArray)
	u8 m_nBoneCount;									// Number of bones in the mesh
	u8 m_nPackedBoneWordCount;							// (m_nBoneCount + 31)/32
	u8 m_nBoneInfoCount;								// Number of elements in m_pBoneInfo
	s8 m_nLimbTypeRightLeg;								// Limb type for the right leg (-1=none)
	s8 m_nLimbTypeLeftLeg;								// Limb type for the left leg (-1=none)
	s8 m_nRootBoneIndex;								// Bone index of root bone used for total destruction of the bot (-1 = none)
	s8 m_nTorsoBoneIndex;								// Bone index of torso bone used for partial destruction of the bot (-1 = none)

	CBoneDef *m_pBoneInfo;								// Per-bone info

	PartTypePool_t *m_pPartTypePoolArray;				// One pool of parts per part type
	CLimbDef *m_pLimbDefArray;							// One limb def per limb type
	s8 *m_anBoneToPartArray;							// Index with a bone index and get a part type (-1 = no part for the bone)
	BOOL8 *m_abBoneIsProxyArray;						// Index with a bone index and get a BOOL that's TRUE if this is a proxy bone
	u32 *m_pnBoneIsProxyBitmask;						// Bit fields (one per bone index) of bones that are proxy bones
	u32 *m_pnOrigBoneCallbackEnabledBitmask;			// Bit fields (one per bone index) of bones that have had their bone anim callback enabled by the bot




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static FINLINE BOOL IsSystemInitialized( void ) { return m_bSystemInitialized; }

	FINLINE const CBoneDef* GetBoneInfoArray( u32 *pnBoneInfoCountOut ) const { (*pnBoneInfoCountOut) = m_nBoneInfoCount; return m_pBoneInfo; }




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	CBotPartPool();
	~CBotPartPool();

	static BOOL RegisterClient( CBotPartPool **m_ppPartPool, CBot *pBot, cchar *pszGameDataFileName, u32 nPartsPerPoolCount, const FMesh_t *pMesh );
	static void UnregisterClient( CBotPartPool *pPartPool );
	BOOL IsCreated( void ) const { return (m_nClientCount > 0); }


	CBotPart *FindFreePart( u32 nPartType );
	BOOL FindAvailablePartsForLimbDangling( u32 nLimbType, BOOL bRepairWhenStolen );

	FINLINE s32 GetPartTypeFromBoneIndex( u32 nBoneIndex ) const;
	FINLINE s32 GetLimbTypeFromBoneIndex( u32 nBoneIndex ) const;
	FINLINE BOOL GetLimbAndPartTypesFromBoneIndex( u32 nBoneIndex, u32 *pnLimbType, u32 *pnPartType ) const;

	void _ClearDataMembers( void );
	static void _InitEffectDefs( void );

	FINLINE void _SetProxyBoneBitmask( u32 nBoneIndex );
	FINLINE void _ClearProxyBoneBitmask( u32 nBoneIndex );
	FINLINE BOOL _GetProxyBoneBitmask( u32 nBoneIndex ) const;

	FINLINE void _SetOrigBoneCallbackEnabledBitmask( u32 nBoneIndex );
	FINLINE void _ClearOrigBoneCallbackEnabledBitmask( u32 nBoneIndex );
	FINLINE BOOL _GetOrigBoneCallbackEnabledBitmask( u32 nBoneIndex ) const;


	friend class CBotPartMgr;
	friend class CBotZomPart;


	FCLASS_STACKMEM_ALIGN( CBotPartPool );
} FCLASS_ALIGN_SUFFIX;




FINLINE s32 CBotPartPool::GetPartTypeFromBoneIndex( u32 nBoneIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nBoneIndex<m_nBoneCount );
	
	return (s32)m_anBoneToPartArray[nBoneIndex];
}


FINLINE s32 CBotPartPool::GetLimbTypeFromBoneIndex( u32 nBoneIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nBoneIndex < m_nBoneCount );

	s8 nPartType = m_anBoneToPartArray[nBoneIndex];

	if( nPartType < 0 ) {
		return -1;
	}

	return (s32)m_pPartTypePoolArray[nPartType].nLimbType;
}


FINLINE BOOL CBotPartPool::GetLimbAndPartTypesFromBoneIndex( u32 nBoneIndex, u32 *pnLimbType, u32 *pnPartType ) const {
	FASSERT( IsCreated() );
	FASSERT( nBoneIndex < m_nBoneCount );

	s32 nPartType = m_anBoneToPartArray[nBoneIndex];

	if( nPartType < 0 ) {
		return FALSE;
	}

	*pnPartType = (u32)nPartType;
	*pnLimbType = (u32)m_pPartTypePoolArray[nPartType].nLimbType;

	return TRUE;
}


FINLINE void CBotPartPool::_SetProxyBoneBitmask( u32 nBoneIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nBoneIndex < m_nBoneCount );

	m_pnBoneIsProxyBitmask[ nBoneIndex >> 5 ] |= (1 << (nBoneIndex & 31));
}


FINLINE void CBotPartPool::_ClearProxyBoneBitmask( u32 nBoneIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nBoneIndex < m_nBoneCount );

	m_pnBoneIsProxyBitmask[ nBoneIndex >> 5 ] &= ~(1 << (nBoneIndex & 31));
}


FINLINE BOOL CBotPartPool::_GetProxyBoneBitmask( u32 nBoneIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nBoneIndex < m_nBoneCount );

	return (BOOL)((m_pnBoneIsProxyBitmask[ nBoneIndex >> 5 ] >> (nBoneIndex & 31)) & 1);
}


FINLINE void CBotPartPool::_SetOrigBoneCallbackEnabledBitmask( u32 nBoneIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nBoneIndex < m_nBoneCount );

	m_pnOrigBoneCallbackEnabledBitmask[ nBoneIndex >> 5 ] |= (1 << (nBoneIndex & 31));
}


FINLINE void CBotPartPool::_ClearOrigBoneCallbackEnabledBitmask( u32 nBoneIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nBoneIndex < m_nBoneCount );

	m_pnOrigBoneCallbackEnabledBitmask[ nBoneIndex >> 5 ] &= ~(1 << (nBoneIndex & 31));
}


FINLINE BOOL CBotPartPool::_GetOrigBoneCallbackEnabledBitmask( u32 nBoneIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nBoneIndex < m_nBoneCount );

	return (BOOL)((m_pnOrigBoneCallbackEnabledBitmask[ nBoneIndex >> 5 ] >> (nBoneIndex & 31)) & 1);
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotPartMgr
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotPartMgr {

//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		LIMB_STATE_INTACT,								// Part is intact on bot
		LIMB_STATE_DANGLING,							// Part is dangling on bot
		LIMB_STATE_REMOVED,								// Part has been blown off bot

		LIMB_STATE_COUNT
	} LimbState_e;


	typedef enum {										// NOTE: The order of these must be such that CBotPartPool::COMPONENT_TYPE_FLAG_* == (1 << ComponentType_e) !!!
		COMPONENT_TYPE_EYES,							// Component that controls the bot's vision
		COMPONENT_TYPE_PRIMARY,							// Component that controls the bot's primary weapon
		COMPONENT_TYPE_SECONDARY,						// Component that controls the bot's secondary weapon
		COMPONENT_TYPE_LIGHT,							// Component that controls the bot's spotlight

		COMPONENT_TYPE_NONE,							// The limb isn't one of the components above (always COMPONENT_STATUS_ALL_FULLY_OPERATIONAL)

		COMPONENT_TYPE_COUNT
	} ComponentType_e;


	typedef enum {
		COMPONENT_STATUS_ALL_FULLY_OPERATIONAL,			// All of the components of a particular type are fully operational
		COMPONENT_STATUS_SOME_FULLY_OPERATIONAL,		// At least one component of a particular type is fully operational. Others are dangling or blown off.
		COMPONENT_STATUS_SOME_PARTIALLY_OPERATIONAL,	// At least one component of a particular type is dangling. Others are dangling or blown off. None are fully operational.
		COMPONENT_STATUS_NONE_OPERATIONAL,				// All of the components of a particular type have been blown off.

		COMPONENT_STATUS_COUNT
	} ComponentStatus_e;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum {
		LIMB_STATE_FLAG_INVINCIBLE			= 0x01,		// This limb cannot be damaged (unless bot is destroyed)
		LIMB_STATE_FLAG_REPAIR_WHEN_STOLEN	= 0x02,		// When the pool is dry, repair this limb type instead of removing it

		LIMB_STATE_FLAG_NONE				= 0x00
	};


	struct PartState_t {
		CBotPart *pDanglingBotPart;						// If a part is dangling, this points to the part
	};


	struct LimbState_t {
		u8 nState;										// State of this limb (see LimbState_e for info)
		u8 nFlags;										// See LIMB_STATE_FLAG_* for info
		f32 fNormHealth;								// Normalized health
	};


	struct BoneMap_t {
		u8 nBoneIndex;									// Bone index to be captured in the matrix palette callback function
		u8 nPartType;									// Part type
	};


	enum {
		FLAG_RIGHT_LEG_NOT_INTACT		= 0x01,			// TRUE when the right leg is dangling or has been blown off
		FLAG_LEFT_LEG_NOT_INTACT		= 0x02,			// TRUE when the left leg is dangling or has been blown off
		FLAG_BACK_BROKEN				= 0x04,			// TRUE when the back is dangling or blown off
		FLAG_TINT_DEBRIS_CHUNKS			= 0x08,			// TRUE if the debris chunks are to be tinted with m_nTintRed, m_nTintGreen, and m_nTintBlue

		FLAG_NONE						= 0x00
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static CDamageResult m_DamageResult;				// Used to temporarily store a limb's damage result
	static CBotPartMgr *m_pCallbackPartMgr;				// Used during the selective kill debris callback

	CBot *m_pBot;										// Bot that owns this manager
	CBotPartPool *m_pPartPool;							// Pointer to this bot class's part pool

	f32 m_fBotInitialNormHealth;						// This is the bot's initial normalized health

	u8 m_nPartTypeCount;								// Number of elements in m_pPartStateArray
	u8 m_nLimbTypeCount;								// Number of elements in m_pLimbStateArray
	u8 m_nActiveBoneMapCount;							// Number of active entries in m_pBoneMapArray
	u8 m_nSeveredBoneWordCount;							// Number of elements in m_pnSeveredBoneBitmask
	u8 m_nSeveredBoneCount;								// Number of bits set in m_pnSeveredBoneBitmask
	u8 m_nFlags;										// See FLAG_* for info
	u8 m_nBoneGeoDebrisCount;							// Number of bone geo debris chunks flying around
	u8 m_nComponentStatus[COMPONENT_TYPE_COUNT];		// See ComponentStatus_e for possible values
	u8 m_nTintRed;										// Red tint value for debris chunks
	u8 m_nTintGreen;									// Green tint value for debris chunks
	u8 m_nTintBlue;										// Blue tint value for debris chunks

	PartState_t *m_pPartStateArray;						// One per part type
	LimbState_t *m_pLimbStateArray;						// One per limb type
	BoneMap_t *m_pBoneMapArray;							// Array of bone map data
	u32 *m_pnSeveredBoneBitmask;						// Bit fields (one per bone index) of bones that have been severed

	f32 m_fSecsUntilNextLimbBlowSound;
	f32 m_fSecsUntilNextDebrisBlowSound;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Creation:
	CBotPartMgr() { m_pPartPool = NULL; }
	~CBotPartMgr() { Destroy(); }

	BOOL Create( CBot *pBot, CBotPartPool **ppPartPool, cchar *pszGameDataFileName, u32 nPartsPerPoolCount, u32 nLimbTypeCount );
	void Destroy( void );
	FINLINE BOOL IsCreated( void ) const { return (m_pPartPool != NULL); }

	void SetDebrisChunkTintColor( const CFColorRGB *pTintColorRGB );


	// Work:
	void Work( void );


	// Actions:
	void ResetAllToIntact( void );

	void InflictLimbDamage( const CDamageData *pDamageData, const CDamageResult *pBotDamageResult );
	BOOL DestroyBot( const CDamageData *pDamageData );

	void SetState_Intact( u32 nLimbType );
	void SetState_BlowUp( u32 nLimbType, BOOL bDontBlowUpIfInvincible=TRUE );
	void SetState_Removed( u32 nLimbType, BOOL bSilentlyVanish=FALSE, BOOL bDontRemoveIfInvincible=TRUE );
	void SetState_Dangle( u32 nLimbType, BOOL bDontDangleIfInvincible=TRUE, f32 fGravity=-64.0f );
	BOOL SetRandomLimb_Dangle( BOOL bDontSelectInvincibleLimbs=TRUE );
	void SetAllLimbs_Dangle( BOOL bDontSelectInvincibleLimbs=TRUE );
	void RepairLimb( u32 nLimbType );

	FINLINE void MakeLimbInvincible( u32 nLimbType ) { FASSERT( IsCreated() ); FASSERT( nLimbType < m_nLimbTypeCount ); FMATH_SETBITMASK( m_pLimbStateArray[nLimbType].nFlags, LIMB_STATE_FLAG_INVINCIBLE ); }
	FINLINE void RepairWhenStolen( u32 nLimbType ) { FASSERT( IsCreated() ); FASSERT( nLimbType < m_nLimbTypeCount ); FMATH_SETBITMASK( m_pLimbStateArray[nLimbType].nFlags, LIMB_STATE_FLAG_REPAIR_WHEN_STOLEN ); }


	// Info:
	FINLINE ComponentStatus_e GetComponentStatus( ComponentType_e nComponentType ) const { FASSERT( IsCreated() ); FASSERT( nComponentType < COMPONENT_TYPE_COUNT ); return (ComponentStatus_e)m_nComponentStatus[nComponentType]; }
	FINLINE LimbState_e GetLimbState( u32 nLimbType ) const { FASSERT( IsCreated() ); FASSERT( nLimbType < m_nLimbTypeCount ); return (LimbState_e)m_pLimbStateArray[nLimbType].nState; }
	FINLINE f32 GetLimbNormHealth( u32 nLimbType ) const { FASSERT( IsCreated() ); FASSERT( nLimbType < m_nLimbTypeCount ); return m_pLimbStateArray[nLimbType].fNormHealth; }
	FINLINE u32 CurrentlyOwnsDebris( void ) const { FASSERT( IsCreated() ); return m_nBoneGeoDebrisCount > 0; }
	FINLINE const CBotPartPool* GetBotPartPool( void ) const { return m_pPartPool; }
	FINLINE s32 GetLimbTypeFromBoneIndex( u32 nBoneIndex ) const { FASSERT( IsCreated() ); FASSERT( m_pPartPool && m_pPartPool->IsCreated() ); return m_pPartPool->GetLimbTypeFromBoneIndex( nBoneIndex ); }
	FINLINE u8 GetLimbTypeCount( void ) const { FASSERT( IsCreated() ); return m_nLimbTypeCount; }
	FINLINE BOOL IsBackBroken( void ) const { FASSERT( IsCreated() ); return m_nFlags & FLAG_BACK_BROKEN; }
	FINLINE BOOL IsLimbInvincible( u32 uLimbType ) const { FASSERT( IsCreated() ); FASSERT( uLimbType < m_nLimbTypeCount ); return (m_pLimbStateArray[uLimbType].nFlags & LIMB_STATE_FLAG_INVINCIBLE); }

	// Work:
	FINLINE BOOL AnimBoneCallbackFunctionHandler( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL _TestPointInBox( const CFVec3A *pPos_WS, const CFMtx43A *pUnitMtx, const CFVec3A *pMinCorner_WS, const CFVec3A *pMaxCorner_WS );
	static BOOL _KillDebrisCallback( const CFDebris *pDebris );

	void _UpdateAllComponentStatus( void );
	void _UpdateOneComponentStatus( ComponentType_e nComponentType );

	void _Part_Dangle( u32 nPartType );
	void _Part_Disable( u32 nPartType );
	BOOL _AnimBoneCallbackFunctionHandler( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	void _InflictLimbDamage_Impact( const CDamageData *pDamageData );

	void _InflictLimbDamage_Blast( const CDamageData *pDamageData, const CDamageResult *pBotDamageResult );
	void _InflictLimbDamage_Blast_Dangling( u32 nLimbType, CDamageData *pDamageData, const CDamageResult *pBotDamageResult );
	void _ComputeBlastDamageResult_Intact( u32 nLimbType, CDamageData *pDamageData, f32 fDistToNearestBoneCenter2, const CFVec3A *pNearestBoneCenter_WS, f32 fNearestBoneRadius );
	BOOL _FindBoneNearestToBlastCenter( u32 nLimbType, const CDamageData *pDamageData,
										f32 *pfDistToNearestBoneCenter2, CFVec3A *pNearestBoneCenter_WS, f32 *pfNearestBoneRadius );

	void _ImpulseDanglingPart( u32 nPartType );
	void _InflictPartDamage( u32 nLimbType, const CDamageData *pDamageData );
	void _ComputeDamageResult( u32 nLimbType, const CDamageData *pDamageData );

	void _ApplyPopOffTorqueToLimb( u32 nLimbType );
	void _ApplyPopOffTorqueToPart( u32 nLimbType, u32 nPartType );

	void _SpawnLimbEffects( u32 nLimbType );

	void _KillBoneGeo( u32 nBoneIndex );
	void _SpawnBoneChunks( u32 nBoneIndex, const CFVec3A *pUnitDir_WS );
	void _TurnBoneIntoDebris( u32 nBoneIndex, BOOL bSilentlyVanish );

	static void _BoneGeoDebrisCallback( CFDebris *pDebris, CFDebrisDef::CallbackReason_e nReason, const FCollImpact_t *pCollImpact );
	static void _ChunkAndGutsDebrisCallback( CFDebris *pDebris, CFDebrisDef::CallbackReason_e nReason, const FCollImpact_t *pCollImpact );
	void _BlowOffBoneHierarchyIntoPieces( s32 nBaseBoneIndex, BOOL bNoLimping=FALSE, BOOL bSilentlyVanish=FALSE );
	void _BlowOffLimbIntoPieces( u32 nLimbType, BOOL bNoLimping=FALSE, BOOL bSilentlyVanish=FALSE );

	FINLINE void _SetSeveredBoneBitMask( u32 nBoneIndex );
	FINLINE void _ClearSeveredBoneBitMask( u32 nBoneIndex );
	FINLINE BOOL _GetSeveredBoneBitMask( u32 nBoneIndex ) const;

	friend class CBotPartPool;

	FCLASS_STACKMEM_ALIGN( CBotPartMgr );
} FCLASS_ALIGN_SUFFIX;




// Returns TRUE if we are driving the bone, or FALSE otherwise.
FINLINE BOOL CBotPartMgr::AnimBoneCallbackFunctionHandler( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	if( !IsCreated() ) {
		return FALSE;
	}

	if( (m_nActiveBoneMapCount == 0) && (m_nSeveredBoneCount == 0) ) {
		return FALSE;
	}

	return _AnimBoneCallbackFunctionHandler( nBoneIndex, rNewMtx, rParentMtx, rBoneMtx );
}


FINLINE void CBotPartMgr::_SetSeveredBoneBitMask( u32 nBoneIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nBoneIndex < m_pBot->m_pWorldMesh->m_pMesh->nBoneCount );

	if( !_GetSeveredBoneBitMask(nBoneIndex) ) {
		m_pnSeveredBoneBitmask[ nBoneIndex >> 5 ] |= (1 << (nBoneIndex & 31));
		++m_nSeveredBoneCount;
	}
}


FINLINE void CBotPartMgr::_ClearSeveredBoneBitMask( u32 nBoneIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nBoneIndex < m_pBot->m_pWorldMesh->m_pMesh->nBoneCount );

	if( _GetSeveredBoneBitMask(nBoneIndex) ) {
		m_pnSeveredBoneBitmask[ nBoneIndex >> 5 ] &= ~(1 << (nBoneIndex & 31));
		--m_nSeveredBoneCount;
	}
}


FINLINE BOOL CBotPartMgr::_GetSeveredBoneBitMask( u32 nBoneIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nBoneIndex < m_pBot->m_pWorldMesh->m_pMesh->nBoneCount );

	return (BOOL)((m_pnSeveredBoneBitmask[ nBoneIndex >> 5 ] >> (nBoneIndex & 31)) & 1);
}



#endif


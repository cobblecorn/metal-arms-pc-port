//////////////////////////////////////////////////////////////////////////////////////
// fAntenna.h - antenna animation module
//
// Author: Chris MacDonald
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
// 4/01/03  MacDonald   Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FANTENNA_H_
#define _FANTENNA_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fboxfilter.h"

class CFWorldMesh;

FCLASS_ALIGN_PREFIX class CFAntenna
{
//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

#define _INVALID_BONE_INDEX		( 0xffffffff )

	typedef struct
	{
		CFMtx43A mOr;			// WS orientation & position of bone
		f32 fLength;			// length of antenna segment that starts at this bone (last bone will have 0.0 length)
		u32 uBoneIndex;			// index of bone in mesh
		u32 uPad;				// necessary for 16 byte alignment of mOr matrix in array
		u32 uPad2;				// necessary for 16 byte alignment of mOr matrix in array
	} BoneData_t;

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	CFWorldMesh *m_pMesh;				// pointer to mesh containing antenna object
	BoneData_t *m_paBoneData;			// pointer to array of data for antenna bones
	CFVec3A m_vSpringPos;				// spring pos
	CFVec3A m_vSpringVel;				// spring vel
	CFVec3A m_vBasePos;					// position of antenna base
	CFVec3A m_vBaseVel;					// velocity of antenna base
	f32 m_fKSpring;						// spring constant
	f32 m_fKDamping;					// damping constant
	f32 m_fMagnitude;					// unitless magnitude of swaying effect
	f32 m_fOOMagnitude;					// inverse of magnitude
	u32 m_uNumBones;					// number of bones in this antenns
	f32 m_fOONumBones;					// one over number of bones
	BOOL m_bSetPos;						// TRUE for one frame if SetPos was called
	FBoxFilterHandle_t m_hFramerateFilter;	// filter for smoothing framerate (used to scale magnitude of swaying)

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CFAntenna();
	BOOL Create( CFWorldMesh *pMesh, u32 uNumBones, f32 fKSpring, f32 fKDamping, f32 fMagnitude );	// allocates memory and inits instance of CFAntenna.
	void Destroy( void );										// does nothing at the moment.
	void RegisterBones( s32 *paBones );							// registers bones with the antenna.
	void SetPos( void );										// sets the antenna at the current bone position and orientation with antenna at rest.
	void Work( void );											// updates the antenna position, orientation and physics.  Call every frame.
	s32 FindPositionFromBoneIndex( u32 uBoneIndex );			// if uBoneIndex is in Antenna, returns position in antenna (0 = root), or -1 if not found
	BOOL AnimateBone( u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );	// anim bone callback function to animate an antenna bone
	FINLINE void SetSpringConstant( f32 fKSpring ) { m_fKSpring = fKSpring; };
	FINLINE void SetSpringDamping( f32 fKDamping ) { m_fKDamping = fKDamping; };
	FINLINE void SetEffectMagnitude( f32 fMagnitude ) { m_fMagnitude = fMagnitude; m_fOOMagnitude = 1.0f / fMagnitude; };

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	BOOL _ValidateRegistration( void );

	FCLASS_STACKMEM_ALIGN( CFAntenna );
} FCLASS_ALIGN_SUFFIX;

#endif // _FANTENNA_H_
//////////////////////////////////////////////////////////////////////////////////////
// EParticlePool.h - a generic pool of CEParticles
//
// Author: Mike Starich      
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
// 02/06/03 Starich        Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _EPARTICLEPOOL_H_
#define _EPARTICLEPOOL_H_ 1

#include "fang.h"
#include "eparticle.h"

#define EPARTICLE_POOL_COUNT	50

extern BOOL eparticlepool_InitSystem( void );
extern void eparticlepool_UninitSystem( void );

// called before a level, will Create each entry in the pool
extern BOOL eparticlepool_InitLevel( void );

// called after the level is finished, will Destroy all emitters and return them to the pool
extern void eparticlepool_UninitLevel( void );

// called to get a temp emitter, once this emitter has stopped emitting particles it will be reused automatically
extern CEParticle *eparticlepool_GetEmitter( void );

#endif

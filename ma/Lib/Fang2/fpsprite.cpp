//////////////////////////////////////////////////////////////////////////////////////
// fpsprite.cpp - 
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
// 08/01/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fpsprite.h"
 

void CFPSpriteGroup::ComputeBoundingSphere() {
	CFVec3 Min, Max;

	if( m_nRenderCount == 0 ) {
		return;
	}
	// create a min/max box about all of the sprites
	ComputeMinMaxBox( Min, Max );
	
	// build the bounding sphere from the min/max box
	m_BoundSphere_MS.BuildFromMinMaxBox( Min, Max );
}

void CFPSpriteGroup::ComputeMinMaxBox( CFVec3 &rMin, CFVec3 &rMax ) const {
	u32 i, nLeftOver;
	const FPSprite_t *pSprite = &m_pBase[m_nRenderStartIndex];

	if( m_nRenderCount == 0 ) {
		return;
	}
	// set the min/max to the 1st sprite
	rMin = pSprite->Point_MS;
	rMax = rMin;
	// walk the rest of the sprites
	nLeftOver = m_nRenderStartIndex + m_nRenderCount;
	if( nLeftOver > m_nMaxCount ) {
		// 2 loops
		for( i=m_nRenderStartIndex+1; i < m_nMaxCount; i++ ) {
			pSprite++;
			rMin.Min( pSprite->Point_MS );
			rMax.Max( pSprite->Point_MS );
		}
		nLeftOver -= m_nMaxCount;
		pSprite = m_pBase;
		for( i=0; i < nLeftOver; i++ ) {
			rMin.Min( pSprite->Point_MS );
			rMax.Max( pSprite->Point_MS );
			pSprite++;
		}
	} else {
		for( i=1; i < m_nRenderCount; i++ ) {
			pSprite++;
			rMin.Min( pSprite->Point_MS );
			rMax.Max( pSprite->Point_MS );
		}
	}
}



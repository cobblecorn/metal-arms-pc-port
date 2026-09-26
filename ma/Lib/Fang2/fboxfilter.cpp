//////////////////////////////////////////////////////////////////////////////////////
// ffboxfilter.cpp - Fang box filter module.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 11/28/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fboxfilter.h"
#include "fmath.h"
#include "fres.h"



typedef struct {	
	u32 nBufferMaxElements;
	u32 *pnBuffer;

	u32 nMaxElements;
	u32 nElementCount;
	u32 nNextElement;
	u32 nLastValue;
	u32 nRunningSum;
} FBoxFilter_u32_t;

typedef struct {
	u32 nBufferMaxElements;
	s32 *pnBuffer;

	u32 nMaxElements;
	u32 nElementCount;
	u32 nNextElement;
	u32 nLastValue;
	s32 nRunningSum;
} FBoxFilter_s32_t;

typedef struct {
	u32 nBufferMaxElements;
	f32 *pfBuffer;

	u32 nMaxElements;
	u32 nElementCount;
	u32 nNextElement;
	f32 fLastValue;
	f32 fRunningSum;
} FBoxFilter_f32_t;





static BOOL _bModuleInitialized;

BOOL fboxfilter_ModuleStartup( void ) {
	FASSERT( !_bModuleInitialized );

	_bModuleInitialized = TRUE;

	return TRUE;
}

void fboxfilter_ModuleShutdown( void ) {
	FASSERT( _bModuleInitialized );

	_bModuleInitialized = FALSE;
}



//--------------------------------------------------------------------------------------------------------------------
// u32 functions:

FBoxFilterHandle_t fboxfilter_Create_u32( u32 nMaxElements ) {
	FBoxFilter_u32_t *pBoxFilter_u32;
	FResFrame_t ResFrame;

	FASSERT( _bModuleInitialized );

	ResFrame = fres_GetFrame();

	pBoxFilter_u32 = (FBoxFilter_u32_t *)fres_Alloc( sizeof(FBoxFilter_u32_t) );
	if( pBoxFilter_u32 == NULL ) {
		goto _ExitCreateWithError;
	}

	pBoxFilter_u32->pnBuffer = (u32 *)fres_Alloc( sizeof(u32) * nMaxElements );
	if( pBoxFilter_u32->pnBuffer == NULL ) {
		goto _ExitCreateWithError;
	}

	pBoxFilter_u32->nBufferMaxElements = nMaxElements;
	pBoxFilter_u32->nMaxElements = nMaxElements;
	pBoxFilter_u32->nElementCount = 0;
	pBoxFilter_u32->nNextElement = 0;
	pBoxFilter_u32->nLastValue = 0;
	pBoxFilter_u32->nRunningSum = 0;

	return (FBoxFilterHandle_t)pBoxFilter_u32;

_ExitCreateWithError:
	// Error...
	fres_ReleaseFrame( ResFrame );
	return NULL;
}

void fboxfilter_Reset_u32( FBoxFilterHandle_t hBoxFilter ) {
	FBoxFilter_u32_t *pBoxFilter_u32;

	FASSERT( _bModuleInitialized );

	pBoxFilter_u32 = (FBoxFilter_u32_t *)hBoxFilter;

	pBoxFilter_u32->nElementCount = 0;
	pBoxFilter_u32->nNextElement = 0;
	pBoxFilter_u32->nLastValue = 0;
	pBoxFilter_u32->nRunningSum = 0;
}

u32 fboxfilter_GetFilterElementCount_u32( FBoxFilterHandle_t hBoxFilter ) {
	FBoxFilter_u32_t *pBoxFilter_u32;

	FASSERT( _bModuleInitialized );

	pBoxFilter_u32 = (FBoxFilter_u32_t *)hBoxFilter;

	return pBoxFilter_u32->nMaxElements;
}

u32 fboxfilter_SetFilterElementCount_u32( FBoxFilterHandle_t hBoxFilter, u32 nFilterElementCount ) {
	FBoxFilter_u32_t *pBoxFilter_u32;
	u32 nPreviousValue;

	FASSERT( _bModuleInitialized );

	pBoxFilter_u32 = (FBoxFilter_u32_t *)hBoxFilter;

	nPreviousValue = pBoxFilter_u32->nMaxElements;
	pBoxFilter_u32->nMaxElements = FMATH_MIN( nFilterElementCount, pBoxFilter_u32->nBufferMaxElements );
	fboxfilter_Reset_u32( hBoxFilter );

	return nPreviousValue;
}

void fboxfilter_Add_u32( FBoxFilterHandle_t hBoxFilter, u32 nValue ) {
	FBoxFilter_u32_t *pBoxFilter_u32;

	FASSERT( _bModuleInitialized );

	pBoxFilter_u32 = (FBoxFilter_u32_t *)hBoxFilter;

	// Update running sum...
	if( pBoxFilter_u32->nElementCount == pBoxFilter_u32->nMaxElements ) {
		pBoxFilter_u32->nRunningSum -= pBoxFilter_u32->pnBuffer[ pBoxFilter_u32->nNextElement ];
	} else {
		pBoxFilter_u32->nElementCount++;
	}
	pBoxFilter_u32->nRunningSum += nValue;

	// Store value into buffer...
	pBoxFilter_u32->pnBuffer[ pBoxFilter_u32->nNextElement ] = nValue;
	pBoxFilter_u32->nLastValue = nValue;

	// Update next element index...
	pBoxFilter_u32->nNextElement++;
	if( pBoxFilter_u32->nNextElement == pBoxFilter_u32->nMaxElements ) {
		pBoxFilter_u32->nNextElement = 0;
	}
}

void fboxfilter_Get_u32( FBoxFilterHandle_t hBoxFilter,
					   u32 *pnLastValue, u32 *pnAvgValue, float *pfAvgValue, u32 *pnSum, u32 *pnNumValues ) {
	FBoxFilter_u32_t *pBoxFilter_u32;

	FASSERT( _bModuleInitialized );

	pBoxFilter_u32 = (FBoxFilter_u32_t *)hBoxFilter;

	if( pnLastValue ) {
		*pnLastValue = pBoxFilter_u32->nLastValue;
	}

	if( pnAvgValue ) {
		if( pBoxFilter_u32->nElementCount ) {
			*pnAvgValue = pBoxFilter_u32->nRunningSum / pBoxFilter_u32->nElementCount;
		} else {
			*pnAvgValue = 0;
		}
	}

	if( pfAvgValue ) {
		if( pBoxFilter_u32->nElementCount ) {
			*pfAvgValue = (float)pBoxFilter_u32->nRunningSum / (float)pBoxFilter_u32->nElementCount;
		} else {
			*pfAvgValue = 0;
		}
	}

	if( pnSum ) {
		*pnSum = pBoxFilter_u32->nRunningSum;
	}

	if( pnNumValues ) {
		*pnNumValues = pBoxFilter_u32->nElementCount;
	}
}




//--------------------------------------------------------------------------------------------------------------------
// s32 functions:

FBoxFilterHandle_t fboxfilter_Create_s32( u32 nMaxElements ) {
	FBoxFilter_s32_t *pBoxFilter_s32;
	FResFrame_t ResFrame;

	FASSERT( _bModuleInitialized );

	ResFrame = fres_GetFrame();

	pBoxFilter_s32 = (FBoxFilter_s32_t *)fres_Alloc( sizeof(FBoxFilter_s32_t) );
	if( pBoxFilter_s32 == NULL ) {
		goto _ExitCreateWithError;
	}

	pBoxFilter_s32->pnBuffer = (s32 *)fres_Alloc( sizeof(s32) * nMaxElements );
	if( pBoxFilter_s32->pnBuffer == NULL ) {
		goto _ExitCreateWithError;
	}

	pBoxFilter_s32->nBufferMaxElements = nMaxElements;
	pBoxFilter_s32->nMaxElements = nMaxElements;
	pBoxFilter_s32->nElementCount = 0;
	pBoxFilter_s32->nNextElement = 0;
	pBoxFilter_s32->nLastValue = 0;
	pBoxFilter_s32->nRunningSum = 0;

	return (FBoxFilterHandle_t)pBoxFilter_s32;

_ExitCreateWithError:
	// Error...
	fres_ReleaseFrame( ResFrame );
	return NULL;
}

void fboxfilter_Reset_s32( FBoxFilterHandle_t hBoxFilter ) {
	FBoxFilter_s32_t *pBoxFilter_s32;

	FASSERT( _bModuleInitialized );

	pBoxFilter_s32 = (FBoxFilter_s32_t *)hBoxFilter;

	pBoxFilter_s32->nElementCount = 0;
	pBoxFilter_s32->nNextElement = 0;
	pBoxFilter_s32->nLastValue = 0;
	pBoxFilter_s32->nRunningSum = 0;
}

u32 fboxfilter_GetFilterElementCount_s32( FBoxFilterHandle_t hBoxFilter ) {
	FBoxFilter_s32_t *pBoxFilter_s32;

	FASSERT( _bModuleInitialized );

	pBoxFilter_s32 = (FBoxFilter_s32_t *)hBoxFilter;

	return pBoxFilter_s32->nMaxElements;
}

u32 fboxfilter_SetFilterElementCount_s32( FBoxFilterHandle_t hBoxFilter, u32 nFilterElementCount ) {
	FBoxFilter_s32_t *pBoxFilter_s32;
	u32 nPreviousValue;

	FASSERT( _bModuleInitialized );

	pBoxFilter_s32 = (FBoxFilter_s32_t *)hBoxFilter;

	nPreviousValue = pBoxFilter_s32->nMaxElements;
	pBoxFilter_s32->nMaxElements = FMATH_MIN( nFilterElementCount, pBoxFilter_s32->nBufferMaxElements );
	fboxfilter_Reset_s32( hBoxFilter );

	return nPreviousValue;
}

void fboxfilter_Add_s32( FBoxFilterHandle_t hBoxFilter, s32 nValue ) {
	FBoxFilter_s32_t *pBoxFilter_s32;

	FASSERT( _bModuleInitialized );

	pBoxFilter_s32 = (FBoxFilter_s32_t *)hBoxFilter;

	// Update running sum...
	if( pBoxFilter_s32->nElementCount == pBoxFilter_s32->nMaxElements ) {
		pBoxFilter_s32->nRunningSum -= pBoxFilter_s32->pnBuffer[ pBoxFilter_s32->nNextElement ];
	} else {
		pBoxFilter_s32->nElementCount++;
	}
	pBoxFilter_s32->nRunningSum += nValue;

	// Store value into buffer...
	pBoxFilter_s32->pnBuffer[ pBoxFilter_s32->nNextElement ] = nValue;
	pBoxFilter_s32->nLastValue = nValue;

	// Update next element index...
	pBoxFilter_s32->nNextElement++;
	if( pBoxFilter_s32->nNextElement == pBoxFilter_s32->nMaxElements ) {
		pBoxFilter_s32->nNextElement = 0;
	}
}

void fboxfilter_Get_s32( FBoxFilterHandle_t hBoxFilter,
					   s32 *pnLastValue, s32 *pnAvgValue, float *pfAvgValue, s32 *pnSum, u32 *pnNumValues ) {
	FBoxFilter_s32_t *pBoxFilter_s32;

	FASSERT( _bModuleInitialized );

	pBoxFilter_s32 = (FBoxFilter_s32_t *)hBoxFilter;

	if( pnLastValue ) {
		*pnLastValue = pBoxFilter_s32->nLastValue;
	}

	if( pnAvgValue ) {
		if( pBoxFilter_s32->nElementCount ) {
			*pnAvgValue = pBoxFilter_s32->nRunningSum / pBoxFilter_s32->nElementCount;
		} else {
			*pnAvgValue = 0;
		}
	}

	if( pfAvgValue ) {
		if( pBoxFilter_s32->nElementCount ) {
			*pfAvgValue = (float)pBoxFilter_s32->nRunningSum / (float)pBoxFilter_s32->nElementCount;
		} else {
			*pfAvgValue = 0;
		}
	}

	if( pnSum ) {
		*pnSum = pBoxFilter_s32->nRunningSum;
	}

	if( pnNumValues ) {
		*pnNumValues = pBoxFilter_s32->nElementCount;
	}
}




//--------------------------------------------------------------------------------------------------------------------
// f32 functions:

FBoxFilterHandle_t fboxfilter_Create_f32( u32 nMaxElements ) {
	FBoxFilter_f32_t *pBoxFilter_f32;
	FResFrame_t ResFrame;

	FASSERT( _bModuleInitialized );

	ResFrame = fres_GetFrame();

	pBoxFilter_f32 = (FBoxFilter_f32_t *)fres_Alloc( sizeof(FBoxFilter_f32_t) );
	if( pBoxFilter_f32 == NULL ) {
		goto _ExitCreateWithError;
	}

	pBoxFilter_f32->pfBuffer = (f32 *)fres_Alloc( sizeof(f32) * nMaxElements );
	if( pBoxFilter_f32->pfBuffer == NULL ) {
		goto _ExitCreateWithError;
	}

	pBoxFilter_f32->nBufferMaxElements = nMaxElements;
	pBoxFilter_f32->nMaxElements = nMaxElements;
	pBoxFilter_f32->nElementCount = 0;
	pBoxFilter_f32->nNextElement = 0;
	pBoxFilter_f32->fLastValue = 0.0f;
	pBoxFilter_f32->fRunningSum = 0.0f;

	return (FBoxFilterHandle_t)pBoxFilter_f32;

_ExitCreateWithError:
	// Error...
	fres_ReleaseFrame( ResFrame );
	return NULL;
}

void fboxfilter_Reset_f32( FBoxFilterHandle_t hBoxFilter ) {
	FBoxFilter_f32_t *pBoxFilter_f32;

	FASSERT( _bModuleInitialized );

	pBoxFilter_f32 = (FBoxFilter_f32_t *)hBoxFilter;

	pBoxFilter_f32->nElementCount = 0;
	pBoxFilter_f32->nNextElement = 0;
	pBoxFilter_f32->fLastValue = 0;
	pBoxFilter_f32->fRunningSum = 0;
}

u32 fboxfilter_GetFilterElementCount_f32( FBoxFilterHandle_t hBoxFilter ) {
	FBoxFilter_f32_t *pBoxFilter_f32;

	FASSERT( _bModuleInitialized );

	pBoxFilter_f32 = (FBoxFilter_f32_t *)hBoxFilter;

	return pBoxFilter_f32->nMaxElements;
}

u32 fboxfilter_SetFilterElementCount_f32( FBoxFilterHandle_t hBoxFilter, u32 nFilterElementCount ) {
	FBoxFilter_f32_t *pBoxFilter_f32;
	u32 nPreviousValue;

	FASSERT( _bModuleInitialized );

	pBoxFilter_f32 = (FBoxFilter_f32_t *)hBoxFilter;

	nPreviousValue = pBoxFilter_f32->nMaxElements;
	pBoxFilter_f32->nMaxElements = FMATH_MIN( nFilterElementCount, pBoxFilter_f32->nBufferMaxElements );
	fboxfilter_Reset_f32( hBoxFilter );

	return nPreviousValue;
}

void fboxfilter_Add_f32( FBoxFilterHandle_t hBoxFilter, f32 fValue ) {
	FBoxFilter_f32_t *pBoxFilter_f32;

	FASSERT( _bModuleInitialized );

	pBoxFilter_f32 = (FBoxFilter_f32_t *)hBoxFilter;

	// Update running sum...
	if( pBoxFilter_f32->nElementCount == pBoxFilter_f32->nMaxElements ) {
		pBoxFilter_f32->fRunningSum -= pBoxFilter_f32->pfBuffer[ pBoxFilter_f32->nNextElement ];
	} else {
		pBoxFilter_f32->nElementCount++;
	}
	pBoxFilter_f32->fRunningSum += fValue;

	// Store value into buffer...
	pBoxFilter_f32->pfBuffer[ pBoxFilter_f32->nNextElement ] = fValue;
	pBoxFilter_f32->fLastValue = fValue;

	// Update next element index...
	pBoxFilter_f32->nNextElement++;
	if( pBoxFilter_f32->nNextElement == pBoxFilter_f32->nMaxElements ) {
		pBoxFilter_f32->nNextElement = 0;
	}
}

void fboxfilter_Get_f32( FBoxFilterHandle_t hBoxFilter,
					   f32 *pfLastValue, f32 *pfAvgValue, f32 *pfSum, u32 *pnNumValues ) {
	FBoxFilter_f32_t *pBoxFilter_f32;

	FASSERT( _bModuleInitialized );

	pBoxFilter_f32 = (FBoxFilter_f32_t *)hBoxFilter;

	if( pfLastValue ) {
		*pfLastValue = pBoxFilter_f32->fLastValue;
	}

	if( pfAvgValue ) {
		if( pBoxFilter_f32->nElementCount ) {
			*pfAvgValue = pBoxFilter_f32->fRunningSum / (float)pBoxFilter_f32->nElementCount;
		} else {
			*pfAvgValue = 0;
		}
	}

	if( pfSum ) {
		*pfSum = pBoxFilter_f32->fRunningSum;
	}

	if( pnNumValues ) {
		*pnNumValues = pBoxFilter_f32->nElementCount;
	}
}


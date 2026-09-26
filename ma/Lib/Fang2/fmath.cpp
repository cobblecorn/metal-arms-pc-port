//////////////////////////////////////////////////////////////////////////////////////
// fmath.cpp - Fang math module.
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
// 10/02/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fmath.h"



#define _RANDOM_SEED2	0xb78d0945


//u32 FMath_nRandomSeed1 = FMATH_DEFAULT_RANDOM_SEED;
//u32 FMath_nRandomSeed2 = _RANDOM_SEED2;


#if FANG_PLATFORM_DX
	f32 FMath_afInterpTable1[FMATH_SINTBL_ROUND_TABLE_COUNT];
	f32 FMath_afInterpTable2[FMATH_SINTBL_ROUND_TABLE_COUNT];
#endif



static const u32 __auCRCs[ 256 ] = 
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
	0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
	0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
	0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
	0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
	0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
	0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
	0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
	0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
	0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};


const f32 FMath_afAtanTable[]= 
{
    0.000000f, 0.007812f, 0.015624f, 0.023433f,
    0.031240f, 0.039043f, 0.046841f, 0.054633f,
    0.062419f, 0.070197f, 0.077967f, 0.085727f,
    0.093477f, 0.101215f, 0.108942f, 0.116655f,
    0.124355f, 0.132040f, 0.139709f, 0.147361f,
    0.154997f, 0.162614f, 0.170212f, 0.177790f,
    0.185348f, 0.192884f, 0.200399f, 0.207890f,
    0.215358f, 0.222801f, 0.230220f, 0.237612f,
    0.244979f, 0.252318f, 0.259630f, 0.266913f,
    0.274167f, 0.281392f, 0.288587f, 0.295752f,
    0.302885f, 0.309986f, 0.317056f, 0.324092f,
    0.331096f, 0.338066f, 0.345002f, 0.351904f,
    0.358771f, 0.365602f, 0.372398f, 0.379159f,
    0.385883f, 0.392570f, 0.399221f, 0.405834f,
    0.412410f, 0.418949f, 0.425450f, 0.431912f,
    0.438337f, 0.444722f, 0.451070f, 0.457378f,
    0.463648f, 0.469878f, 0.476069f, 0.482221f,
    0.488334f, 0.494407f, 0.500441f, 0.506435f,
    0.512389f, 0.518304f, 0.524180f, 0.530015f,
    0.535811f, 0.541568f, 0.547284f, 0.552962f,
    0.558599f, 0.564198f, 0.569756f, 0.575276f,
    0.580756f, 0.586198f, 0.591600f, 0.596963f,
    0.602287f, 0.607573f, 0.612820f, 0.618029f,
    0.623199f, 0.628332f, 0.633426f, 0.638482f,
    0.643501f, 0.648482f, 0.653426f, 0.658333f,
    0.663203f, 0.668036f, 0.672833f, 0.677593f,
    0.682317f, 0.687005f, 0.691657f, 0.696273f,
    0.700854f, 0.705400f, 0.709912f, 0.714388f,
    0.718830f, 0.723238f, 0.727611f, 0.731951f,
    0.736257f, 0.740530f, 0.744770f, 0.748977f,
    0.753151f, 0.757293f, 0.761403f, 0.765480f,
    0.769526f, 0.773541f, 0.777524f, 0.781477f,
    0.785398f,
};


const FMath_AtanCoefInfo_t FMath_aAtanQuadrantTable[]= 
{
    {  1.0f, FMATH_DEG2RAD(  0.0f) },				/* 0-45    */
    { -1.0f, FMATH_DEG2RAD( 90.0f) },				/* 45-90   */
    { -1.0f, FMATH_DEG2RAD(180.0f) },				/* 135-180 */
    {  1.0f, FMATH_DEG2RAD( 90.0f) },				/* 90-135  */
    { -1.0f, FMATH_DEG2RAD(360.0f)-FMATH_2PI },		/* 315-360 */
    {  1.0f, FMATH_DEG2RAD(270.0f)-FMATH_2PI },		/* 270-315 */
    {  1.0f, FMATH_DEG2RAD(180.0f)-FMATH_2PI },		/* 180-225 */
    { -1.0f, FMATH_DEG2RAD(270.0f)-FMATH_2PI },		/* 225-270 */
};


static BOOL _bModuleInitialized;


extern void fmath_RandomInit( void );


//
//
//
BOOL fmath_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	_bModuleInitialized = TRUE;

//	fmath_SetRandomSeed( FMATH_DEFAULT_RANDOM_SEED );
	#if FANG_PLATFORM_DX
		u32 i;
		for( i=0; i<FMATH_SINTBL_ROUND_TABLE_COUNT; i++ ) 
		{
			FMath_afInterpTable2[i] = (f32)i / (f32)FMATH_SINTBL_ROUND_TABLE_COUNT;
			FMath_afInterpTable1[i] = 1.0f - FMath_afInterpTable2[i];
		}
	#endif

	fmath_RandomInit();

	return TRUE;
}


//
//
//
void fmath_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	_bModuleInitialized = FALSE;
}


//
// Sets the random seed.
//
void fmath_SetRandomSeed( u32 nNewSeed ) 
{
	FASSERT( _bModuleInitialized );
#if 0
	FMath_nRandomSeed1 = nNewSeed;
	FMath_nRandomSeed2 = _RANDOM_SEED2;
#endif
}


//
// Checks the binary format of the specified ANSI floating point number and returns
// a result (see FMathFcheckResult_e for info).
//
FMathFcheckResult_e fmath_Fcheck( f32 f ) 
{
	FMathFcheckResult_e nReturnCode;
	s32 nSign, nExp, nFrac;
	u32 i;

	nReturnCode = FMATH_FCHECK_RESULT_OK;

	i = *(u32 *)&f;

	nSign = (s32)( i>>31 );
	nExp = (s32)( ((i & 0x7f800000)>>23)-127 );
	nFrac = (s32)( (i & 0x007fffff) );

	if ( nExp == 128 ) 
	{
		if( nFrac ) 
		{
			// Not-a-number or Q-Not-a-number...
			nReturnCode = (nFrac & 0x400000) ? FMATH_FCHECK_RESULT_SNAN : FMATH_FCHECK_RESULT_QNAN;
		} 
		else 
		{
			// -Infinity or +Infinity
			nReturnCode = nSign ? FMATH_FCHECK_RESULT_NINF : FMATH_FCHECK_RESULT_INF;
		}
	}

	// Do it this way so breakpoints can be set...
	if ( nReturnCode == FMATH_FCHECK_RESULT_OK ) 
	{
		// No error...
		return FMATH_FCHECK_RESULT_OK;
	} 
	else 
	{
		// Error in floating point binary format...
		return nReturnCode;
	}
}


//
//
//
u32 fmath_Crc32( u32 nCrc, const u8 *pnData, u32 nDataSize ) 
{
	if( pnData == NULL ) 
	{
		return 0;
	}

	#define __DO1( p )	nCrc = __auCRCs[ ( nCrc ^ ( *pnData++ ) ) & 0xff ] ^ ( nCrc >> 8 );
	#define __DO2( p )	__DO1( p ); __DO1( p );
	#define __DO4( p )	__DO2( p ); __DO2( p );
	#define __DO8( p )	__DO4( p ); __DO4( p );

	nCrc = nCrc ^ 0xffffffff;
	while ( nDataSize >= 8 ) 
	{
		__DO8( pnData );
		nDataSize -= 8;
	}
	if ( nDataSize ) 
	{
		do 
		{
			__DO1( pnData );
		} while( --nDataSize );
	}

	return nCrc ^ 0xffffffff;

	#undef __DO1
	#undef __DO2
	#undef __DO4
	#undef __DO8
}




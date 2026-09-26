//////////////////////////////////////////////////////////////////////////////////////
// fdx8cpu.cpp - Fang CPU module (DX8 version).
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
#include "fcpu.h"

#if FANG_PLATFORM_WIN
#include <windows.h>
#endif

#if FANG_PLATFORM_XB
#include <xtl.h>
#endif



FCpu_Info_t FCpu_Info;
FCpu_Intel_Info_t FCpu_IntelInfo;



#define _FREQ_TOLERANCE_MHZ		1		// Number of MHz to allow samplings to deviate from average of samplings

#define _CPUID_CHARCODE(a,b,c,d) ( (d<<24) | (c<<16) | (b<<8) | a )


typedef struct {
	u32 nEAX;
	u32 nEBX;
	u32 nECX;
	u32 nEDX;
} _CpuidInfo_t;

typedef struct {
	u32 nRawMhz;				// Raw CPU frequency in MHz
	u32 nNormalizedMhz;			// Normalized CPU frequency in MHz
} _FreqInfo_t;



static BOOL _bModuleInitialized;


static void _InvalidateGlobalStructures( void );
static BOOL _Install_Intel( void );
static void _CPUID( u32 nInputEAX, _CpuidInfo_t *pCpuidInfo );
static void _GetIntelInfo( void );
static void _RegisterCacheInfoReg( u32 nRegister, BOOL bSkipLowByte );
static void _RegisterCacheInfoByte( u8 nByte );
static BOOL _IsCPUIDInstructionSupported( void );
static BOOL _IsCPUAn8086( void );
static BOOL _IsCPUAn80286( void );
static BOOL _IsCPUAn80386( void );
static float _MeasureFreqRDTSC( void );



BOOL fcpu_ModuleStartup( void ) {
	FASSERT( !_bModuleInitialized );

	_InvalidateGlobalStructures();

	if( !_Install_Intel() ) {
		// Catastrophic failure. Exit with error...
		DEVPRINTF( "fcpu_ModuleStartup(): Could not detect CPU.\n" );
		return FALSE;
	}

	_bModuleInitialized = TRUE;

	if( FCpu_IntelInfo.bSupports_CPUID && (FCpu_IntelInfo.CPUID.nMaxCPUIDInputValue > 0) ) {
		if( FCpu_IntelInfo.CPUID.CPUID1.bTimeStampCounter ) {
			DEVPRINTF( "%uMHz %s CPU detected%s.\n",
				(u32)(FCpu_IntelInfo.fCyclesPerSecond_RDTSC*(1.0f/1000000.0f)),
				drvcpu_intel_GetCpuCodeString(),
				FCpu_IntelInfo.CPUID.bGenuineIntel ? " (Genuine Intel)" : ""
			);
		} else {
			DEVPRINTF( "%s CPU detected%s.\n",
				drvcpu_intel_GetCpuCodeString(),
				FCpu_IntelInfo.CPUID.bGenuineIntel ? " (Genuine Intel)" : ""
			);
			DEVPRINTF( "WARNING: RDTSC not supported by this CPU!\n" );
			DEVPRINTF( "         Fang cannot function.\n" );
		}

		if( !FCpu_IntelInfo.CPUID.CPUID1.bSIMDTechnology ) {
			DEVPRINTF( "WARNING: SIMD is not supported by this CPU!\n" );
			DEVPRINTF( "         Fang cannot function.\n" );
		}

		if( !FCpu_IntelInfo.bSIMDSupportedByOS ) {
			DEVPRINTF( "WARNING: SIMD is not supported by this OS!\n" );
			DEVPRINTF( "         Fang cannot function.\n" );
		}

		if( !FCpu_IntelInfo.CPUID.CPUID1.bMMXTechnology ) {
			DEVPRINTF( "WARNING: MMX is not supported by this CPU!\n" );
			DEVPRINTF( "         Fang may not run at its optimal performance.\n" );
		}
	} else {
		DEVPRINTF( "%s CPU detected.\n", drvcpu_intel_GetCpuCodeString() );
		DEVPRINTF( "WARNING: CPUID1 is not supported by this CPU!\n" );
		DEVPRINTF( "         Fang cannot function.\n" );
	}

	return TRUE;
}

// Uninstalls the CPU driver.
void fcpu_ModuleShutdown( void ) {
	FASSERT( _bModuleInitialized );

	_InvalidateGlobalStructures();

	_bModuleInitialized = FALSE;
}

// Returns a pointer to a character string with the CPU's code name in it.
const char *drvcpu_intel_GetCpuCodeString( void ) {
	const char *_aszCpuCodeString[FCPU_INTEL_CPUCODE_COUNT] = {
		"Unknown",
		"8086",
		"80286",
		"80386",
		"80486",
		"Pentium",
		"PentiumPro",
		"PentiumII",
		"PentiumIII",
	};

	FASSERT( _bModuleInitialized );

	if( FCpu_IntelInfo.nCpuCode < FCPU_INTEL_CPUCODE_COUNT ) {
		return _aszCpuCodeString[FCpu_IntelInfo.nCpuCode];
	} else {
		return "Unknown";
	}
}



static void _InvalidateGlobalStructures( void ) {
	fang_MemZero( &FCpu_Info, sizeof(FCpu_Info_t) );
	FCpu_Info.CacheD.bCacheDataValid = FALSE;
	FCpu_Info.CacheI.bCacheDataValid = FALSE;
	FCpu_Info.CacheL2.bCacheDataValid = FALSE;
}

static BOOL _Install_Intel( void ) {
	_GetIntelInfo();

	#if FANG_PLATFORM_XB
		FCpu_IntelInfo.fCyclesPerSecond_RDTSC = 733333333.333333333333333f;
	#else
		FCpu_IntelInfo.fCyclesPerSecond_RDTSC = _MeasureFreqRDTSC();
	#endif

	return TRUE;
}

static void _GetIntelInfo( void ) {
	u32 i, nIterations;
	_CpuidInfo_t Cpuid;

	fang_MemZero( &FCpu_IntelInfo, sizeof(FCpu_Intel_Info_t) );

	if( !_IsCPUIDInstructionSupported() ) {
		// CPUID instruction is NOT supported...

		FCpu_IntelInfo.bSupports_CPUID = FALSE;

		if( _IsCPUAn8086() ) {
			FCpu_IntelInfo.nCpuCode = FCPU_INTEL_CPUCODE_8086;
		} else if( _IsCPUAn80286() ) {
			FCpu_IntelInfo.nCpuCode = FCPU_INTEL_CPUCODE_80286;
		} else if( _IsCPUAn80386() ) {
			FCpu_IntelInfo.nCpuCode = FCPU_INTEL_CPUCODE_80386;
		} else {
			// Must be an 80486...
			FCpu_IntelInfo.nCpuCode = FCPU_INTEL_CPUCODE_80486;
		}
	} else {
		// CPUID instruction is supported...
		FCpu_IntelInfo.bSupports_CPUID = TRUE;

		_CPUID( 0, &Cpuid );
		FCpu_IntelInfo.CPUID.nRawEAX0 = Cpuid.nEAX;
		FCpu_IntelInfo.CPUID.nRawEBX0 = Cpuid.nEBX;
		FCpu_IntelInfo.CPUID.nRawECX0 = Cpuid.nECX;
		FCpu_IntelInfo.CPUID.nRawEDX0 = Cpuid.nEDX;

		FCpu_IntelInfo.CPUID.nMaxCPUIDInputValue = FCpu_IntelInfo.CPUID.nRawEAX0;

		if( FCpu_IntelInfo.CPUID.nRawEBX0==_CPUID_CHARCODE('G','e','n','u')
			&& FCpu_IntelInfo.CPUID.nRawEDX0==_CPUID_CHARCODE('i','n','e','I')
			&& FCpu_IntelInfo.CPUID.nRawECX0==_CPUID_CHARCODE('n','t','e','l') ) {
			FCpu_IntelInfo.CPUID.bGenuineIntel = TRUE;
		} else {
			FCpu_IntelInfo.CPUID.bGenuineIntel = FALSE;
		}

		if( FCpu_IntelInfo.CPUID.nMaxCPUIDInputValue < 1 ) {
			// CPUID(1) is not supported...
			FCpu_IntelInfo.nCpuCode = FCPU_INTEL_CPUCODE_80486;
		} else {
			_CPUID( 1, &Cpuid );
			FCpu_IntelInfo.CPUID.CPUID1.nRawEAX1 = Cpuid.nEAX;
			FCpu_IntelInfo.CPUID.CPUID1.nRawEBX1 = Cpuid.nEBX;
			FCpu_IntelInfo.CPUID.CPUID1.nRawECX1 = Cpuid.nECX;
			FCpu_IntelInfo.CPUID.CPUID1.nRawEDX1 = Cpuid.nEDX;

			FCpu_IntelInfo.CPUID.CPUID1.nProcessorType = (FCpu_IntelInfo.CPUID.CPUID1.nRawEAX1 >> 12) & 0x3;
			FCpu_IntelInfo.CPUID.CPUID1.nProcessorFamily = (FCpu_IntelInfo.CPUID.CPUID1.nRawEAX1 >> 8) & 0xf;
			FCpu_IntelInfo.CPUID.CPUID1.nProcessorModel = (FCpu_IntelInfo.CPUID.CPUID1.nRawEAX1 >> 4) & 0xf;
			FCpu_IntelInfo.CPUID.CPUID1.nSteppingId = (FCpu_IntelInfo.CPUID.CPUID1.nRawEAX1 >> 0) & 0xf;

			switch( FCpu_IntelInfo.CPUID.CPUID1.nProcessorFamily ) {
			case 0:
				FCpu_IntelInfo.nCpuCode = FCPU_INTEL_CPUCODE_8086;
				break;
			case 1:
				FCpu_IntelInfo.nCpuCode = FCPU_INTEL_CPUCODE_UNKNOWN;
				break;
			case 2:
				FCpu_IntelInfo.nCpuCode = FCPU_INTEL_CPUCODE_80286;
				break;
			case 3:
				FCpu_IntelInfo.nCpuCode = FCPU_INTEL_CPUCODE_80386;
				break;
			case 4:
				FCpu_IntelInfo.nCpuCode = FCPU_INTEL_CPUCODE_80486;
				break;
			case 5:
				FCpu_IntelInfo.nCpuCode = FCPU_INTEL_CPUCODE_P1;
				break;
			case 6:
				switch( FCpu_IntelInfo.CPUID.CPUID1.nProcessorModel ) {
				case 1:
					FCpu_IntelInfo.nCpuCode = FCPU_INTEL_CPUCODE_PPRO;
					break;
				case 3:
				case 5:
				case 6:
					FCpu_IntelInfo.nCpuCode = FCPU_INTEL_CPUCODE_P2;
					break;
				case 7:
					FCpu_IntelInfo.nCpuCode = FCPU_INTEL_CPUCODE_P3;
					break;
				default:
					// Unknown. Assume most recent CPU known...
					FCpu_IntelInfo.nCpuCode = FCPU_INTEL_CPUCODE_P3;
					break;
				}
				break;

			default:
				// Unknown. Assume most recent CPU known...
				FCpu_IntelInfo.nCpuCode = FCPU_INTEL_CPUCODE_P3;
				break;
			}

			#define _SET_SUPPORTED_FEATURE_BIT( szVarName, nBit ) \
					(FCpu_IntelInfo.CPUID.CPUID1.##szVarName## = (FCpu_IntelInfo.CPUID.CPUID1.nRawEDX1 & (1<<nBit) ? TRUE : FALSE) )

			_SET_SUPPORTED_FEATURE_BIT( bBuiltInFPU, 0 );
			_SET_SUPPORTED_FEATURE_BIT( bVirtual8086ModeEnhancement, 1 );
			_SET_SUPPORTED_FEATURE_BIT( bDebuggingExtensions, 2 );
			_SET_SUPPORTED_FEATURE_BIT( bPageSizeExtensions, 3 );
			_SET_SUPPORTED_FEATURE_BIT( bTimeStampCounter, 4 );
			_SET_SUPPORTED_FEATURE_BIT( bMSRInstructions, 5 );
			_SET_SUPPORTED_FEATURE_BIT( bPhysicalAddressExtensions, 6 );
			_SET_SUPPORTED_FEATURE_BIT( bMachineCheckException, 7 );
			_SET_SUPPORTED_FEATURE_BIT( bCMPXCHG8BInstruction, 8 );
			_SET_SUPPORTED_FEATURE_BIT( bBuiltInAPIC, 9 );
			_SET_SUPPORTED_FEATURE_BIT( bMTRRSupport, 12 );
			_SET_SUPPORTED_FEATURE_BIT( bPTEGlobalBit, 13 );
			_SET_SUPPORTED_FEATURE_BIT( bMachineCheckArchitecture, 14 );
			_SET_SUPPORTED_FEATURE_BIT( bConditionalMOVCMPInstructions, 15 );
			_SET_SUPPORTED_FEATURE_BIT( bMMXTechnology, 23 );
			_SET_SUPPORTED_FEATURE_BIT( bSIMDTechnology, 25 );

			if( FCpu_IntelInfo.CPUID.nMaxCPUIDInputValue >= 2 ) {
				// CPUID(2) is supported...

				_CPUID( 2, &Cpuid );
				nIterations = Cpuid.nEAX & 0xff;
				for( i=0; i<nIterations; i++ ) {
					if( i ) {
						_CPUID( 2, &Cpuid );
					}

					_RegisterCacheInfoReg( Cpuid.nEAX, i==0 );
					_RegisterCacheInfoReg( Cpuid.nEBX, 0 );
					_RegisterCacheInfoReg( Cpuid.nECX, 0 );
					_RegisterCacheInfoReg( Cpuid.nEDX, 0 );
				}
			}
		}
	}

	if( FCpu_IntelInfo.CPUID.CPUID1.bSIMDTechnology ) {
		FCpu_IntelInfo.bSIMDSupportedByOS = TRUE;

		__try {
			__asm {
				_emit 0x0f		// andps xmm0, xmm0
				_emit 0x54
				_emit 0xc0
			}
		}
		__except( EXCEPTION_EXECUTE_HANDLER ) {
			FCpu_IntelInfo.bSIMDSupportedByOS = FALSE;
		}
	} else {
		FCpu_IntelInfo.bSIMDSupportedByOS = FALSE;
	}
}

static void _RegisterCacheInfoReg( u32 nRegister, BOOL bSkipLowByte ) {
	if( !(nRegister & 0x80000000) ) {
		if( !bSkipLowByte ) {
			_RegisterCacheInfoByte( (u8)(nRegister & 0xff) );
		}
		_RegisterCacheInfoByte( (u8)((nRegister>>8) & 0xff) );
		_RegisterCacheInfoByte( (u8)((nRegister>>16) & 0xff) );
		_RegisterCacheInfoByte( (u8)((nRegister>>24) & 0xff) );
	}
}

static void _RegisterCacheInfoByte( u8 nByte ) {
	switch( nByte ) {
	case 0x6:	// Instruction cache: 8K Bytes, 4-way set associative, 32 byte line size:
		FCpu_Info.CacheI.nCacheBytes = 8 * 1024;
		FCpu_Info.CacheI.nLineBytes = 32;
		FCpu_Info.CacheI.nSetAssociative = 4;
		FCpu_Info.CacheI.bCacheDataValid = TRUE;
		break;

	case 0x8:	// Instruction cache: 16K Bytes, 4-way set associative, 32 byte line size:
		FCpu_Info.CacheI.nCacheBytes = 16 * 1024;
		FCpu_Info.CacheI.nLineBytes = 32;
		FCpu_Info.CacheI.nSetAssociative = 4;
		FCpu_Info.CacheI.bCacheDataValid = TRUE;
		break;

	case 0xA:	// Data cache: 8K Bytes, 2-way set associative, 32 byte line size:
		FCpu_Info.CacheD.nCacheBytes = 8 * 1024;
		FCpu_Info.CacheD.nLineBytes = 32;
		FCpu_Info.CacheD.nSetAssociative = 2;
		FCpu_Info.CacheD.bCacheDataValid = TRUE;
		break;

	case 0xC:	// Data cache: 16K Bytes, 2-way or 4-way set associative, 32 byte line size
		FCpu_Info.CacheD.nCacheBytes = 16 * 1024;
		FCpu_Info.CacheD.nLineBytes = 32;
		FCpu_Info.CacheD.nSetAssociative = 2;
		FCpu_Info.CacheD.bCacheDataValid = TRUE;
		break;

	case 0x41:	// L2 Unified cache: 128K Bytes, 4-way set associative, 32 byte line size:
		FCpu_Info.CacheL2.nCacheBytes = 128 * 1024;
		FCpu_Info.CacheL2.nLineBytes = 32;
		FCpu_Info.CacheL2.nSetAssociative = 4;
		FCpu_Info.CacheL2.bCacheDataValid = TRUE;
		break;

	case 0x42:	// L2 Unified cache: 256K Bytes, 4-way set associative, 32 byte line size:
		FCpu_Info.CacheL2.nCacheBytes = 256 * 1024;
		FCpu_Info.CacheL2.nLineBytes = 32;
		FCpu_Info.CacheL2.nSetAssociative = 4;
		FCpu_Info.CacheL2.bCacheDataValid = TRUE;
		break;

	case 0x43:	// L2 Unified cache: 512K Bytes, 4-way set associative, 32 byte line size:
		FCpu_Info.CacheL2.nCacheBytes = 512 * 1024;
		FCpu_Info.CacheL2.nLineBytes = 32;
		FCpu_Info.CacheL2.nSetAssociative = 4;
		FCpu_Info.CacheL2.bCacheDataValid = TRUE;
		break;

	case 0x44:	// L2 Unified cache: 1M Byte, 4-way set associative, 32 byte line size:
		FCpu_Info.CacheL2.nCacheBytes = 1024 * 1024;
		FCpu_Info.CacheL2.nLineBytes = 32;
		FCpu_Info.CacheL2.nSetAssociative = 4;
		FCpu_Info.CacheL2.bCacheDataValid = TRUE;
		break;

	case 0x45:	// L2 Unified cache: 2M Byte, 4-way set associative, 32 byte line size:
		FCpu_Info.CacheL2.nCacheBytes = 2048 * 1024;
		FCpu_Info.CacheL2.nLineBytes = 32;
		FCpu_Info.CacheL2.nSetAssociative = 4;
		FCpu_Info.CacheL2.bCacheDataValid = TRUE;
		break;

	default:
		break;
	}
}


static void _CPUID( u32 nInputEAX, _CpuidInfo_t *pCpuidInfo ) {
	u32 nEAX, nEBX, nECX, nEDX;

	__asm {
		push eax
		push ebx
		push ecx
		push edx

		mov eax, nInputEAX
		_emit 0x0f				// cpuid instruction...
		_emit 0xa2
		mov nEAX, eax
		mov nEBX, ebx
		mov nECX, ecx
		mov nEDX, edx

		pop edx
		pop ecx
		pop ebx
		pop eax
	}

	pCpuidInfo->nEAX = nEAX;
	pCpuidInfo->nEBX = nEBX;
	pCpuidInfo->nECX = nECX;
	pCpuidInfo->nEDX = nEDX;
}

// Returns TRUE if the CPUID instruction is supported.
static BOOL _IsCPUIDInstructionSupported( void ) {
	BOOL bSupported = TRUE;

	__asm {
		pushfd					// Save all modified registers...
		push eax
		push ecx

		pushfd					// Get original EFLAGS in eax and ecx...
		pop eax
		mov ecx, eax

		xor eax, 0x200000		// Try to flip ID bit in EFLAGS...
		push eax
		popfd

		pushfd					// Get new EFLAGS in eax...
		pop eax

		xor eax, ecx			// Did we actually toggle ID bit?
		jnz ExitCPUID			// Jump if yes

		mov bSupported, FALSE	// CPUID not supported

ExitCPUID:
		pop ecx					// Restore registers...
		pop eax
		popfd
	}

	return bSupported;
}

// Returns TRUE if the CPU is an 8086.
//
// IMPORTANT: Call this function ONLY if _IsCPUIDInstructionSupported() has failed.
static BOOL _IsCPUAn8086( void ) {
	BOOL bIsAn8086 = TRUE;

	__asm {
		pushf					// Push all modified registers...
		push ax
		push cx

		pushf					// Get original FLAGS in ax and cx...
		pop ax
		mov cx, ax

		and ax, 0xfff			// Try to clear bits 12-15 in FLAGS...
		push ax
		popf

		pushf					// Get new FLAGS in ax...
		pop ax

		and ax, 0xf000			// If bits 12-15 are set then the CPU is an 8086...
		cmp ax, 0xf000
		je Exit8086				// Jump if this is an 8086

		mov bIsAn8086, FALSE	// Not an 8086

Exit8086:
		pop cx					// Restore registers...
		pop ax
		popf
	}

	return bIsAn8086;
}

// Returns TRUE if the CPU is an 80286.
//
// IMPORTANT: Call this function ONLY if _IsCPUAn8086() has failed.
static BOOL _IsCPUAn80286( void ) {
	BOOL bIsAn80286 = TRUE;

	__asm {
		pushf					// Save all modified registers...
		push ax
		push bx
		push cx

		pushf					// Get original FLAGS in cx and bx...
		pop cx
		mov bx, cx

		or cx, 0f000h			// Try to set bits 12-15 in FLAGS...
		push cx
		popf

		pushf					// Get new FLAGS in ax...
		pop ax

		and ax, 0f000h			// If bits 12-15 are clear then the CPU is an 80286...
		jz Exit80286			// Jump if this is an 80286

		mov bIsAn80286, FALSE	// Not an 80286

Exit80286:
		pop cx					// Restore registers...
		pop bx
		pop ax
		popf
	}

	return bIsAn80286;
}

// Returns TRUE if the CPU is an 80386.
//
// IMPORTANT: Call this function ONLY if _IsCPUAn80286() has failed.
static BOOL _IsCPUAn80386( void ) {
	BOOL bIsAn80386 = TRUE;

	__asm {
		pushfd					// Save all modified registers...
		push eax
		push ebx
		push ecx

		mov bx, sp
		and sp, not 3

		pushfd					// Get original EFLAGS in eax and ecx...
		pop eax
		mov ecx, eax

		xor eax, 40000h			// Try to flip AC bit in EFLAGS...
		push eax
		popfd

		pushfd					// Get new EFLAGS in eax...
		pop eax

		xor eax, ecx			// If we can't toggle AC bit, the CPU is an 80386...
		jz Exit80386			// Jump if this is an 80386

		mov bIsAn80386, FALSE	// Not an 80386

Exit80386:
		push ecx				// Restore sp...
		popfd
		mov sp, bx

		pop ecx					// Restore registers...
		pop ebx
		pop eax
		popfd
	}

	return bIsAn80386;
}

// Measures the increment frequency of the RDTSC instruction.
// Returns the RDTSC frequency in Hz if successful.
// Returns 0.0f if the frequency could not be determined.
static float _MeasureFreqRDTSC( void ) {
	LARGE_INTEGER LargeInt0, LargeInt1;	// Variables for High-Resolution Performance Counter reads
	LARGE_INTEGER nCountFreq;			// High Resolution Performance Counter frequency

	u32 nFreq=0;						// Most current freq calculation
	u32 nFreq2=0;						// 2nd most current freq calculation
	u32 nFreq3=0;						// 3rd most current freq calculation
	u32 nTotal;							// Sum of previous three frequency calculations
	int nTries=0;						// Number of times a calculation has been made on this call

	u32 nTotalCycles=0, nCycles;		// Clock cycles elapsed during test
	u32 nStamp0 = 0, nStamp1 = 0;		// Time Stamp Variable for beginning and end of test
	u32 nTotalTicks=0, nTicks;			// Microseconds elapsed during test

	int nThreadPriority;				// Current thread priority

	HANDLE hThread = GetCurrentThread();

	if( !QueryPerformanceFrequency( &nCountFreq ) ) {
		// Trouble obtaining performance frequency...
		return 0.0f;
	}

	// On processors supporting the RDTSC instruction, compare the elapsed time on the hires counter with
	// the elapsed cycles on the RDTSC register.

	do {
		// This do loop runs up to 20 times or until the average of the previous three calculated frequencies is 
	   	// within 1 MHz of each of the individual calculated frequencies. This resampling increases the 
		// accuracy of the results since outside factors could affect this calculation.

		nTries++;			// Increment number of times sampled on this call to cpuspeed
			
		nFreq3 = nFreq2;	// Shift frequencies back to make room for the new frequency measurement
		nFreq2 = nFreq;

    	QueryPerformanceCounter( &LargeInt0 );		// Get hires performance counter timer

		LargeInt1.LowPart = LargeInt0.LowPart;		// Set initial time
		LargeInt1.HighPart = LargeInt0.HighPart;

		nThreadPriority = GetThreadPriority(hThread);
		if( nThreadPriority != THREAD_PRIORITY_ERROR_RETURN ) {
			SetThreadPriority( hThread, THREAD_PRIORITY_TIME_CRITICAL );
		}

   		while( ( (u32)LargeInt1.LowPart - (u32)LargeInt0.LowPart ) < 50 ) {
   			// Loop until 50 ticks have passed since last read of hires counter. This accounts for
			// overhead later.

			QueryPerformanceCounter( &LargeInt1 );

			__asm {
				push eax
				push edx
				rdtsc
				mov nStamp0, eax	// Place lower 32-bits of RDTSC return value into nStamp0
				pop edx
				pop eax
			}
		}

		LargeInt0.LowPart = LargeInt1.LowPart;		// Reset initial time...
		LargeInt0.HighPart = LargeInt1.HighPart;

   		while( ( (u32)LargeInt1.LowPart-(u32)LargeInt0.LowPart ) < 1000 ) {
   			// Loop until 1000 ticks have passed since last read of hires counter. This allows for
			// elapsed time for sampling.

   			QueryPerformanceCounter(&LargeInt1);

			__asm {
				push eax
				push edx
				rdtsc
				mov nStamp1, eax	// Place lower 32-bits of RDTSC return value into nStamp1
				pop edx
				pop eax
			}
		}

		// Restore priority...
		if( nThreadPriority != THREAD_PRIORITY_ERROR_RETURN ) {
			SetThreadPriority( hThread, nThreadPriority );
		}

		// Number of internal clock cycles is the difference between the two time stamp readings...
       	nCycles = nStamp1 - nStamp0;

		// Number of external ticks is the difference between the two hires counter reads...
    	nTicks = (u32) LargeInt1.LowPart - (u32) LargeInt0.LowPart;	

		// Note that some seemingly arbitrary mulitplies and divides are done below. This is to maintain a 
		// high level of precision without truncating the most significant data.

		// Convert ticks to one-hundred-thousandths of a tick...
		//    hundred_thousandths_of_a_tick / ( 10 ticks/second ) = us
		nTicks = nTicks * 100000;
		nTicks = nTicks / ( nCountFreq.LowPart/10 );

		nTotalTicks += nTicks;
		nTotalCycles += nCycles;

		// Round up if necessary...
		if( nTicks%nCountFreq.LowPart > nCountFreq.LowPart/2 ) {
			nTicks++;
		}

		// Cycles / us = MHz
		nFreq = nCycles / nTicks;

		// Round up if necessary...
		if( nCycles%nTicks > nTicks/2 ) {
       		nFreq++;
		}

		// Total the last three frequency calculations...
		nTotal = ( nFreq + nFreq2 + nFreq3 );
	} while( (nTries < 10 ) ||
			 (nTries < 20) &&
				(( abs((int)(3*nFreq  - nTotal)) > 3*_FREQ_TOLERANCE_MHZ ) ||
				 ( abs((int)(3*nFreq2 - nTotal)) > 3*_FREQ_TOLERANCE_MHZ ) ||
				 ( abs((int)(3*nFreq3 - nTotal)) > 3*_FREQ_TOLERANCE_MHZ )
				)
		   );

	return 1000000.0f * (float)nTotalCycles / (float)nTotalTicks;
}

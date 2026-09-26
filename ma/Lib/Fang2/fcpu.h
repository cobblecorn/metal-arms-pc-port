//////////////////////////////////////////////////////////////////////////////////////
// fcpu.h - Fang CPU module.
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

#ifndef _FCPU_H_
#define _FCPU_H_ 1

#include "fang.h"


#if FANG_PLATFORM_DX
	typedef enum {
		FCPU_INTEL_CPUTYPE_ORIGOEM = 0,
		FCPU_INTEL_CPUTYPE_OVERDRIVE = 1,
		FCPU_INTEL_CPUTYPE_DUALCPU = 2,
		FCPU_INTEL_CPUTYPE_RESERVED = 3
	} FCpu_Intel_CpuType_e;

	typedef enum {
		FCPU_INTEL_CPUFAMILY_486 = 4,		// 486
		FCPU_INTEL_CPUFAMILY_P1 = 5,		// Pentium
		FCPU_INTEL_CPUFAMILY_P2 = 6			// Includes Pentium Pro, Pentium 2, and Pentium 3
	} FCpu_Intel_CpuFamily_e;

	typedef enum {
		FCPU_INTEL_CPUCODE_UNKNOWN,
		FCPU_INTEL_CPUCODE_8086,
		FCPU_INTEL_CPUCODE_80286,
		FCPU_INTEL_CPUCODE_80386,
		FCPU_INTEL_CPUCODE_80486,
		FCPU_INTEL_CPUCODE_P1,
		FCPU_INTEL_CPUCODE_PPRO,
		FCPU_INTEL_CPUCODE_P2,				// Includes Celeron
		FCPU_INTEL_CPUCODE_P3,

		FCPU_INTEL_CPUCODE_COUNT
	} FCpu_Intel_CpuCode_e;

	typedef struct {
		u32 nRawEAX1;
		u32 nRawEBX1;
		u32 nRawECX1;
		u32 nRawEDX1;

		u32 nProcessorType;					// Shifted to bit 0 and masked to field length
		u32 nProcessorFamily;				// Shifted to bit 0 and masked to field length
		u32 nProcessorModel;				// Shifted to bit 0 and masked to field length
		u32 nSteppingId;					// Shifted to bit 0 and masked to field length

		BOOL bBuiltInFPU;					// TRUE if the CPU has a built-in FPU
		BOOL bVirtual8086ModeEnhancement;	// TRUE if the CPU supports virtual 8086-mode enhancement
		BOOL bDebuggingExtensions;			// TRUE if the CPU supports debugging extensions
		BOOL bPageSizeExtensions;			// TRUE if the CPU supports page size extensions
		BOOL bTimeStampCounter;				// TRUE if the CPU supports a time stamp counter
		BOOL bMSRInstructions;				// TRUE if the CPU supports RDMSR and WRMSR instructions
		BOOL bPhysicalAddressExtensions;	// TRUE if the CPU supports physical address extensions
		BOOL bMachineCheckException;		// TRUE if the CPU supports the machine check exception
		BOOL bCMPXCHG8BInstruction;			// TRUE if the CPU supports the CMPXCHG8B instruction
		BOOL bBuiltInAPIC;					// TRUE if the CPU has a built-in APIC
		BOOL bMTRRSupport;					// TRUE if the CPU supports MTRRs
		BOOL bPTEGlobalBit;					// TRUE if the CPU supports the PTE global bit
		BOOL bMachineCheckArchitecture;		// TRUE if the CPU supports the machine check architecture
		BOOL bConditionalMOVCMPInstructions;// TRUE if the CPU supports conditional MOV and CMP instructions
		BOOL bMMXTechnology;				// TRUE if the CPU supports MMX technology
		BOOL bSIMDTechnology;				// TRUE if the CPU supports SIMD technology
	} FCpu_Intel_CPUID1_t;

	typedef struct {
		u32 nRawEAX0;
		u32 nRawEBX0;
		u32 nRawECX0;
		u32 nRawEDX0;

		u32 nMaxCPUIDInputValue;			// Maximum value allowed in eax on input to the CPUID instruction
		BOOL bGenuineIntel;					// "GenuineIntel" returned from CPUID(0)

		FCpu_Intel_CPUID1_t CPUID1;			// Valid only if nMaxCPUIDInputValue > 0
	} FCpu_Intel_CPUID_t;

	typedef struct {
		FCpu_Intel_CpuCode_e nCpuCode;		// See FCpu_Intel_CpuCode_e for possible values

		BOOL bSupports_CPUID;				// TRUE if the CPUID instruction is supported
		FCpu_Intel_CPUID_t CPUID;			// (valid only if bSupports_CPUID is TRUE, otherwise all fields set to 0)

		BOOL bSIMDSupportedByOS;			// TRUE if the OS supports SIMD technology
		float fCyclesPerSecond_RDTSC;		// Frequency of the RDTSC instruction (valid only when bTimeStampCounter is TRUE)
	} FCpu_Intel_Info_t;
#endif


typedef struct {
	BOOL bCacheDataValid;					// TRUE: This structure contains valid data
	u32 nCacheBytes;						// Cache bytes (0=no cache)
	u32 nLineBytes;							// Cache line bytes (0=no cache)
	u32 nSetAssociative;					// Usually 2 or 4 for 2-way or 4-way set associative caches, respectively
} FCpu_CacheInfo_t;

typedef struct {
	FCpu_CacheInfo_t CacheI;				// L1 instruction cache info
	FCpu_CacheInfo_t CacheD;				// L1 data cache info
	FCpu_CacheInfo_t CacheL2;				// L2 cache info
} FCpu_Info_t;



extern BOOL fcpu_ModuleStartup( void );
extern void fcpu_ModuleShutdown( void );


#if FANG_PLATFORM_DX
	extern FCpu_Intel_Info_t FCpu_IntelInfo;

	extern const char *drvcpu_intel_GetCpuCodeString( void );
#endif


extern FCpu_Info_t FCpu_Info;




#endif


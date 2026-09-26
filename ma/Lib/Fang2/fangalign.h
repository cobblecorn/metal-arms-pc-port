//////////////////////////////////////////////////////////////////////////////////////
// fangalign.h - Extension of fang.h for memory alignment definitions.
//
// Author: Steve Ranck     
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
// 04/06/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FANGALIGN_H_
#define _FANGALIGN_H_ 1



extern BOOL _FRes_bTestForDestructor;
extern BOOL _FRes_bHasDestructor;


#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
	#define FRES_DEBUG_STACK_SIZE	10

	typedef struct {
		cchar *pszFilename;
		u32 nLineNum;
		void *pMemBlock;
	} FResDebugStack_t;


	extern FResDebugStack_t FRes_aDebugStack[FRES_DEBUG_STACK_SIZE];
	extern u32 FRes_nDebugStackCount;

	extern void *fres_Debug_AlignedAlloc( cchar *pszFileName, u32 nLineNum, u32 nBytes, u32 nByteAlignment );
	extern void *fres_Debug_AlignedAllocWithOffset( cchar *pszFileName, u32 nLineNum, u32 nBytes, u32 nByteAlignment, u32 nOffset );

	extern void *fres_DebugNewWithDestructor( u32 nBytesToAlloc, u32 nByteAlignment, cchar *pszClassName, u32 nClassByteCount, cchar *pszFileName, u32 nLineNum );
	extern void *fres_DebugNewArrayWithDestructor( u32 nBytesToAlloc, u32 nByteAlignment, u32 nByteOffset, cchar *pszClassName, u32 nClassByteCount, cchar *pszFileName, u32 nLineNum );
	extern void fres_DebugDelete( void *pBase, u32 nByteAlignment, cchar *pszClassName, u32 nClassByteCount );
#endif


#define FCLASS_NOALIGN_PREFIX
#define FCLASS_NOALIGN_SUFFIX


#if FANG_PLATFORM_DX
	#if !FANG_WINGC
		// Windows or Xbox builds:
		#define FCLASS_BYTE_ALIGN	16
	#else
		// WinGC build (use GC alignment):
		#define FCLASS_BYTE_ALIGN	8
	#endif

	#define FCLASS_ALIGN_PREFIX		__declspec( align(FCLASS_BYTE_ALIGN) )
	#define FCLASS_ALIGN_SUFFIX
#elif FANG_PLATFORM_GC
	#define FCLASS_BYTE_ALIGN		8
	#define FCLASS_ALIGN_PREFIX
	#define FCLASS_ALIGN_SUFFIX		__attribute__ ((aligned (FCLASS_BYTE_ALIGN)))
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	#define FCLASS_BYTE_ALIGN		16
	#define FCLASS_ALIGN_PREFIX
	#define FCLASS_ALIGN_SUFFIX		__attribute__ ((aligned (FCLASS_BYTE_ALIGN)))
//ARG - <<<<<
#else
	#error <Need FCLASS_* definitions for platform.>
#endif


//--------------------------------------------------------------------------------------------------------------------------
// Windows Implementation:
//--------------------------------------------------------------------------------------------------------------------------
#if FANG_PLATFORM_WIN & 0

	// Fang uses stack-based memory allocation for the "new" and "delete" operators.
	// In Windows, when "new []" is used on a class with a destructor, the compiler
	// adds 4 to the address returned by "new []" in order to save the number of elements
	// to later destroy via "delete []". Note that "delete" will not perform any memory
	// management.

	#if FANG_DEBUG_BUILD || FANG_TEST_BUILD

		#define FCLASS_STACKMEM_ALIGN( szClassName )														\
				public:																						\
																											\
				enum { CLASS_BYTE_ALIGNMENT = FCLASS_BYTE_ALIGN };											\
																											\
				void *operator new( unsigned int nBytesToAlloc ) {											\
					FASSERT_NOW_MSG( "Fang: Use 'fnew' instead of 'new'." );								\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( NULL, 0, nBytesToAlloc, CLASS_BYTE_ALIGNMENT );		\
					}																						\
					return fres_DebugNewWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName), NULL, 0 ); \
				}																							\
																											\
				void *operator new( unsigned int nBytesToAlloc, cchar *pszFileName, int nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( pszFileName, nLineNum, nBytesToAlloc, CLASS_BYTE_ALIGNMENT ); \
					}																						\
					return fres_DebugNewWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName), pszFileName, nLineNum ); \
				}																							\
																											\
				void *operator new []( unsigned int nBytesToAlloc ) {										\
					FASSERT_NOW_MSG( "Fang: Use 'fnew' instead of 'new'." );								\
					FASSERT( !_FRes_bTestForDestructor );													\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( NULL, 0, nBytesToAlloc, CLASS_BYTE_ALIGNMENT );		\
					}																						\
					return fres_DebugNewArrayWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, sizeof(size_t), #szClassName, sizeof(szClassName), NULL, 0 ); \
				}																							\
																											\
				void *operator new []( unsigned int nBytesToAlloc, cchar *pszFileName, int nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					if( !_FRes_bTestForDestructor ) {														\
						_FRes_bTestForDestructor = TRUE;													\
						new( NULL, 0, 0, 0, 0 ) szClassName[2];												\
						if( !_FRes_bHasDestructor ) {														\
							return fres_Debug_AlignedAlloc( pszFileName, nLineNum, nBytesToAlloc, CLASS_BYTE_ALIGNMENT ); \
						}																					\
						return fres_DebugNewArrayWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, sizeof(size_t), #szClassName, sizeof(szClassName), pszFileName, nLineNum ); \
					} else {																				\
						_FRes_bTestForDestructor = FALSE;													\
						_FRes_bHasDestructor = ( nBytesToAlloc != 2*sizeof(szClassName) );					\
						return (void *)NULL;																\
					}																						\
				}																							\
																											\
				void operator delete( void *pMemoryBlock ) {												\
					FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == pMemoryBlock), "Fang: Use 'fdelete()' instead of 'delete'." ); \
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
				}																							\
																											\
				void operator delete( void *pMemoryBlock, cchar *pszFileName, int nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);								\
					FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = pszFileName;						\
					FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = nLineNum;							\
					FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = pMemoryBlock;						\
					FRes_nDebugStackCount++;																\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
					FRes_nDebugStackCount--;																\
				}																							\
																											\
				void operator delete []( void *pMemoryBlock ) {												\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == pMemoryBlock), "Fang: Use 'fdelete_array()' instead of 'delete []'." ); \
					} else {																				\
						FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == (void *)((u32)pMemoryBlock+sizeof(u32))), "Fang: Use 'fdelete_array()' instead of 'delete []'." ); \
					}																						\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
				}																							\
																											\
				void operator delete []( void *pMemoryBlock, cchar *pszFileName, int nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);								\
					FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = pszFileName;						\
					FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = nLineNum;							\
					FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = pMemoryBlock;						\
					FRes_nDebugStackCount++;																\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
					FRes_nDebugStackCount--;																\
				}


		#define FCLASS_STACKMEM_NOALIGN( szClassName )														\
				public:																						\
																											\
				enum { CLASS_BYTE_ALIGNMENT = 4 };															\
																											\
				void *operator new( unsigned int nBytesToAlloc ) {											\
					FASSERT_NOW_MSG( "Fang: Use 'fnew' instead of 'new'." );								\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( NULL, 0, nBytesToAlloc, CLASS_BYTE_ALIGNMENT );		\
					}																						\
					return fres_DebugNewWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName), NULL, 0 ); \
				}																							\
																											\
				void *operator new( unsigned int nBytesToAlloc, cchar *pszFileName, int nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( pszFileName, nLineNum, nBytesToAlloc, CLASS_BYTE_ALIGNMENT ); \
					}																						\
					return fres_DebugNewWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName), pszFileName, nLineNum ); \
				}																							\
																											\
				void *operator new []( unsigned int nBytesToAlloc ) {										\
					FASSERT_NOW_MSG( "Fang: Use 'fnew' instead of 'new'." );								\
					FASSERT( !_FRes_bTestForDestructor );													\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( NULL, 0, nBytesToAlloc, CLASS_BYTE_ALIGNMENT );		\
					}																						\
					return fres_DebugNewArrayWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, 0, #szClassName, sizeof(szClassName), NULL, 0 ); \
				}																							\
																											\
				void *operator new []( unsigned int nBytesToAlloc, cchar *pszFileName, int nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					if( !_FRes_bTestForDestructor ) {														\
						_FRes_bTestForDestructor = TRUE;													\
						new( NULL, 0, 0, 0, 0 ) szClassName[2];												\
						if( !_FRes_bHasDestructor ) {														\
							return fres_Debug_AlignedAlloc( pszFileName, nLineNum, nBytesToAlloc, CLASS_BYTE_ALIGNMENT ); \
						}																					\
						return fres_DebugNewArrayWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, 0, #szClassName, sizeof(szClassName), pszFileName, nLineNum ); \
					} else {																				\
						_FRes_bTestForDestructor = FALSE;													\
						_FRes_bHasDestructor = ( nBytesToAlloc != 2*sizeof(szClassName) );					\
						return (void *)NULL;																\
					}																						\
				}																							\
																											\
				void operator delete( void *pMemoryBlock ) {												\
					FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == pMemoryBlock), "Fang: Use 'fdelete()' instead of 'delete'." ); \
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
				}																							\
																											\
				void operator delete( void *pMemoryBlock, cchar *pszFileName, int nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);								\
					FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = pszFileName;						\
					FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = nLineNum;							\
					FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = pMemoryBlock;						\
					FRes_nDebugStackCount++;																\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
					FRes_nDebugStackCount--;																\
				}																							\
																											\
				void operator delete []( void *pMemoryBlock ) {												\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == pMemoryBlock), "Fang: Use 'fdelete_array()' instead of 'delete []'." ); \
					} else {																				\
						FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == (void *)((u32)pMemoryBlock+sizeof(u32))), "Fang: Use 'fdelete_array()' instead of 'delete []'." ); \
					}																						\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
				}																							\
																											\
				void operator delete []( void *pMemoryBlock, cchar *pszFileName, int nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);								\
					FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = pszFileName;						\
					FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = nLineNum;							\
					FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = pMemoryBlock;						\
					FRes_nDebugStackCount++;																\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
					FRes_nDebugStackCount--;																\
				}


		#define fnew new( __FILE__, __LINE__, 0, 0, 0 )

		#define fdelete( pMemoryBlock ) {																	\
					if( (pMemoryBlock) ) {																	\
						FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);							\
						FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = __FILE__;						\
						FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = __LINE__;						\
						FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = (pMemoryBlock);					\
						FRes_nDebugStackCount++;															\
						delete (pMemoryBlock);																\
						FASSERT( FRes_nDebugStackCount );													\
						FRes_nDebugStackCount--;															\
					}																						\
				}

		#define fdelete_array( pMemoryBlock ) {																\
					if( (pMemoryBlock) ) {																	\
						FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);							\
						FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = __FILE__;						\
						FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = __LINE__;						\
						FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = (pMemoryBlock);					\
						FRes_nDebugStackCount++;															\
						delete [] (pMemoryBlock);															\
						FASSERT( FRes_nDebugStackCount );													\
						FRes_nDebugStackCount--;															\
					}																						\
				}

	#else

		extern void *_fres_AlignedAlloc( u32 nBytes, u32 nByteAlignment );
		extern void *_fres_AlignedAllocWithOffset( u32 nBytes, u32 nByteAlignment, u32 nOffset );

		#define FCLASS_STACKMEM_ALIGN( szClassName )														\
				public:																						\
																											\
				enum { CLASS_BYTE_ALIGNMENT = FCLASS_BYTE_ALIGN };											\
																											\
				void *operator new( unsigned int nBytesToAlloc ) {											\
					return _fres_AlignedAlloc( nBytesToAlloc, CLASS_BYTE_ALIGNMENT );						\
				}																							\
																											\
				void *operator new []( unsigned int nBytesToAlloc ) {										\
					if( !_FRes_bTestForDestructor ) {														\
						_FRes_bTestForDestructor = TRUE;													\
						new szClassName[2];																	\
						if( !_FRes_bHasDestructor ) {														\
							return _fres_AlignedAlloc( nBytesToAlloc, CLASS_BYTE_ALIGNMENT );				\
						}																					\
						return _fres_AlignedAllocWithOffset( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, sizeof(size_t) );	\
					} else {																				\
						_FRes_bTestForDestructor = FALSE;													\
						_FRes_bHasDestructor = ( nBytesToAlloc != 2*sizeof(szClassName) );					\
						return (void *)NULL;																\
					}																						\
				}																							\
																											\
				FINLINE void operator delete( void *pMemoryBlock ) {										\
				}																							\
																											\
				FINLINE void operator delete []( void *pMemoryBlock ) {										\
				}


		#define FCLASS_STACKMEM_NOALIGN( szClassName )														\
				public:																						\
																											\
				enum { CLASS_BYTE_ALIGNMENT = 4 };															\
																											\
				void *operator new( unsigned int nBytesToAlloc ) {											\
					return _fres_AlignedAlloc( nBytesToAlloc, CLASS_BYTE_ALIGNMENT );						\
				}																							\
																											\
				void *operator new []( unsigned int nBytesToAlloc ) {										\
					return _fres_AlignedAlloc( nBytesToAlloc, CLASS_BYTE_ALIGNMENT );						\
				}																							\
																											\
				FINLINE void operator delete( void *pMemoryBlock ) {										\
				}																							\
																											\
				FINLINE void operator delete []( void *pMemoryBlock ) {										\
				}


		#define fnew new
		#define fdelete( pMemoryBlock ) delete pMemoryBlock
		#define fdelete_array( pMemoryBlock ) delete [] pMemoryBlock

	#endif



//--------------------------------------------------------------------------------------------------------------------------
// Xbox Implementation:
//--------------------------------------------------------------------------------------------------------------------------
#elif FANG_PLATFORM_XB || FANG_PLATFORM_WIN

	// Fang uses stack-based memory allocation for the "new" and "delete" operators.
	// The Xbox compiler doesn't have the same "new []" alignment issue as the
	// Windows compiler.

	#if FANG_DEBUG_BUILD || FANG_TEST_BUILD

		#define FCLASS_STACKMEM_ALIGN( szClassName )														\
				public:																						\
																											\
				enum { CLASS_BYTE_ALIGNMENT = FCLASS_BYTE_ALIGN };											\
																											\
				void *operator new( unsigned int nBytesToAlloc ) {											\
					FASSERT_NOW_MSG( "Fang: Use 'fnew' instead of 'new'." );								\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( NULL, 0, nBytesToAlloc, CLASS_BYTE_ALIGNMENT );		\
					}																						\
					return fres_DebugNewWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, #szClassName, 0, NULL, 0 ); \
				}																							\
																											\
				void *operator new( unsigned int nBytesToAlloc, cchar *pszFileName, int nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( pszFileName, nLineNum, nBytesToAlloc, CLASS_BYTE_ALIGNMENT ); \
					}																						\
					return fres_DebugNewWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, #szClassName, 0, pszFileName, nLineNum ); \
				}																							\
																											\
				void *operator new []( unsigned int nBytesToAlloc ) {										\
					FASSERT_NOW_MSG( "Fang: Use 'fnew' instead of 'new'." );								\
					FASSERT( !_FRes_bTestForDestructor );													\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( NULL, 0, nBytesToAlloc, CLASS_BYTE_ALIGNMENT );		\
					}																						\
					return fres_DebugNewArrayWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, 0, #szClassName, sizeof(szClassName), NULL, 0 ); \
				}																							\
																											\
				void *operator new []( unsigned int nBytesToAlloc, cchar *pszFileName, int nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					if( !_FRes_bTestForDestructor ) {														\
						_FRes_bTestForDestructor = TRUE;													\
						new( NULL, 0, 0, 0, 0 ) szClassName[2];												\
						if( !_FRes_bHasDestructor ) {														\
							return fres_Debug_AlignedAlloc( pszFileName, nLineNum, nBytesToAlloc, CLASS_BYTE_ALIGNMENT ); \
						}																					\
						return fres_DebugNewArrayWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, 0, #szClassName, sizeof(szClassName), pszFileName, nLineNum ); \
					} else {																				\
						_FRes_bTestForDestructor = FALSE;													\
						_FRes_bHasDestructor = ( nBytesToAlloc != 2*sizeof(szClassName) );					\
						return (void *)NULL;																\
					}																						\
				}																							\
																											\
				void operator delete( void *pMemoryBlock ) {												\
					FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == pMemoryBlock), "Fang: Use 'fdelete()' instead of 'delete'." ); \
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
				}																							\
																											\
				void operator delete( void *pMemoryBlock, cchar *pszFileName, int nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);								\
					FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = pszFileName;						\
					FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = pMemoryBlock;						\
					FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = nLineNum;							\
					FRes_nDebugStackCount++;																\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
					FRes_nDebugStackCount--;																\
				}																							\
																											\
				void operator delete []( void *pMemoryBlock ) {												\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == pMemoryBlock), "Fang: Use 'fdelete_array()' instead of 'delete []'." ); \
					} else {																				\
						FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == (void *)((u32)pMemoryBlock+FCLASS_BYTE_ALIGN)), "Fang: Use 'fdelete_array()' instead of 'delete []'." ); \
					}																						\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
				}																							\
																											\
				void operator delete []( void *pMemoryBlock, cchar *pszFileName, int nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);								\
					FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = pszFileName;						\
					FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = nLineNum;							\
					FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = pMemoryBlock;						\
					FRes_nDebugStackCount++;																\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
					FRes_nDebugStackCount--;																\
				}


		#define FCLASS_STACKMEM_NOALIGN( szClassName )														\
				public:																						\
																											\
				enum { CLASS_BYTE_ALIGNMENT = 4 };															\
																											\
				void *operator new( unsigned int nBytesToAlloc ) {											\
					FASSERT_NOW_MSG( "Fang: Use 'fnew' instead of 'new'." );								\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( NULL, 0, nBytesToAlloc, CLASS_BYTE_ALIGNMENT );		\
					}																						\
					return fres_DebugNewWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, #szClassName, 0, NULL, 0 ); \
				}																							\
																											\
				void *operator new( unsigned int nBytesToAlloc, cchar *pszFileName, int nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];															\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( pszFileName, nLineNum, nBytesToAlloc, CLASS_BYTE_ALIGNMENT ); \
					}																						\
					return fres_DebugNewWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, #szClassName, 0, pszFileName, nLineNum ); \
				}																							\
																											\
				void *operator new []( unsigned int nBytesToAlloc ) {										\
					FASSERT_NOW_MSG( "Fang: Use 'fnew' instead of 'new'." );								\
					FASSERT( !_FRes_bTestForDestructor );													\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( NULL, 0, nBytesToAlloc, CLASS_BYTE_ALIGNMENT );		\
					}																						\
					return fres_DebugNewArrayWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, 0, #szClassName, sizeof(szClassName), NULL, 0 ); \
				}																							\
																											\
				void *operator new []( unsigned int nBytesToAlloc, cchar *pszFileName, int nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					if( !_FRes_bTestForDestructor ) {														\
						_FRes_bTestForDestructor = TRUE;													\
						new( NULL, 0, 0, 0, 0 ) szClassName[2];												\
						if( !_FRes_bHasDestructor ) {														\
							return fres_Debug_AlignedAlloc( pszFileName, nLineNum, nBytesToAlloc, CLASS_BYTE_ALIGNMENT ); \
						}																					\
						return fres_DebugNewArrayWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, 0, #szClassName, sizeof(szClassName), pszFileName, nLineNum ); \
					} else {																				\
						_FRes_bTestForDestructor = FALSE;													\
						_FRes_bHasDestructor = ( nBytesToAlloc != 2*sizeof(szClassName) );					\
						return (void *)NULL;																\
					}																						\
				}																							\
																											\
				void operator delete( void *pMemoryBlock ) {												\
					FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == pMemoryBlock), "Fang: Use 'fdelete()' instead of 'delete'." ); \
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
				}																							\
																											\
				void operator delete( void *pMemoryBlock, cchar *pszFileName, int nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);								\
					FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = pszFileName;						\
					FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = nLineNum;							\
					FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = pMemoryBlock;						\
					FRes_nDebugStackCount++;																\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
					FRes_nDebugStackCount--;																\
				}																							\
																											\
				void operator delete []( void *pMemoryBlock ) {												\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == pMemoryBlock), "Fang: Use 'fdelete_array()' instead of 'delete []'." ); \
					} else {																				\
						FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == (void *)((u32)pMemoryBlock+4)), "Fang: Use 'fdelete_array()' instead of 'delete []'." ); \
					}																						\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
				}																							\
																											\
				void operator delete []( void *pMemoryBlock, cchar *pszFileName, int nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);								\
					FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = pszFileName;						\
					FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = nLineNum;							\
					FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = pMemoryBlock;						\
					FRes_nDebugStackCount++;																\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
					FRes_nDebugStackCount--;																\
				}


		#define fnew new( __FILE__, __LINE__, 0, 0, 0 )

		#define fdelete( pMemoryBlock ) {																	\
					if( (pMemoryBlock) ) {																	\
						FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);							\
						FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = __FILE__;						\
						FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = __LINE__;						\
						FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = (pMemoryBlock);					\
						FRes_nDebugStackCount++;															\
						delete (pMemoryBlock);																\
						FASSERT( FRes_nDebugStackCount );													\
						FRes_nDebugStackCount--;															\
					}																						\
				}

		#define fdelete_array( pMemoryBlock ) {																\
					if( (pMemoryBlock) ) {																	\
						FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);							\
						FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = __FILE__;						\
						FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = __LINE__;						\
						FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = (pMemoryBlock);					\
						FRes_nDebugStackCount++;															\
						delete [] (pMemoryBlock);															\
						FASSERT( FRes_nDebugStackCount );													\
						FRes_nDebugStackCount--;															\
					}																						\
				}

	#else

		extern void *_fres_AlignedAlloc( u32 nBytes, u32 nByteAlignment );
		extern void *_fres_AlignedAllocWithOffset( u32 nBytes, u32 nByteAlignment, u32 nOffset );

		#define FCLASS_STACKMEM_ALIGN( szClassName )														\
				public:																						\
																											\
				enum { CLASS_BYTE_ALIGNMENT = FCLASS_BYTE_ALIGN };											\
																											\
				void *operator new( unsigned int nBytesToAlloc ) {											\
					return _fres_AlignedAlloc( nBytesToAlloc, CLASS_BYTE_ALIGNMENT );						\
				}																							\
																											\
				void *operator new []( unsigned int nBytesToAlloc ) {										\
					return _fres_AlignedAlloc( nBytesToAlloc, CLASS_BYTE_ALIGNMENT );						\
				}																							\
																											\
				void operator delete( void *pMemoryBlock ) {												\
				}																							\
																											\
				void operator delete []( void *pMemoryBlock ) {												\
				}


		#define FCLASS_STACKMEM_NOALIGN( szClassName )														\
				public:																						\
																											\
				enum { CLASS_BYTE_ALIGNMENT = 4 };															\
																											\
				void *operator new( unsigned int nBytesToAlloc ) {											\
					return _fres_AlignedAlloc( nBytesToAlloc, CLASS_BYTE_ALIGNMENT );						\
				}																							\
																											\
				void *operator new []( unsigned int nBytesToAlloc ) {										\
					return _fres_AlignedAlloc( nBytesToAlloc, CLASS_BYTE_ALIGNMENT );						\
				}																							\
																											\
				void operator delete( void *pMemoryBlock ) {												\
				}																							\
																											\
				void operator delete []( void *pMemoryBlock ) {												\
				}


		#define fnew new
		#define fdelete( pMemoryBlock ) delete pMemoryBlock
		#define fdelete_array( pMemoryBlock ) delete [] pMemoryBlock

	#endif



//--------------------------------------------------------------------------------------------------------------------------
// GameCube Implementation:
//--------------------------------------------------------------------------------------------------------------------------
#elif FANG_PLATFORM_GC

	// The GameCube does not support new and delete.  Also, because
	// the heap needs to be set up prior to any memory allocations,
	// we run into an issue in C++ code:  Object creation at run 
	// time (objects instantiated outside of functions)
	// occurs prior to entry into main(), so the heap needs to be
	// setup the first time a "new" or "delete" is called.
	
	#include <os.h>
	
	extern void fgc_InitializeMemory( void );
	extern BOOL FGC_bMemoryInitialized;
	extern BOOL Fang_bInitialized;
	
	//
	FINLINE void* operator new( u32 uBlocksize )
	{
		// If memory is not already initialized, initialize it
		if ( !FGC_bMemoryInitialized )
		{
			fgc_InitializeMemory();
		}

		// If Fang is initialized, use its memory allocation functions			
		if ( Fang_bInitialized )
		{
			return fang_Malloc( uBlocksize, 2 );
		}

		// If Fang is not initialized, use the GC allocation functions directly			
		return OSAlloc( uBlocksize );
	}
	
	//
	FINLINE void* operator new[]( u32 uBlocksize )
	{
		// If memory is not already initialized, initialize it
		if ( !FGC_bMemoryInitialized )
		{
			fgc_InitializeMemory();
		}
			
		// If Fang is initialized, use its memory allocation functions			
		if ( Fang_bInitialized )
		{
			return fang_Malloc( uBlocksize, 2 );
		}

		// If Fang is not initialized, use the GC allocation functions directly			
		return OSAlloc( uBlocksize );
	}
	
	//
	FINLINE void operator delete( void *pMemoryBlock )
	{
		if ( Fang_bInitialized )
		{
			fang_Free( pMemoryBlock );
			return;
		}
		
		OSFree( pMemoryBlock );
	}
	
	//
	FINLINE void operator delete[]( void *pMemoryBlock )
	{
		if ( Fang_bInitialized )
		{
			fang_Free( pMemoryBlock );
			return;
		}

		OSFree( pMemoryBlock );
	}

	// Fang uses stack-based memory allocation for the "new" and "delete" operators.
	// The GameCube compiler doesn't have the same "new []" alignment issue as the
	// Windows compiler.

	#if FANG_DEBUG_BUILD || FANG_TEST_BUILD

		#define FCLASS_STACKMEM_ALIGN( szClassName )														\
				public:																						\
																											\
				enum { CLASS_BYTE_ALIGNMENT = FCLASS_BYTE_ALIGN };											\
																											\
				void *operator new( u32 nBytesToAlloc ) {											\
					FASSERT_NOW_MSG( "Fang: Use 'fnew' instead of 'new'." );								\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( NULL, 0, nBytesToAlloc, CLASS_BYTE_ALIGNMENT );		\
					}																						\
					return fres_DebugNewWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, #szClassName, 0, NULL, 0 ); \
				}																							\
																											\
				void *operator new( u32 nBytesToAlloc, cchar *pszFileName, s32 nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( pszFileName, nLineNum, nBytesToAlloc, CLASS_BYTE_ALIGNMENT ); \
					}																						\
					return fres_DebugNewWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, #szClassName, 0, pszFileName, nLineNum ); \
				}																							\
																											\
				void *operator new []( u32 nBytesToAlloc ) {										\
					FASSERT_NOW_MSG( "Fang: Use 'fnew' instead of 'new'." );								\
					FASSERT( !_FRes_bTestForDestructor );													\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( NULL, 0, nBytesToAlloc, CLASS_BYTE_ALIGNMENT );		\
					}																						\
					return fres_DebugNewArrayWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, 0, #szClassName, sizeof(szClassName), NULL, 0 ); \
				}																							\
																											\
				void *operator new []( u32 nBytesToAlloc, cchar *pszFileName, s32 nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					if( !_FRes_bTestForDestructor ) {														\
						_FRes_bTestForDestructor = TRUE;													\
						new( NULL, 0, 0, 0, 0 ) szClassName[2];												\
						if( !_FRes_bHasDestructor ) {														\
							return fres_Debug_AlignedAlloc( pszFileName, nLineNum, nBytesToAlloc, CLASS_BYTE_ALIGNMENT ); \
						}																					\
						return fres_DebugNewArrayWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, 0, #szClassName, sizeof(szClassName), pszFileName, nLineNum ); \
					} else {																				\
						_FRes_bTestForDestructor = FALSE;													\
						_FRes_bHasDestructor = ( nBytesToAlloc != 2*sizeof(szClassName) );					\
						return (void *)NULL;																\
					}																						\
				}																							\
																											\
				void operator delete( void *pMemoryBlock ) {												\
					FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == pMemoryBlock), "Fang: Use 'fdelete()' instead of 'delete'." ); \
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
				}																							\
																											\
				void operator delete( void *pMemoryBlock, cchar *pszFileName, s32 nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);								\
					FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = pszFileName;						\
					FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = pMemoryBlock;						\
					FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = nLineNum;							\
					FRes_nDebugStackCount++;																\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
					FRes_nDebugStackCount--;																\
				}																							\
																											\
				void operator delete []( void *pMemoryBlock ) {												\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == pMemoryBlock), "FangAlign (No Destructor): Use 'fdelete_array()' instead of 'delete []'." ); \
					} else {																				\
						FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == (void *)((u32)pMemoryBlock+16)), "FangAlign (Destructor): Use 'fdelete_array()' instead of 'delete []'." ); \
					}																						\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
				}																							\
																											\
				void operator delete []( void *pMemoryBlock, cchar *pszFileName, s32 nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);								\
					FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = pszFileName;						\
					FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = nLineNum;							\
					FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = pMemoryBlock;						\
					FRes_nDebugStackCount++;																\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
					FRes_nDebugStackCount--;																\
				}


		#define FCLASS_STACKMEM_NOALIGN( szClassName )														\
				public:																						\
																											\
				enum { CLASS_BYTE_ALIGNMENT = 4 };															\
																											\
				void *operator new( u32 nBytesToAlloc ) {											\
					FASSERT_NOW_MSG( "Fang: Use 'fnew' instead of 'new'." );								\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( NULL, 0, nBytesToAlloc, CLASS_BYTE_ALIGNMENT );		\
					}																						\
					return fres_DebugNewWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, #szClassName, 0, NULL, 0 ); \
				}																							\
																											\
				void *operator new( u32 nBytesToAlloc, cchar *pszFileName, s32 nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];															\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( pszFileName, nLineNum, nBytesToAlloc, CLASS_BYTE_ALIGNMENT ); \
					}																						\
					return fres_DebugNewWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, #szClassName, 0, pszFileName, nLineNum ); \
				}																							\
																											\
				void *operator new []( u32 nBytesToAlloc ) {										\
					FASSERT_NOW_MSG( "Fang: Use 'fnew' instead of 'new'." );								\
					FASSERT( !_FRes_bTestForDestructor );													\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						return fres_Debug_AlignedAlloc( NULL, 0, nBytesToAlloc, CLASS_BYTE_ALIGNMENT );		\
					}																						\
					return fres_DebugNewArrayWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, 0, #szClassName, sizeof(szClassName), NULL, 0 ); \
				}																							\
																											\
				void *operator new []( u32 nBytesToAlloc, cchar *pszFileName, s32 nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					if( !_FRes_bTestForDestructor ) {														\
						_FRes_bTestForDestructor = TRUE;													\
						new( NULL, 0, 0, 0, 0 ) szClassName[2];												\
						if( !_FRes_bHasDestructor ) {														\
							return fres_Debug_AlignedAlloc( pszFileName, nLineNum, nBytesToAlloc, CLASS_BYTE_ALIGNMENT ); \
						}																					\
						return fres_DebugNewArrayWithDestructor( nBytesToAlloc, CLASS_BYTE_ALIGNMENT, 0, #szClassName, sizeof(szClassName), pszFileName, nLineNum ); \
					} else {																				\
						_FRes_bTestForDestructor = FALSE;													\
						_FRes_bHasDestructor = ( nBytesToAlloc != 2*sizeof(szClassName) );					\
						return (void *)NULL;																\
					}																						\
				}																							\
																											\
				void operator delete( void *pMemoryBlock ) {												\
					FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == pMemoryBlock), "Fang: Use 'fdelete()' instead of 'delete'." ); \
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
				}																							\
																											\
				void operator delete( void *pMemoryBlock, cchar *pszFileName, s32 nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);								\
					FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = pszFileName;						\
					FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = nLineNum;							\
					FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = pMemoryBlock;						\
					FRes_nDebugStackCount++;																\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
					FRes_nDebugStackCount--;																\
				}																							\
																											\
				void operator delete []( void *pMemoryBlock ) {												\
					_FRes_bTestForDestructor = TRUE;														\
					new( NULL, 0, 0, 0, 0 ) szClassName[2];													\
					if( !_FRes_bHasDestructor ) {															\
						FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == pMemoryBlock), "FangAlign (No Destructor): Use 'fdelete_array()' instead of 'delete []'." ); \
					} else {																				\
						FASSERT_MSG( FRes_nDebugStackCount && (FRes_aDebugStack[FRes_nDebugStackCount-1].pMemBlock == (void *)((u32)pMemoryBlock+16)), "FangAlign (Destructor): Use 'fdelete_array()' instead of 'delete []'." ); \
					}																						\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
				}																							\
																											\
				void operator delete []( void *pMemoryBlock, cchar *pszFileName, s32 nLineNum, u32 nDummy1, u32 nDummy2, u32 nDummy3 ) { \
					FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);								\
					FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = pszFileName;						\
					FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = nLineNum;							\
					FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = pMemoryBlock;						\
					FRes_nDebugStackCount++;																\
					fres_DebugDelete( pMemoryBlock, CLASS_BYTE_ALIGNMENT, #szClassName, sizeof(szClassName) );	\
					FRes_nDebugStackCount--;																\
				}


		#define fnew new( __FILE__, __LINE__, 0, 0, 0 )

		#define fdelete( pMemoryBlock ) {																	\
					if( (pMemoryBlock) ) {																	\
						FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);							\
						FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = __FILE__;						\
						FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = __LINE__;						\
						FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = (pMemoryBlock);					\
						FRes_nDebugStackCount++;															\
						delete (pMemoryBlock);																\
						FASSERT( FRes_nDebugStackCount );													\
						FRes_nDebugStackCount--;															\
					}																						\
				}

		#define fdelete_array( pMemoryBlock ) {																\
					if( (pMemoryBlock) ) {																	\
						FASSERT( FRes_nDebugStackCount < FRES_DEBUG_STACK_SIZE);							\
						FRes_aDebugStack[FRes_nDebugStackCount].pszFilename = __FILE__;						\
						FRes_aDebugStack[FRes_nDebugStackCount].nLineNum = __LINE__;						\
						FRes_aDebugStack[FRes_nDebugStackCount].pMemBlock = (pMemoryBlock);					\
						FRes_nDebugStackCount++;															\
						delete [] (pMemoryBlock);															\
						FASSERT( FRes_nDebugStackCount );													\
						FRes_nDebugStackCount--;															\
					}																						\
				}

	#else

		extern void *_fres_AlignedAlloc( u32 nBytes, u32 nByteAlignment );
		extern void *_fres_AlignedAllocWithOffset( u32 nBytes, u32 nByteAlignment, u32 nOffset );

		#define FCLASS_STACKMEM_ALIGN( szClassName )														\
				public:																						\
																											\
				enum { CLASS_BYTE_ALIGNMENT = FCLASS_BYTE_ALIGN };											\
																											\
				void *operator new( u32 nBytesToAlloc ) {											\
					return _fres_AlignedAlloc( nBytesToAlloc, CLASS_BYTE_ALIGNMENT );						\
				}																							\
																											\
				void *operator new []( u32 nBytesToAlloc ) {										\
					return _fres_AlignedAlloc( nBytesToAlloc, CLASS_BYTE_ALIGNMENT );						\
				}																							\
																											\
				void operator delete( void *pMemoryBlock ) {												\
					pMemoryBlock;																						\
				}																							\
																											\
				void operator delete []( void *pMemoryBlock ) {												\
					pMemoryBlock;																						\
				}


		#define FCLASS_STACKMEM_NOALIGN( szClassName )														\
				public:																						\
																											\
				enum { CLASS_BYTE_ALIGNMENT = 4 };															\
																											\
				void *operator new( u32 nBytesToAlloc ) {											\
					return _fres_AlignedAlloc( nBytesToAlloc, CLASS_BYTE_ALIGNMENT );						\
				}																							\
																											\
				void *operator new []( u32 nBytesToAlloc ) {										\
					return _fres_AlignedAlloc( nBytesToAlloc, CLASS_BYTE_ALIGNMENT );						\
				}																							\
																											\
				void operator delete( void *pMemoryBlock ) {												\
					pMemoryBlock;																						\
				}																							\
																											\
				void operator delete []( void *pMemoryBlock ) {												\
					pMemoryBlock;																						\
				}


		#define fnew new
		#define fdelete( pMemoryBlock ) delete pMemoryBlock
		#define fdelete_array( pMemoryBlock ) delete [] pMemoryBlock

	#endif

//ARG - >>>>>
//--------------------------------------------------------------------------------------------------------------------------
// Playstation2 Implementation:
//--------------------------------------------------------------------------------------------------------------------------
#elif FANG_PLATFORM_PS2

	//ARG - same as XB none DEBUG at the moment

	extern void *_fres_AlignedAlloc( u32 nBytes, u32 nByteAlignment );
	extern void *_fres_AlignedAllocWithOffset( u32 nBytes, u32 nByteAlignment, u32 nOffset );

	#define FCLASS_STACKMEM_ALIGN( szClassName )														\
			public:																						\
																										\
			enum { CLASS_BYTE_ALIGNMENT = FCLASS_BYTE_ALIGN };											\
																										\
			void *operator new( unsigned int nBytesToAlloc ) {											\
				return _fres_AlignedAlloc( nBytesToAlloc, CLASS_BYTE_ALIGNMENT );						\
			}																							\
																										\
			void *operator new []( unsigned int nBytesToAlloc ) {										\
				return _fres_AlignedAlloc( nBytesToAlloc, CLASS_BYTE_ALIGNMENT );						\
			}																							\
																										\
			void operator delete( void *pMemoryBlock ) {												\
			}																							\
																										\
			void operator delete []( void *pMemoryBlock ) {												\
			}


	#define FCLASS_STACKMEM_NOALIGN( szClassName )														\
			public:																						\
																										\
			enum { CLASS_BYTE_ALIGNMENT = 4 };															\
																										\
			void *operator new( unsigned int nBytesToAlloc ) {											\
				return _fres_AlignedAlloc( nBytesToAlloc, CLASS_BYTE_ALIGNMENT );						\
			}																							\
																										\
			void *operator new []( unsigned int nBytesToAlloc ) {										\
				return _fres_AlignedAlloc( nBytesToAlloc, CLASS_BYTE_ALIGNMENT );						\
			}																							\
																										\
			void operator delete( void *pMemoryBlock ) {												\
			}																							\
																										\
			void operator delete []( void *pMemoryBlock ) {												\
			}


	#define fnew new
	#define fdelete( pMemoryBlock ) delete pMemoryBlock
	#define fdelete_array( pMemoryBlock ) delete [] pMemoryBlock
//ARG - <<<<<
#else

	#error <Need memory management definitions for platform.>

#endif



#endif


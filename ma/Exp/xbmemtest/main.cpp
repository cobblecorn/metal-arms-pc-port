#include "ftimer.h"
#include <xtl.h>
#include <xgraphics.h>
#include <xgmath.h>
#include "xmmintrin.h"
#include "stdio.h"


#define _SAMPLE_COUNT	100000
#define _VEC_COUNT		10000

__m128 _av[_VEC_COUNT];

__m128 *_pTestVec;
__m128 _SourceVec;


void _TestFunc( void );
void _OverheadFunc( void );



int main( int argc, char *argv[] ) {
	float fOHSecs, fTestSecs;
	int i;
	CFTimer Timer;
	char szText[200];

	ftimer_ModuleStartup();

	_pTestVec = _av;

	for( i=0; i<_VEC_COUNT; i++ ) {
		__asm cli

		Timer.Reset();
		__asm {
				mov		ecx, _SAMPLE_COUNT
			OHLabel:
				push	ecx
				call	_OverheadFunc
				pop		ecx
				dec		ecx
				jnz		OHLabel
		}
		fOHSecs = Timer.SampleSeconds();

		Timer.Reset();
		__asm {
				mov		ecx, _SAMPLE_COUNT
			OldLabel:
				push	ecx
				call	_TestFunc
				pop		ecx
				dec		ecx
				jnz		OldLabel
		}
		fTestSecs = Timer.SampleSeconds() - fOHSecs;

		__asm sti

		if( (fTestSecs * (1.0f / (float)_SAMPLE_COUNT) * FTimer_fClockFrequency) > 3.2f ) {
			sprintf( szText, "Cycles=%5.1f,   Address=0x%08x\n", (fTestSecs * (1.0f / (float)_SAMPLE_COUNT) * FTimer_fClockFrequency), _pTestVec );
			OutputDebugString( szText );
		}

		_pTestVec++;
	}

	for(;;);
	return 0;
}



void _TestFunc( void ) {
	__asm {
		movaps		xmm0, _SourceVec
		mov			eax, _pTestVec
		movaps		[eax], xmm0
	}
}



void _OverheadFunc( void ) {
}


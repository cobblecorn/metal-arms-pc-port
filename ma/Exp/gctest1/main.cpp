#include <dolphin.h>

#include "fang.h"
#include "fpadio.h"

void main( int argc, char* argv[] )
{
	OSInit();

	OSReport( "------------------------------------\n");
	OSReport( "----------- BEGIN ------------------\n");
	OSReport( "------------------------------------\n");
	OSReport( "argc = %u\n", argc );
	for( int u = 0; u < argc; ++u )
	{
		OSReport( "argv[ %u ] = %s\n", u, argv[ u ] );
	}
	OSReport( "------------------------------------\n");

fang_Init();

Fang_ConfigDefs.nRes_HeapBytes = 16*1024*1024;

if( !fang_Startup() )
{
	DEVPRINTF( "fang_Startup() failed.\n" );
	while(1){};
}

fpadio_Install();

FPadio_Sample_t **_paoSamples;
u32 _uValidSamples;
u32 _uSample, _uDev;
OSTick oTick;

while( 1 )
{
	oTick = OSGetTick();
//	while( 100 > OSTicksToMilliseconds( OSGetTick() - oTick ) ){};

	fpadio_GetSamples( &_uValidSamples, &_paoSamples );

//	OSReport( "\n%d:", _uValidSamples );

	for( _uSample = 0; _uSample < _uValidSamples; ++_uSample )
	{
		for( _uDev = 0; _uDev < FPADIO_MAX_DEVICES; ++_uDev )
		{
			if( _paoSamples[ _uDev ][ _uSample ].bValid )
			{
				if( ( 0 == _uDev ) && ( 0 == _uSample ) )
//				if( 0 == _uDev )
				{
/*
					OSReport(	"%f\t%f\t%f\n",
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_CROSS_BOTTOM - 1 ],
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_STICK_RIGHT_Y - 1 ],
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_TRIGGER_LEFT - 1 ] );
*/
/*
					OSReport(	"%f\t%f\t%f\n",
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_CROSS_RIGHT - 1 ],
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_CROSS_LEFT - 1 ],
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_CROSS_TOP - 1 ] );
*/
/*
					OSReport(	"%f\t%f\t%f\n",
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_TRIGGER_LEFT - 1 ],
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_TRIGGER_RIGHT - 1 ],
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_START - 1 ] );
*/
/*
					OSReport(	"%f\t%f\t%f\n",
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_GC_DBUTTON_TRIGGER_LEFT - 1 ],
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_GC_DBUTTON_TRIGGER_RIGHT - 1 ],
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z - 1 ] );
*/
/*
					OSReport(	"%f\t%f\t%f\t%f\n",
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_DPAD_X - 1 ],
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_DPAD_Y - 1 ],
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_STICK_LEFT_X - 1 ],
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_STICK_LEFT_Y - 1 ] );
*/
/*
					OSReport(	"%f\t%f\n",
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_STICK_RIGHT_X - 1 ],
								_paoSamples[ _uDev ][ _uSample ].afInputValues[ FPADIO_INPUT_STICK_RIGHT_Y - 1 ] );
*/
				}
			}
		}
	}

	if( 0.3f < _paoSamples[ 0 ][ 0 ].afInputValues[ FPADIO_INPUT_CROSS_BOTTOM - 1 ] )
	{
		fpadio_SetForcefeedback( 0, 1.0f, 1.0f );
	}
	else
	{
		fpadio_SetForcefeedback( 0, 0.0f, 0.0f );
	}
	fpadio_ApplyAllForcefeedbacks();

}

	while( 1 ){};

	OSReport( "------------------------------------\n");
	OSReport( "------------ END -------------------\n");
	OSReport( "------------------------------------\n");
}

/*
u32 uIndex = 0;
for( ; uIndex < 600; ++uIndex )
{
	VIWaitForRetrace();
}
*/
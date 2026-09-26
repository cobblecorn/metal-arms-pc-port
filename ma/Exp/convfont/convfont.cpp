// convfont.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "io.h"
#include "math.h"
#include "convfont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		return 1;
	}

	if( argc != 2 )
	{
		cerr << "wrong arguments";
		return 1;
	}

	FILE *poInputFile = fopen( argv[ 1 ], "rb+" );
	if( ! poInputFile )
	{
		cerr << "can't open file";
		return 1;
	}

	int nFileSize = filelength( fileno( poInputFile ) );

	unsigned char *pauBuff = (unsigned char *)malloc( nFileSize );
	if( ! pauBuff )
	{
		fclose( poInputFile );
		cerr << "can't malloc()";
		return 1;
	}

	if( nFileSize != fread( pauBuff, 1, nFileSize, poInputFile ) )
	{
		free( pauBuff );
		fclose( poInputFile );
		cerr << "can't read file";
		return 1;
	}

//tga header 18 bytes.
//tga footer 26 bytes.

	for( int nIndex = 18; nIndex < ( nFileSize - 26 ); nIndex += 4 )
	{
		if( ( pauBuff[ nIndex + 0 ] == 0 ) && // BB
			( pauBuff[ nIndex + 1 ] == 0 ) && // GG
			( pauBuff[ nIndex + 2 ] == 0 ) )  // RR
		{
			pauBuff[ nIndex + 3 ] = 0; // AA
		}
		else if( ( pauBuff[ nIndex + 0 ] == 0xff ) && // BB
				 ( pauBuff[ nIndex + 1 ] == 0xff ) && // GG
				 ( pauBuff[ nIndex + 2 ] == 0xff ) )  // RR
		{
			pauBuff[ nIndex + 3 ] = 0xff; // AA
		}
		else
		{
			pauBuff[ nIndex + 3 ] = 1; // AA
		}

		pauBuff[ nIndex + 0 ] = 0xff; // BB
		pauBuff[ nIndex + 1 ] = 0xff; // GG
		pauBuff[ nIndex + 2 ] = 0xff; // RR
	}

	if( fseek( poInputFile, 0, SEEK_SET ) )
	{
		free( pauBuff );
		fclose( poInputFile );
		cerr << "can't seek in file";
		return 1;
	}

	if( nFileSize != fwrite( pauBuff, 1, nFileSize, poInputFile ) )
	{
		free( pauBuff );
		fclose( poInputFile );
		cerr << "can't writing file";
		return 1;
	}

	free( pauBuff );
	fclose( poInputFile );

	cout << "done";

	return 0;
}

//RR 255 -> 117
//gamma >1.54 <2.01

// 0.3000f: >1.5400 <1.5700
// 0.2990f: >1.5400 <1.5700
// 0.2125f: >1.9800 <2.0100

//GG 255 -> 204
//gamma >1.50 <2.45

// 0.5900f: >2.3600 <2.4200
// 0.5870f: >2.3800 <2.4500
// 0.7154f: >1.5000 <1.5400

//BB 255 ->  64
//gamma >1.57 <1.93

// 0.1140f: >1.5700 <1.5900
// 0.1100f: >1.5900 <1.6200
// 0.0721f: >1.9000 <1.9300

//float fInverse;
//float fGamma = 1.5400f; // RR
//float fGamma = 2.3800f; // GG
//float fGamma = 1.5700f; // BB

//float fGamma = 2.2000f; // photoshop

//while( fGamma <= 2.0100f ) // RR
//while( fGamma <= 2.500f ) // GG
//while( fGamma <= 1.9300f ) // BB
//{
/*
fInverse = 1.0f / fGamma;

printf( "\r\n" );

printf( "( 0.1815f * 255 ^ %f ) 1 / %f = %f => 117\r\n", fGamma, fInverse, pow( ( 0.1815f * pow( 255.0f, fGamma ) ), fInverse ) );
printf( "( 0.6150f * 255 ^ %f ) 1 / %f = %f => 204\r\n", fGamma, fInverse, pow( ( 0.6150f * pow( 255.0f, fGamma ) ), fInverse ) );
printf( "( 0.0480f * 255 ^ %f ) 1 / %f = %f =>  64\r\n", fGamma, fInverse, pow( ( 0.0480f * pow( 255.0f, fGamma ) ), fInverse ) );
*/
/*
// RR
printf( "( 0.3000f * 255 ^ %f ) 1 / %f = %f\r\n", fGamma, fInverse, pow( ( 0.3000f * pow( 255.0f, fGamma ) ), fInverse ) );
printf( "( 0.2990f * 255 ^ %f ) 1 / %f = %f\r\n", fGamma, fInverse, pow( ( 0.2990f * pow( 255.0f, fGamma ) ), fInverse ) );
printf( "( 0.2125f * 255 ^ %f ) 1 / %f = %f\r\n", fGamma, fInverse, pow( ( 0.2125f * pow( 255.0f, fGamma ) ), fInverse ) );
*/
/*
// GG
printf( "( 0.5900f * 255 ^ %f ) 1 / %f = %f\r\n", fGamma, fInverse, pow( ( 0.5900f * pow( 255.0f, fGamma ) ), fInverse ) );
printf( "( 0.5870f * 255 ^ %f ) 1 / %f = %f\r\n", fGamma, fInverse, pow( ( 0.5870f * pow( 255.0f, fGamma ) ), fInverse ) );
printf( "( 0.7154f * 255 ^ %f ) 1 / %f = %f\r\n", fGamma, fInverse, pow( ( 0.7154f * pow( 255.0f, fGamma ) ), fInverse ) );
*/
/*
// BB
printf( "( 0.1140f * 255 ^ %f ) 1 / %f = %f\r\n", fGamma, fInverse, pow( ( 0.1140f * pow( 255.0f, fGamma ) ), fInverse ) );
printf( "( 0.1100f * 255 ^ %f ) 1 / %f = %f\r\n", fGamma, fInverse, pow( ( 0.1100f * pow( 255.0f, fGamma ) ), fInverse ) );
printf( "( 0.0721f * 255 ^ %f ) 1 / %f = %f\r\n", fGamma, fInverse, pow( ( 0.0721f * pow( 255.0f, fGamma ) ), fInverse ) );
*/
//	fGamma += 0.0100f;
//}
/*
	for( int nIndex0 = 0; nIndex0 < 256; ++nIndex0 ) // RR
	{
		for( int nIndex1 = 0; nIndex1 < 256; ++nIndex1 ) // GG
		{
			for( int nIndex2 = 0; nIndex2 < 256; ++nIndex2 ) // BB
			{

pauBuff[ 18 + 0 + ( nIndex0 * 256 * 256 * 4 ) + ( nIndex1 * 256 * 4 ) + ( nIndex2 * 4 ) ] = (unsigned short)nIndex2; // BB
pauBuff[ 18 + 1 + ( nIndex0 * 256 * 256 * 4 ) + ( nIndex1 * 256 * 4 ) + ( nIndex2 * 4 ) ] = (unsigned short)nIndex1; // GG
pauBuff[ 18 + 2 + ( nIndex0 * 256 * 256 * 4 ) + ( nIndex1 * 256 * 4 ) + ( nIndex2 * 4 ) ] = (unsigned short)nIndex0; // RR
pauBuff[ 18 + 3 + ( nIndex0 * 256 * 256 * 4 ) + ( nIndex1 * 256 * 4 ) + ( nIndex2 * 4 ) ] = 0; // AA


			}
		}
	}
*/
/*
		pauBuff[ nIndex + 3 ] = 0; // AA
		pauBuff[ nIndex + 3 ] = 0x64; // AA
		pauBuff[ nIndex + 3 ] = 0xc0; // AA
		pauBuff[ nIndex + 3 ] = 0xff; // AA
// negatif
		pauBuff[ nIndex + 0 ] = ~pauBuff[ nIndex + 0 ]; // BB
		pauBuff[ nIndex + 1 ] = ~pauBuff[ nIndex + 1 ]; // GG
		pauBuff[ nIndex + 2 ] = ~pauBuff[ nIndex + 2 ]; // RR
// mono
		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 2 ] = ( ( ( ( pauBuff[ nIndex + 0 ] >> 7 ) + ( pauBuff[ nIndex + 1 ] >> 7 ) + ( pauBuff[ nIndex + 2 ] >> 7 ) ) > 1 ) ? 0xff : 0 );
// 111
		pauBuff[ nIndex + 0 ] = ( pauBuff[ nIndex + 0 ] & 0x80 ? 0xff : 0 ); // BB
		pauBuff[ nIndex + 1 ] = ( pauBuff[ nIndex + 1 ] & 0x80 ? 0xff : 0 ); // GG
		pauBuff[ nIndex + 2 ] = ( pauBuff[ nIndex + 2 ] & 0x80 ? 0xff : 0 ); // RR
// 111
		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 0 ] & 0x80; // BB
		pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 1 ] & 0x80; // GG
		pauBuff[ nIndex + 2 ] = pauBuff[ nIndex + 2 ] & 0x80; // RR
// 222
		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 0 ] & 0xc0; // BB
		pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 1 ] & 0xc0; // GG
		pauBuff[ nIndex + 2 ] = pauBuff[ nIndex + 2 ] & 0xc0; // RR
// 333
		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 0 ] & 0xe0; // BB
		pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 1 ] & 0xe0; // GG
		pauBuff[ nIndex + 2 ] = pauBuff[ nIndex + 2 ] & 0xe0; // RR
// 444
		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 0 ] & 0xf0; // BB
		pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 1 ] & 0xf0; // GG
		pauBuff[ nIndex + 2 ] = pauBuff[ nIndex + 2 ] & 0xf0; // RR
// 555
		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 0 ] & 0xf8; // BB
		pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 1 ] & 0xf8; // GG
		pauBuff[ nIndex + 2 ] = pauBuff[ nIndex + 2 ] & 0xf8; // RR
// 666
		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 0 ] & 0xfc; // BB
		pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 1 ] & 0xfc; // GG
		pauBuff[ nIndex + 2 ] = pauBuff[ nIndex + 2 ] & 0xfc; // RR
// 341
		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 0 ] & 0xe0; // BB
		pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 1 ] & 0xf0; // GG
		pauBuff[ nIndex + 2 ] = pauBuff[ nIndex + 2 ] & 0x80; // RR
// 242
		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 0 ] & 0xc0; // BB
		pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 1 ] & 0xf0; // GG
		pauBuff[ nIndex + 2 ] = pauBuff[ nIndex + 2 ] & 0xc0; // RR
// 777
		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 0 ] & 0xfe; // BB
		pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 1 ] & 0xfe; // GG
		pauBuff[ nIndex + 2 ] = pauBuff[ nIndex + 2 ] & 0xfe; // RR
// 332
		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 0 ] & 0xe0; // BB
		pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 1 ] & 0xe0; // GG
		pauBuff[ nIndex + 2 ] = pauBuff[ nIndex + 2 ] & 0xc0; // RR
// 251
		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 0 ] & 0xc0; // BB
		pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 1 ] & 0xf8; // GG
		pauBuff[ nIndex + 2 ] = pauBuff[ nIndex + 2 ] & 0x80; // RR
*/
/*
//grayscale weighted - RANDOM TEST
		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 2 ] = ( pauBuff[ nIndex + 0 ] * 0.1656f ) + ( pauBuff[ nIndex + 1 ] * 0.5298f ) + ( pauBuff[ nIndex + 2 ] * 0.3046f );
//grayscale weighted - current(?) style
		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 2 ] = (unsigned short)( ( (float)pauBuff[ nIndex + 0 ] * 0.0721f ) + ( (float)pauBuff[ nIndex + 1 ] * 0.7154f ) + ( (float)pauBuff[ nIndex + 2 ] * 0.2125f ) );
//grayscale weighted - old(?) style
		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 2 ] = ( pauBuff[ nIndex + 0 ] * 0.114f ) + ( pauBuff[ nIndex + 1 ] * 0.587f ) + ( pauBuff[ nIndex + 2 ] * 0.299f );
//grayscale avg (B from HSB)
		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 2 ] = ( pauBuff[ nIndex + 0 ] + pauBuff[ nIndex + 1 ] + pauBuff[ nIndex + 2 ] ) / 3;
//grayscale brightness (V from HSV)
		uMax = pauBuff[ nIndex + 0 ];

		if( uMax < pauBuff[ nIndex + 1 ] )
		{
			uMax = pauBuff[ nIndex + 1 ];
		}

		if( uMax < pauBuff[ nIndex + 2 ] )
		{
			uMax = pauBuff[ nIndex + 2 ];
		}

		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 2 ] = uMax;
*/
/*
		pauBuff[ nIndex + 0 ] = 0; // BB
		pauBuff[ nIndex + 1 ] = 0; // GG
//			pauBuff[ nIndex + 2 ] = 0; // RR
		pauBuff[ nIndex + 3 ] = 0; // AA
*/
/*
	unsigned char uMax, uMin;

	for( int nIndex = 18; nIndex < ( nFileSize - 4 ); nIndex += 4 )
	{
//grayscale lightness (L from HLS)
		uMin = uMax = pauBuff[ nIndex + 0 ];

		if( uMax < pauBuff[ nIndex + 1 ] )
		{
			uMax = pauBuff[ nIndex + 1 ];
		}

		if( uMax < pauBuff[ nIndex + 2 ] )
		{
			uMax = pauBuff[ nIndex + 2 ];
		}

		if( uMin > pauBuff[ nIndex + 1 ] )
		{
			uMin = pauBuff[ nIndex + 1 ];
		}

		if( uMin > pauBuff[ nIndex + 2 ] )
		{
			uMin = pauBuff[ nIndex + 2 ];
		}

		pauBuff[ nIndex + 0 ] = pauBuff[ nIndex + 1 ] = pauBuff[ nIndex + 2 ] = (unsigned char)( ( (float)uMin + (float)uMax ) / 2.0f );
	}
*/
/*
		if( ( pauBuff[ nIndex + 0 ] != 0 ) || // BB
			( pauBuff[ nIndex + 1 ] != 0 ) || // GG
			( pauBuff[ nIndex + 2 ] != 0 ) )  // RR
		{
			// AA
			pauBuff[ nIndex + 3 ] = pauBuff[ nIndex + 0 ];
			if( pauBuff[ nIndex + 1 ] > pauBuff[ nIndex + 3 ] ) pauBuff[ nIndex + 3 ] = pauBuff[ nIndex + 1 ];
			if( pauBuff[ nIndex + 2 ] > pauBuff[ nIndex + 3 ] ) pauBuff[ nIndex + 3 ] = pauBuff[ nIndex + 2 ];
		}
*/
/*
		if( ( pauBuff[ nIndex + 0 ] == 0xff ) || // BB
			( pauBuff[ nIndex + 1 ] == 0xff ) || // GG
			( pauBuff[ nIndex + 2 ] == 0xff ) )  // RR
		{
			// AA
			pauBuff[ nIndex + 3 ] = 0xff;
		}
		else if( ( pauBuff[ nIndex + 0 ] == 0x7f ) || // BB
				 ( pauBuff[ nIndex + 1 ] == 0x7f ) || // GG
				 ( pauBuff[ nIndex + 2 ] == 0x7f ) )  // RR
		{
			// AA
			pauBuff[ nIndex + 0 ] = 0;
			pauBuff[ nIndex + 1 ] = 0;
			pauBuff[ nIndex + 2 ] = 0;
			pauBuff[ nIndex + 3 ] = 0xff;
		}
		else
		{
			pauBuff[ nIndex + 0 ] = 0xff;
			pauBuff[ nIndex + 1 ] = 0xff;
			pauBuff[ nIndex + 2 ] = 0xff;
			pauBuff[ nIndex + 3 ] = 0;
		}
*/

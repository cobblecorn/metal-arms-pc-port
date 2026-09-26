// screenshot_port.cpp - stand-in for ma/App/ma/win/screenshot.cpp.
//
// The original module captured the front buffer through D3D8 and wrote TGA/JPEG
// files using MFC and Intel's IJL library. It was a developer convenience (and was
// documented as not working in windowed mode). This keeps the same interface so the
// game links and its mode/type settings behave, but does not capture anything yet.
//
// TODO: implement capture (GetBackBuffer -> GetRenderTargetData -> WIC encoder).

#include "fang.h"
#include "screenshot.h"

static BOOL _bSingleShot = TRUE;
static ScreenShotOutputType_e _nType = SCREENSHOT_OUTPUT_TYPE_TGA;

void screenshot_Init( cchar *, BOOL )
{
}

void screenshot_Shutdown( void )
{
}

void screenshot_Work( void )
{
}

void screenshot_SetScreenShotMode( BOOL bSingle )
{
	_bSingleShot = bSingle;
}

BOOL screenshot_GetScreenShotMode( void )
{
	return _bSingleShot;
}

void screenshot_SetScreenShotType( ScreenShotOutputType_e nScreenShotType )
{
	_nType = nScreenShotType;
}

ScreenShotOutputType_e screenshot_GetScreenShotType( void )
{
	return _nType;
}

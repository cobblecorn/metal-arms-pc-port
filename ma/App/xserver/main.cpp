#include "fang.h"
#include "floop.h"
#include "ftext.h"
#include "fvid.h"
#include "fxfm.h"
#include "faudio.h"
#include "fpadio.h"
#include "fdraw.h"
#include "frenderer.h"
#include "fperf.h"
#include "dx/fserver.h"
#include <xtl.h>
#include <XbDm.h>

static BOOL _ServerInit(void *pParameter)
{
//	floop_EnableGovernor( TRUE );
	return TRUE;
}

static void _ServerTerm( FLoopTermCode_t nTermCode, void *pParameter )
{

}

static BOOL _ServerMain( BOOL bExitRequest, void *pParameter )
{
	return !bExitRequest;
}

int main(int argc, char *argv[])
{
	fang_Init();
		
	//Fang_ConfigDefs.pszFile_MasterFilePathName = "d:\\mettlearms_xb.mst";

	if (!fang_Startup())
	{
		// Trouble starting up Fang...
		OutputDebugString( "Could not start up Fang :(\n" );
		DmReboot( DMBOOT_WARM );
		for(;;);
	}

	if (!fserver_GraphicsStartup())
	{
		DEVPRINTF("failed graphics\n");
		DmReboot( DMBOOT_WARM );
		for(;;);
	}

	FPerf_nDisplayPerfType = FPERF_TYPE_NONE;

	fserver_SetTakeover(TRUE);

	floop_InstallGameloop(_ServerInit, _ServerMain, _ServerTerm, 0, 60, 15);

	DmReboot(DMBOOT_WARM);
	for(;;);

	return 0;
}

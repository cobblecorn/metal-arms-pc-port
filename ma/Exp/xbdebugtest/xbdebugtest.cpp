// xbdebugtest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "xboxdbg.h"
#include "conio.h"
#include "stdio.h"

void Reboot( int argc,char **argv );

static char *_aszDirFile[] = {
	"",
	"xe:\\samples\\fresnelreflect",
	"fresnelreflect.xbe"
};


int main( int argc, char* argv[] ) {
#if 1
	DM_FILE_ATTRIBUTES  FileAttributes;

	printf( "Test\n" );

	DmReboot( /*DMBOOT_WARM | */DMBOOT_WAIT );
	Sleep(10000);
//	DmSetTitle( "xe:\\samples\\fresnelreflect", "fresnelreflect.xbe", "" );
	DmSetTitle( "xe:\\samples\\DolphinClassic", "DolphinClassic.xbe", "" );
	DmGo();

//	DmGetFileAttributes( "xe:\\dxt\\vt_cgrf.dxt", &FileAttributes );
//	printf( "%u\n", FileAttributes.SizeLow );
#else
#endif

	Reboot( 3, &_aszDirFile[0] );

	printf( "Done. Press a key...\n" );
	getch();
	return 0;
}


//#include "stdafx.h"
//(stdafx.h includes xboxdg.h)

HANDLE g_hevtReboot;

DWORD CALLBACK XboxNotification( DWORD dwMsg, DWORD dwParam ) {
	switch(dwMsg & DM_NOTIFICATIONMASK)
	{
	case DM_EXEC:
		if(dwParam == DMN_EXEC_PENDING)
			SetEvent(g_hevtReboot);
		break;
	}
	
	return 0;
}


void Reboot( int argc,char **argv ) {
	HRESULT hr;

	/* Cut down on repeated connection requests */
	DmUseSharedConnection(TRUE);

	if( argc != 3 ) {
		printf("Usage: xbrun [path to directory XBE is stored in] [filename of XBE]\n(Note: directory will usually be on E: drive");
		return;
	}

	// Determine if file exists before rebooting
	hr = DmSetTitle(argv[1], argv[2], NULL);
	if (FAILED(hr))
	{ // hr = 82db0002 for no-such-file
		if (0x82db0002 == hr)
			printf("No such file exists\n");
		else
			printf("Set Title failed...");
		return;
	}
	
	printf("Rebooting XBOX...\n");
	g_hevtReboot = CreateEvent(NULL, FALSE, FALSE, NULL);
	PDMN_SESSION pSession;
	hr = DmOpenNotificationSession(DM_PERSISTENT, &pSession);
	if (FAILED(hr))
		return;
	hr = DmNotify(pSession, DM_EXEC, XboxNotification);
	if (FAILED(hr))
		return;
	hr = DmReboot(DMBOOT_WARM | DMBOOT_WAIT); 
	if (FAILED(hr))
		return;
	if(WaitForSingleObject(g_hevtReboot, 30000) == WAIT_TIMEOUT)
	{
		hr = XBDM_CONNECTIONLOST;
		return;
	}
	CloseHandle(g_hevtReboot);
	
	// Add your own argv[] parsing for a better calling syntax
	printf("Running %s at path %s...\n",argv[2], argv[1]);
	hr = DmSetTitle(argv[1], argv[2], NULL); // no command line for now
	if (FAILED(hr))
	{ // hr = 82db0002 for no-such-file
		if (0x82db0002 == hr)
			printf("After reboot, path was invalid\n");
	}
	DmGo();
	
	DmCloseNotificationSession(pSession);
	
	DmUseSharedConnection(FALSE);
}

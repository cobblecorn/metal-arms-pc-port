# Microsoft Developer Studio Project File - Name="ma_win" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ma_win - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ma_win.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ma_win.mak" CFG="ma_win - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ma_win - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ma_win - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "ma_win - Win32 Test" (based on "Win32 (x86) Application")
!MESSAGE "ma_win - Win32 Production" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/Src/App/ma_win", LIDAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ma_win - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /MD /W3 /Gi /GX /Zi /O2 /Ob2 /I "..\..\..\lib\SmallAmx" /I "..\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /YX"fang.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 fang2win_r.lib dinput8.lib dxguid.lib d3d8.lib d3dx8.lib winmm.lib dsound.lib smallamxwin_r.lib msacm32.lib winmm.lib /nologo /subsystem:windows /incremental:yes /map /debug /machine:I386

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "..\..\..\lib\SmallAmx" /I "..\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 fang2win_d.lib dinput8.lib dxguid.lib d3d8.lib d3dx8.lib winmm.lib dsound.lib smallamxwin_d.lib msacm32.lib winmm.lib /nologo /subsystem:windows /map /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ma_win___Win32_Test"
# PROP BASE Intermediate_Dir "ma_win___Win32_Test"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Test"
# PROP Intermediate_Dir "Test"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /Gi /GX /O2 /Ob2 /I "..\..\lib\SmallAmx" /I ".." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /Gi /GX /O2 /Ob2 /I "..\..\..\lib\SmallAmx" /I "..\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "_FANGDEF_PLATFORM_WIN" /D "_FANGDEF_TEST_BUILD" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 fang2win_r.lib dinput8.lib dxguid.lib d3d8.lib d3dx8.lib winmm.lib dsound.lib smallamxwin_r.lib ws2_32.lib /nologo /subsystem:windows /incremental:yes /machine:I386
# ADD LINK32 fang2win_t.lib dinput8.lib dxguid.lib d3d8.lib d3dx8.lib winmm.lib dsound.lib smallamxwin_t.lib ws2_32.lib msacm32.lib winmm.lib /nologo /subsystem:windows /incremental:yes /map /machine:I386

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ma_win___Win32_Production"
# PROP BASE Intermediate_Dir "ma_win___Win32_Production"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Production"
# PROP Intermediate_Dir "Production"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /Gi /GX /O2 /Ob2 /I "..\..\lib\SmallAmx" /I ".." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /Gi /GX /O2 /Ob2 /I "..\..\..\lib\SmallAmx" /I "..\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "_FANGDEF_PLATFORM_WIN" /D "_FANGDEF_PRODUCTION_BUILD" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 fang2win_r.lib dinput8.lib dxguid.lib d3d8.lib d3dx8.lib winmm.lib dsound.lib smallamxwin_r.lib ws2_32.lib /nologo /subsystem:windows /incremental:yes /machine:I386
# ADD LINK32 fang2win_p.lib dinput8.lib dxguid.lib d3d8.lib d3dx8.lib winmm.lib dsound.lib smallamxwin_p.lib ws2_32.lib msacm32.lib winmm.lib /nologo /subsystem:windows /incremental:yes /map /machine:I386

!ENDIF 

# Begin Target

# Name "ma_win - Win32 Release"
# Name "ma_win - Win32 Debug"
# Name "ma_win - Win32 Test"
# Name "ma_win - Win32 Production"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Win Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\fangpch.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

# ADD CPP /Yc"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /Yc"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FileInfo.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\InputEmulation.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ma_win.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ma_winDlg.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MasterFileCompile.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PickAsset.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pickdir.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenGrab.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\screenshot.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Settings.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

# ADD CPP /YX

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

# ADD CPP /Yc"stdafx.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

# ADD CPP /Yc"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\VidMode.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\winprintf.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# End Group
# Begin Group "AI Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Ai\AI.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AI3DBotMover.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AIBotMover.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AIBrain.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AIBrainman.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AIBrainReact.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBrainUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBTATable.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBuilder.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AIControlGoals.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AIEdgeLock.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AIEnviro.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIFormations.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIFSM.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIGameUtils.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AIGraph.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AiGraphSearcher.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\aigroup.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIHazard.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AIKnowledge.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AIMain.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AIMover.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AIPath.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AIPatrolPath.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AIRooms.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AIThought.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AIThoughtBiped.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Ai\AIWeaponCtrl.cpp
# End Source File
# End Group
# Begin Group "Entity Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\bot.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\bot_data.cpp
# End Source File
# Begin Source File

SOURCE=..\botanim.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\botblink.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\botblink_data.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\botgrunt.cpp
# End Source File
# Begin Source File

SOURCE=..\botgrunt_data.cpp
# End Source File
# Begin Source File

SOURCE=..\botpred.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\botpred_data.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\bottitan.cpp
# End Source File
# Begin Source File

SOURCE=..\bottitan_data.cpp
# End Source File
# Begin Source File

SOURCE=..\DestructEntity.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Door.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ebox.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\econsole.cpp
# End Source File
# Begin Source File

SOURCE=..\EGoodie.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\eline.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\entity.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\entitycontrol.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\EParticle.cpp
# End Source File
# Begin Source File

SOURCE=..\epoint.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\eproj.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\eproj_arrow.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\eproj_cleaner.cpp
# End Source File
# Begin Source File

SOURCE=..\eproj_grenade.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\eproj_linear.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\eproj_saw.cpp
# End Source File
# Begin Source File

SOURCE=..\eproj_swarmer.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\esphere.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\espline.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ESwitch.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\meshentity.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\weapon.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\weapon_blaster.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\weapon_chaingun.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_cleaner.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_flamer.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_gren.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\weapon_hand.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\weapon_laser.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\weapon_mortar.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_ripper.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_rivet.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\weapon_rocket.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\weapon_scope.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_spew.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_tether.cpp
# End Source File
# Begin Source File

SOURCE=..\ZipLine.cpp
# End Source File
# End Group
# Begin Group "User Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\user_albert.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\user_john.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\user_justin.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\user_mike.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\user_pat.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\user_steve.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# End Group
# Begin Group "Barter Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\BarterSound.cpp
# End Source File
# Begin Source File

SOURCE=..\BarterSystem.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\BarterTypes.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Shady.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ShadyAnim.cpp
# End Source File
# Begin Source File

SOURCE=..\Slim.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\SSTable.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# End Group
# Begin Group "BotTalk Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\BotTalkAction.cpp
# End Source File
# Begin Source File

SOURCE=..\BotTalkData.cpp
# End Source File
# Begin Source File

SOURCE=..\BotTalkInst.cpp
# End Source File
# Begin Source File

SOURCE=..\TalkSystem2.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=..\Actor.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\BlinkGlow.cpp
# End Source File
# Begin Source File

SOURCE=..\BlinkShell.cpp
# End Source File
# Begin Source File

SOURCE=..\BlinkSpeed.cpp
# End Source File
# Begin Source File

SOURCE=..\BotFx.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\CamActor.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\CamBarter.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\CamBot.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\CamDebug.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\CamManual.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\CamSimple.cpp
# End Source File
# Begin Source File

SOURCE=..\DamageSystem.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\debris.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\explosion.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\flamer.cpp
# End Source File
# Begin Source File

SOURCE=..\FXStreamer.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\game.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gamecam.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\GameError.cpp
# End Source File
# Begin Source File

SOURCE=..\gameloop.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gamepad.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\GamePools.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gamesave.cpp
# End Source File
# Begin Source File

SOURCE=..\GoodieBag.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\GoodieProps.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gstring.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\guid.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Hud2.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\HudWeaponProfile.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Item.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ItemInst.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ItemRepository.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\LaserBeam.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\letterbox.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\level.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\leveltest.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\MAScriptTypes.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\MenuTypes.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\MuzzleFlash.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\PauseScreen.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\PickLevel.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\player.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\potmark.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ProTrack.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\PsPool.cpp
# End Source File
# Begin Source File

SOURCE=..\reticle.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\RoboBuddy.cpp
# End Source File
# Begin Source File

SOURCE=..\save.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\skybox.cpp
# End Source File
# Begin Source File

SOURCE=..\smoketrail.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\SpaceDock.cpp
# End Source File
# Begin Source File

SOURCE=..\tether.cpp
# End Source File
# Begin Source File

SOURCE=..\tracer.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Tripwire.cpp
# End Source File
# Begin Source File

SOURCE=..\VtxPool.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Workable.cpp

!IF  "$(CFG)" == "ma_win - Win32 Release"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Debug"

# ADD CPP /YX"fang.h"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Test"

!ELSEIF  "$(CFG)" == "ma_win - Win32 Production"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Win Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\FileInfo.h
# End Source File
# Begin Source File

SOURCE=.\ijl.h
# End Source File
# Begin Source File

SOURCE=.\InputEmulation.h
# End Source File
# Begin Source File

SOURCE=.\ma_win.h
# End Source File
# Begin Source File

SOURCE=.\ma_winDlg.h
# End Source File
# Begin Source File

SOURCE=.\MasterFileCompile.h
# End Source File
# Begin Source File

SOURCE=.\PickAsset.h
# End Source File
# Begin Source File

SOURCE=.\pickdir.h
# End Source File
# Begin Source File

SOURCE=.\ScreenGrab.h
# End Source File
# Begin Source File

SOURCE=.\screenshot.h
# End Source File
# Begin Source File

SOURCE=.\Settings.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\VidMode.h
# End Source File
# Begin Source File

SOURCE=.\winprintf.h
# End Source File
# End Group
# Begin Group "AI Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Ai\AI.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AiBotMover.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBrain.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBrainMan.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBrainUtils.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBTAPool.h
# End Source File
# Begin Source File

SOURCE=..\Ai\aiBuilder.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIControlGoals.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIEdgeLock.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIEnviro.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIFormations.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIGameUtils.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIGraph.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIGraphSearcher.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIGroup.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIHazard.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIKnowledge.h
# End Source File
# Begin Source File

SOURCE=..\Ai\aimain.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIMath.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIMover.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIMover.inl
# End Source File
# Begin Source File

SOURCE=..\Ai\AINodePools.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIPath.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIPath.inl
# End Source File
# Begin Source File

SOURCE=..\Ai\AIPatrolPath.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIRooms.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIThought.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIThoughtBiped.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIWeaponCtrl.h
# End Source File
# Begin Source File

SOURCE=..\Ai\apenew.h
# End Source File
# Begin Source File

SOURCE=..\Ai\ftl.h
# End Source File
# End Group
# Begin Group "Entity Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\bot.h
# End Source File
# Begin Source File

SOURCE=..\botanim.h
# End Source File
# Begin Source File

SOURCE=..\botblink.h
# End Source File
# Begin Source File

SOURCE=..\botgrunt.h
# End Source File
# Begin Source File

SOURCE=..\botpred.h
# End Source File
# Begin Source File

SOURCE=..\bottitan.h
# End Source File
# Begin Source File

SOURCE=..\DestructEntity.h
# End Source File
# Begin Source File

SOURCE=..\Door.h
# End Source File
# Begin Source File

SOURCE=..\ebox.h
# End Source File
# Begin Source File

SOURCE=..\EGoodie.h
# End Source File
# Begin Source File

SOURCE=..\eline.h
# End Source File
# Begin Source File

SOURCE=..\entity.h
# End Source File
# Begin Source File

SOURCE=..\entitycontrol.h
# End Source File
# Begin Source File

SOURCE=..\EParticle.h
# End Source File
# Begin Source File

SOURCE=..\epoint.h
# End Source File
# Begin Source File

SOURCE=..\eproj.h
# End Source File
# Begin Source File

SOURCE=..\eproj_arrow.h
# End Source File
# Begin Source File

SOURCE=..\eproj_cleaner.h
# End Source File
# Begin Source File

SOURCE=..\eproj_grenade.h
# End Source File
# Begin Source File

SOURCE=..\eproj_linear.h
# End Source File
# Begin Source File

SOURCE=..\eproj_swarmer.h
# End Source File
# Begin Source File

SOURCE=..\esphere.h
# End Source File
# Begin Source File

SOURCE=..\espline.h
# End Source File
# Begin Source File

SOURCE=..\ESwitch.h
# End Source File
# Begin Source File

SOURCE=..\meshentity.h
# End Source File
# Begin Source File

SOURCE=..\weapon.h
# End Source File
# Begin Source File

SOURCE=..\weapon_blaster.h
# End Source File
# Begin Source File

SOURCE=..\weapon_chaingun.h
# End Source File
# Begin Source File

SOURCE=..\weapon_cleaner.h
# End Source File
# Begin Source File

SOURCE=..\weapon_flamer.h
# End Source File
# Begin Source File

SOURCE=..\weapon_gren.h
# End Source File
# Begin Source File

SOURCE=..\weapon_hand.h
# End Source File
# Begin Source File

SOURCE=..\weapon_laser.h
# End Source File
# Begin Source File

SOURCE=..\weapon_mortar.h
# End Source File
# Begin Source File

SOURCE=..\weapon_ripper.h
# End Source File
# Begin Source File

SOURCE=..\weapon_rivet.h
# End Source File
# Begin Source File

SOURCE=..\weapon_rocket.h
# End Source File
# Begin Source File

SOURCE=..\weapon_scope.h
# End Source File
# Begin Source File

SOURCE=..\weapon_spew.h
# End Source File
# Begin Source File

SOURCE=..\weapon_tether.h
# End Source File
# Begin Source File

SOURCE=..\weapons.h
# End Source File
# Begin Source File

SOURCE=..\ZipLine.h
# End Source File
# End Group
# Begin Group "User Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\sas_user.h
# End Source File
# Begin Source File

SOURCE=..\user_albert.h
# End Source File
# Begin Source File

SOURCE=..\user_john.h
# End Source File
# Begin Source File

SOURCE=..\user_justin.h
# End Source File
# Begin Source File

SOURCE=..\user_mike.h
# End Source File
# Begin Source File

SOURCE=..\user_pat.h
# End Source File
# Begin Source File

SOURCE=..\user_steve.h
# End Source File
# End Group
# Begin Group "Barter Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\BarterSound.h
# End Source File
# Begin Source File

SOURCE=..\BarterSystem.h
# End Source File
# Begin Source File

SOURCE=..\BarterTypes.h
# End Source File
# Begin Source File

SOURCE=..\Shady.h
# End Source File
# Begin Source File

SOURCE=..\ShadyAnim.h
# End Source File
# Begin Source File

SOURCE=..\Slim.h
# End Source File
# Begin Source File

SOURCE=..\SSTable.h
# End Source File
# End Group
# Begin Group "BotTalk Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\BotTalkAction.h
# End Source File
# Begin Source File

SOURCE=..\BotTalkCsvFlags.h
# End Source File
# Begin Source File

SOURCE=..\BotTalkData.h
# End Source File
# Begin Source File

SOURCE=..\BotTalkInst.h
# End Source File
# Begin Source File

SOURCE=..\TalkSystem2.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\Actor.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBTATable.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIFSM.h
# End Source File
# Begin Source File

SOURCE=..\BlinkGlow.h
# End Source File
# Begin Source File

SOURCE=..\BlinkShell.h
# End Source File
# Begin Source File

SOURCE=..\BlinkSpeed.h
# End Source File
# Begin Source File

SOURCE=..\BotFx.h
# End Source File
# Begin Source File

SOURCE=..\CamActor.h
# End Source File
# Begin Source File

SOURCE=..\CamBarter.h
# End Source File
# Begin Source File

SOURCE=..\CamBot.h
# End Source File
# Begin Source File

SOURCE=..\CamDebug.h
# End Source File
# Begin Source File

SOURCE=..\CamManual.h
# End Source File
# Begin Source File

SOURCE=..\CamSimple.h
# End Source File
# Begin Source File

SOURCE=..\DamageSystem.h
# End Source File
# Begin Source File

SOURCE=..\debris.h
# End Source File
# Begin Source File

SOURCE=..\eproj_saw.h
# End Source File
# Begin Source File

SOURCE=..\explosion.h
# End Source File
# Begin Source File

SOURCE=..\flamer.h
# End Source File
# Begin Source File

SOURCE=..\FXStreamer.h
# End Source File
# Begin Source File

SOURCE=..\game.h
# End Source File
# Begin Source File

SOURCE=..\gamecam.h
# End Source File
# Begin Source File

SOURCE=..\GameError.h
# End Source File
# Begin Source File

SOURCE=..\gameloop.h
# End Source File
# Begin Source File

SOURCE=..\gamepad.h
# End Source File
# Begin Source File

SOURCE=..\GamePools.h
# End Source File
# Begin Source File

SOURCE=..\gamesave.h
# End Source File
# Begin Source File

SOURCE=..\GoodieBag.h
# End Source File
# Begin Source File

SOURCE=..\GoodieProps.h
# End Source File
# Begin Source File

SOURCE=..\gstring.h
# End Source File
# Begin Source File

SOURCE=..\gstring.h
# End Source File
# Begin Source File

SOURCE=..\guid.h
# End Source File
# Begin Source File

SOURCE=..\Hud2.h
# End Source File
# Begin Source File

SOURCE=..\HudWeaponProfile.h
# End Source File
# Begin Source File

SOURCE=..\Item.h
# End Source File
# Begin Source File

SOURCE=..\ItemInst.h
# End Source File
# Begin Source File

SOURCE=..\ItemRepository.h
# End Source File
# Begin Source File

SOURCE=..\LaserBeam.h
# End Source File
# Begin Source File

SOURCE=..\letterbox.h
# End Source File
# Begin Source File

SOURCE=..\level.h
# End Source File
# Begin Source File

SOURCE=..\leveltest.h
# End Source File
# Begin Source File

SOURCE=..\MAScriptTypes.h
# End Source File
# Begin Source File

SOURCE=..\MenuTypes.h
# End Source File
# Begin Source File

SOURCE=..\MeshTypes.h
# End Source File
# Begin Source File

SOURCE=..\MuzzleFlash.h
# End Source File
# Begin Source File

SOURCE=..\PauseScreen.h
# End Source File
# Begin Source File

SOURCE=..\PickLevel.h
# End Source File
# Begin Source File

SOURCE=..\player.h
# End Source File
# Begin Source File

SOURCE=..\potmark.h
# End Source File
# Begin Source File

SOURCE=..\ProTrack.h
# End Source File
# Begin Source File

SOURCE=..\PsPool.h
# End Source File
# Begin Source File

SOURCE=..\reticle.h
# End Source File
# Begin Source File

SOURCE=..\RoboBuddy.h
# End Source File
# Begin Source File

SOURCE=..\save.h
# End Source File
# Begin Source File

SOURCE=..\skybox.h
# End Source File
# Begin Source File

SOURCE=..\smoketrail.h
# End Source File
# Begin Source File

SOURCE=..\SpaceDock.h
# End Source File
# Begin Source File

SOURCE=..\tether.h
# End Source File
# Begin Source File

SOURCE=..\tracer.h
# End Source File
# Begin Source File

SOURCE=..\TripWire.h
# End Source File
# Begin Source File

SOURCE=..\VtxPool.h
# End Source File
# Begin Source File

SOURCE=..\Workable.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\ma_win.ico
# End Source File
# Begin Source File

SOURCE=.\ma_win.rc
# End Source File
# Begin Source File

SOURCE=.\res\ma_win.rc2
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ijl15l.lib
# End Source File
# End Target
# End Project

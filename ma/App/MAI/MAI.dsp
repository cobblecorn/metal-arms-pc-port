# Microsoft Developer Studio Project File - Name="MAI" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=MAI - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MAI.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MAI.mak" CFG="MAI - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MAI - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "MAI - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/Src/App/Mai", EABAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MAI - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "ai" /I "../../lib/mgc/Source\MgcCore" /I "../../lib/mgc/Source\MgcDistance" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "AIGRAPH_EDITOR_ENABLED" /D "_AFXDLL" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 fang2win_r.lib dinput8.lib dxguid.lib d3d8.lib d3dx8.lib winmm.lib dsound.lib ws2_32.lib SmallAMXwin_r.lib MgcCore_R.lib MgcDistance_R.lib msacm32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "ai" /I "../../lib/mgc/Source\MgcCore" /I "../../lib/mgc/Source\MgcDistance" /D "_DEBUG" /D "_AFXDLL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "AIGRAPH_EDITOR_ENABLED" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 fang2win_d.lib dinput8.lib dxguid.lib d3d8.lib d3dx8.lib winmm.lib dsound.lib ws2_32.lib SmallAMXwin_d.lib MgcCore_D.lib MgcDistance_D.lib msacm32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "MAI - Win32 Release"
# Name "MAI - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Ai"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Ai\AIGraph.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ai\AiGraphSearcher.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ai\AIPath.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ai\AIUtil.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\DlgStringInput.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\EdgePropsDlg.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FileInfo.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\K9.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\K9Dlg.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Leaf.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MaiAddTool.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MaiAddTool3D.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MaiBaseTool.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MaiEditGraph.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MaiJoinTool.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MaiMain.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MaiPoiTool.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MaiQueryDetails.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MaiSelectTool.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MaiTestGraphTool.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MaiToolBar.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MaiUndo.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MasterFileCompile.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PickAsset.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pickdir.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PoiPropsDlg.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\QueryDetailsDlg.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenGrab.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\screenshot.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Settings.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\VertPropsDlg.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\VidMode.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WinPrintf.cpp

!IF  "$(CFG)" == "MAI - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "MAI - Win32 Debug"

# ADD CPP /Yu"stdafx.h"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "AiH"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Ai\AIGraph.h
# End Source File
# Begin Source File

SOURCE=.\Ai\AIGraphSearcher.h
# End Source File
# Begin Source File

SOURCE=.\Ai\AIPath.h
# End Source File
# Begin Source File

SOURCE=.\Ai\AIPath.inl
# End Source File
# Begin Source File

SOURCE=.\Ai\AIUtil.h
# End Source File
# Begin Source File

SOURCE=.\Ai\apenew.h
# End Source File
# Begin Source File

SOURCE=.\Ai\ftl.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\CSharedStruct.h
# End Source File
# Begin Source File

SOURCE=.\DlgStringInput.h
# End Source File
# Begin Source File

SOURCE=.\EdgePropsDlg.h
# End Source File
# Begin Source File

SOURCE=.\FileInfo.h
# End Source File
# Begin Source File

SOURCE=.\ijl.h
# End Source File
# Begin Source File

SOURCE=.\InterProcessData.h
# End Source File
# Begin Source File

SOURCE=.\K9.h
# End Source File
# Begin Source File

SOURCE=.\K9Dlg.h
# End Source File
# Begin Source File

SOURCE=.\Leaf.h
# End Source File
# Begin Source File

SOURCE=.\MaiAddTool.h
# End Source File
# Begin Source File

SOURCE=.\MaiAddTool3D.h
# End Source File
# Begin Source File

SOURCE=.\MaiBaseTool.h
# End Source File
# Begin Source File

SOURCE=.\MaiEditGraph.h
# End Source File
# Begin Source File

SOURCE=.\MaiJoinTool.h
# End Source File
# Begin Source File

SOURCE=.\Maimain.h
# End Source File
# Begin Source File

SOURCE=.\MaiPoiTool.h
# End Source File
# Begin Source File

SOURCE=.\MaiQueryDetails.h
# End Source File
# Begin Source File

SOURCE=.\MaiSelectTool.h
# End Source File
# Begin Source File

SOURCE=.\MaiTestGraphTool.h
# End Source File
# Begin Source File

SOURCE=.\MAIToolBar.h
# End Source File
# Begin Source File

SOURCE=.\MaiUndo.h
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

SOURCE=.\PoiPropsDlg.h
# End Source File
# Begin Source File

SOURCE=.\QueryDetailsDlg.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
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

SOURCE=.\VertPropsDlg.h
# End Source File
# Begin Source File

SOURCE=.\VidMode.h
# End Source File
# Begin Source File

SOURCE=.\WinPrintf.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=.\res\graphtoo.bmp
# End Source File
# Begin Source File

SOURCE=.\res\K9.ico
# End Source File
# Begin Source File

SOURCE=.\K9.rc
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ijl15l.lib
# End Source File
# End Target
# End Project

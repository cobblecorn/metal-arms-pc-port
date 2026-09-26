# Microsoft Developer Studio Project File - Name="Spit" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Spit - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Spit.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Spit.mak" CFG="Spit - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Spit - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Spit - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/Src/App/Spit", RJFAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Spit - Win32 Release"

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
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "_FANGDEF_PLATFORM_WIN" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 fang2win_r.lib dinput8.lib dxguid.lib d3d8.lib d3dx8.lib winmm.lib dsound.lib SmallAMXwin_r.lib msacm32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "Spit - Win32 Debug"

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
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "_FANGDEF_PLATFORM_WIN" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 fang2win_d.lib dinput8.lib dxguid.lib d3d8.lib d3dx8.lib winmm.lib dsound.lib SmallAMXwin_d.lib msacm32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Spit - Win32 Release"
# Name "Spit - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\amsEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DefFileIO.cpp
# End Source File
# Begin Source File

SOURCE=.\EmissiveDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FileInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\ForcesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GlobalDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LifeSpanDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LightDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LodDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MasterFileCompile.cpp
# End Source File
# Begin Source File

SOURCE=.\NumParticlesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PickAsset.cpp
# End Source File
# Begin Source File

SOURCE=.\pickdir.cpp
# End Source File
# Begin Source File

SOURCE=.\PostionDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RollupCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ScaleDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Settings.cpp
# End Source File
# Begin Source File

SOURCE=.\Spit.cpp
# End Source File
# Begin Source File

SOURCE=.\SpitDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SpriteDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\VelocityDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\VidMode.cpp
# End Source File
# Begin Source File

SOURCE=.\wcSliderButton.cpp
# End Source File
# Begin Source File

SOURCE=.\wcSliderPopup.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\amsEdit.h
# End Source File
# Begin Source File

SOURCE=.\ColorDlg.h
# End Source File
# Begin Source File

SOURCE=.\DefFileIO.h
# End Source File
# Begin Source File

SOURCE=.\EmissiveDlg.h
# End Source File
# Begin Source File

SOURCE=.\FileInfo.h
# End Source File
# Begin Source File

SOURCE=.\ForcesDlg.h
# End Source File
# Begin Source File

SOURCE=.\GlobalDlg.h
# End Source File
# Begin Source File

SOURCE=.\InterProcessData.h
# End Source File
# Begin Source File

SOURCE=.\LifeSpanDlg.h
# End Source File
# Begin Source File

SOURCE=.\LightDlg.h
# End Source File
# Begin Source File

SOURCE=.\LodDlg.h
# End Source File
# Begin Source File

SOURCE=.\MasterFileCompile.h
# End Source File
# Begin Source File

SOURCE=.\NumParticlesDlg.h
# End Source File
# Begin Source File

SOURCE=.\PickAsset.h
# End Source File
# Begin Source File

SOURCE=.\pickdir.h
# End Source File
# Begin Source File

SOURCE=.\PostionDlg.h
# End Source File
# Begin Source File

SOURCE=.\RollupCtrl.h
# End Source File
# Begin Source File

SOURCE=.\ScaleDlg.h
# End Source File
# Begin Source File

SOURCE=.\Settings.h
# End Source File
# Begin Source File

SOURCE=.\Spit.h
# End Source File
# Begin Source File

SOURCE=.\SpitDlg.h
# End Source File
# Begin Source File

SOURCE=.\SpriteDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\VelocityDlg.h
# End Source File
# Begin Source File

SOURCE=.\VidMode.h
# End Source File
# Begin Source File

SOURCE=.\wcSliderButton.h
# End Source File
# Begin Source File

SOURCE=.\wcSliderPopup.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\res\Spit.ico
# End Source File
# Begin Source File

SOURCE=.\Spit.rc
# End Source File
# Begin Source File

SOURCE=.\res\Spit.rc2
# End Source File
# End Group
# End Target
# End Project

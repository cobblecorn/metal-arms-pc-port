# Microsoft Developer Studio Project File - Name="AIGraphConvert" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=AIGraphConvert - Win32 GCDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AIGraphConvert.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AIGraphConvert.mak" CFG="AIGraphConvert - Win32 GCDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AIGraphConvert - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "AIGraphConvert - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "AIGraphConvert - Win32 GCRelease" (based on "Win32 (x86) Static Library")
!MESSAGE "AIGraphConvert - Win32 GCDebug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/Src/Lib/AIGraphConvert", MLEAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AIGraphConvert - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "AIGraphConvert___Win32_Release"
# PROP BASE Intermediate_Dir "AIGraphConvert___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D AIGRAPH_EDITOR_ENABLED=2 /D DONT_USE_APE_NEW=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\AIGraphConvert_R.lib"

!ELSEIF  "$(CFG)" == "AIGraphConvert - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "AIGraphConvert___Win32_Debug"
# PROP BASE Intermediate_Dir "AIGraphConvert___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D AIGRAPH_EDITOR_ENABLED=2 /D DONT_USE_APE_NEW=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\AIGraphConvert_D.lib"

!ELSEIF  "$(CFG)" == "AIGraphConvert - Win32 GCRelease"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "AIGraphConvert___Win32_GCRelease"
# PROP BASE Intermediate_Dir "AIGraphConvert___Win32_GCRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "GCRelease"
# PROP Intermediate_Dir "GCRelease"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D AIGRAPH_EDITOR_ENABLED=2 /D DONT_USE_APE_NEW=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D AIGRAPH_EDITOR_ENABLED=2 /D DONT_USE_APE_NEW=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "_FANGDEF_WINGC" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\lib\AIGraphConvert_R.lib"
# ADD LIB32 /nologo /out:"..\..\..\lib\AIGraphConvert_gcr.lib"

!ELSEIF  "$(CFG)" == "AIGraphConvert - Win32 GCDebug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "AIGraphConvert___Win32_GCDebug"
# PROP BASE Intermediate_Dir "AIGraphConvert___Win32_GCDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "GCDebug"
# PROP Intermediate_Dir "GCDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D AIGRAPH_EDITOR_ENABLED=2 /D DONT_USE_APE_NEW=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D AIGRAPH_EDITOR_ENABLED=2 /D DONT_USE_APE_NEW=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "_FANGDEF_WINGC" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\lib\AIGraphConvert_D.lib"
# ADD LIB32 /nologo /out:"..\..\..\lib\AIGraphConvert_gcd.lib"

!ENDIF 

# Begin Target

# Name "AIGraphConvert - Win32 Release"
# Name "AIGraphConvert - Win32 Debug"
# Name "AIGraphConvert - Win32 GCRelease"
# Name "AIGraphConvert - Win32 GCDebug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "AI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Ai\AIGraph.cpp
# End Source File
# Begin Source File

SOURCE=.\Ai\AIUtil.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\AIGraphConvert.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Ai\AIGraph.h
# End Source File
# Begin Source File

SOURCE=.\AIGraphConvert.h
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
# End Target
# End Project

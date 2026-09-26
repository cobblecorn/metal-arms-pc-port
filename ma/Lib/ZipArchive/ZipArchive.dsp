# Microsoft Developer Studio Project File - Name="ZipArchive" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ZipArchive - Win32 GCDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ZipArchive.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ZipArchive.mak" CFG="ZipArchive - Win32 GCDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ZipArchive - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ZipArchive - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ZipArchive - Win32 GCRelease" (based on "Win32 (x86) Static Library")
!MESSAGE "ZipArchive - Win32 GCDebug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/Src/Lib/ZipArchive", NNDAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ZipArchive - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "ZLIB_DLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x415 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x415 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\ZipArchive_r.lib"

!ELSEIF  "$(CFG)" == "ZipArchive - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W4 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "ZLIB_DLL" /Fr /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x415 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x415 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\ZipArchive_d.lib"

!ELSEIF  "$(CFG)" == "ZipArchive - Win32 GCRelease"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ZipArchive___Win32_GCRelease"
# PROP BASE Intermediate_Dir "ZipArchive___Win32_GCRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "GCRelease"
# PROP Intermediate_Dir "GCRelease"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "ZLIB_DLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "ZLIB_DLL" /D "_AFXDLL" /D "_FANGDEF_WINGC" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x415 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x415 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\lib\ZipArchive_r.lib"
# ADD LIB32 /nologo /out:"..\..\..\lib\ZipArchive_gcr.lib"

!ELSEIF  "$(CFG)" == "ZipArchive - Win32 GCDebug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ZipArchive___Win32_GCDebug"
# PROP BASE Intermediate_Dir "ZipArchive___Win32_GCDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "GCDebug"
# PROP Intermediate_Dir "GCDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MDd /W4 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "ZLIB_DLL" /Fr /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W4 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "ZLIB_DLL" /D "_FANGDEF_WINGC" /Fr /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x415 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x415 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\lib\ZipArchive_d.lib"
# ADD LIB32 /nologo /out:"..\..\..\lib\ZipArchive_gcd.lib"

!ENDIF 

# Begin Target

# Name "ZipArchive - Win32 Release"
# Name "ZipArchive - Win32 Debug"
# Name "ZipArchive - Win32 GCRelease"
# Name "ZipArchive - Win32 GCDebug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\ZipArchive.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipAutoBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipBigFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipCentralDir.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipException.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipFileHeader.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipInternalInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipStorage.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\zconf.h
# End Source File
# Begin Source File

SOURCE=.\ZipArchive.h
# End Source File
# Begin Source File

SOURCE=.\ZipAutoBuffer.h
# End Source File
# Begin Source File

SOURCE=.\ZipBigFile.h
# End Source File
# Begin Source File

SOURCE=.\ZipCentralDir.h
# End Source File
# Begin Source File

SOURCE=.\ZipException.h
# End Source File
# Begin Source File

SOURCE=.\ZipFileHeader.h
# End Source File
# Begin Source File

SOURCE=.\ZipInternalInfo.h
# End Source File
# Begin Source File

SOURCE=.\ZipStorage.h
# End Source File
# Begin Source File

SOURCE=.\zlib.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Appnote.txt
# End Source File
# Begin Source File

SOURCE=.\zlib.lib
# End Source File
# End Target
# End Project
# Section ZipArchive : {427B9281-D5E8-11D3-B7C7-E75054E13747}
# 	2:12:AutoBuffer.h:AutoBuffer.h
# 	2:18:CLASS: CAutoBuffer:CAutoBuffer
# 	2:19:Application Include:Zip.h
# 	2:14:AutoBuffer.cpp:AutoBuffer.cpp
# End Section

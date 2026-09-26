# Microsoft Developer Studio Project File - Name="SmallAMX" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104
# TARGTYPE "Xbox Static Library" 0x0b04

CFG=SmallAMX - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SmallAMX.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SmallAMX.mak" CFG="SmallAMX - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SmallAMX - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "SmallAMX - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "SmallAMX - Xbox Release" (based on "Xbox Static Library")
!MESSAGE "SmallAMX - Xbox Debug" (based on "Xbox Static Library")
!MESSAGE "SmallAMX - Win32 Test" (based on "Win32 (x86) Static Library")
!MESSAGE "SmallAMX - Win32 Production" (based on "Win32 (x86) Static Library")
!MESSAGE "SmallAMX - Xbox Test" (based on "Xbox Static Library")
!MESSAGE "SmallAMX - Xbox Production" (based on "Xbox Static Library")
!MESSAGE "SmallAMX - Win32 GCDebug" (based on "Win32 (x86) Static Library")
!MESSAGE "SmallAMX - Win32 GCRelease" (based on "Win32 (x86) Static Library")
!MESSAGE "SmallAMX - Win32 Debug Unicode" (based on "Win32 (x86) Static Library")
!MESSAGE "SmallAMX - Win32 Release Unicode" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/Src/Lib/SmallAMX", XRCAAAAA"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "SmallAMX - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /D "_FANGDEF_PLATFORM_WIN" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\smallamxwin_r.lib"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /D "_FANGDEF_PLATFORM_WIN" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\smallamxwin_d.lib"

!ELSEIF  "$(CFG)" == "SmallAMX - Xbox Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_xb"
# PROP BASE Intermediate_Dir "Release_xb"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_xb"
# PROP Intermediate_Dir "Release_xb"
# PROP Target_Dir ""
RSC=rc.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /Ob2 /D "_FANGDEF_PLATFORM_XB" /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\smallamxxb_r.lib"

!ELSEIF  "$(CFG)" == "SmallAMX - Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug_xb"
# PROP BASE Intermediate_Dir "Debug_xb"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_xb"
# PROP Intermediate_Dir "Debug_xb"
# PROP Target_Dir ""
RSC=rc.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_FANGDEF_PLATFORM_XB" /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\smallamxxb_d.lib"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Test"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SmallAMX___Win32_Test"
# PROP BASE Intermediate_Dir "SmallAMX___Win32_Test"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Test"
# PROP Intermediate_Dir "Test"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /G6 /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /D "_FANGDEF_PLATFORM_WIN" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\lib\smallamxwin_r.lib"
# ADD LIB32 /nologo /out:"..\..\..\lib\smallamxwin_t.lib"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Production"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SmallAMX___Win32_Production"
# PROP BASE Intermediate_Dir "SmallAMX___Win32_Production"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Production"
# PROP Intermediate_Dir "Production"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /G6 /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /D "_FANGDEF_PLATFORM_WIN" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\lib\smallamxwin_r.lib"
# ADD LIB32 /nologo /out:"..\..\..\lib\smallamxwin_p.lib"

!ELSEIF  "$(CFG)" == "SmallAMX - Xbox Test"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SmallAMX___Xbox_Test"
# PROP BASE Intermediate_Dir "SmallAMX___Xbox_Test"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Test_xb"
# PROP Intermediate_Dir "Test_xb"
# PROP Target_Dir ""
RSC=rc.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /GX /O2 /Ob2 /D "_FANGDEF_PLATFORM_XB" /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\lib\smallamxxb_r.lib"
# ADD LIB32 /nologo /out:"..\..\..\lib\smallamxxb_t.lib"

!ELSEIF  "$(CFG)" == "SmallAMX - Xbox Production"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SmallAMX___Xbox_Production"
# PROP BASE Intermediate_Dir "SmallAMX___Xbox_Production"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Production_xb"
# PROP Intermediate_Dir "Production_xb"
# PROP Target_Dir ""
RSC=rc.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /GX /O2 /Ob2 /D "_FANGDEF_PLATFORM_XB" /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\lib\smallamxxb_r.lib"
# ADD LIB32 /nologo /out:"..\..\..\lib\smallamxxb_p.lib"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 GCDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SmallAMX___Win32_GCDebug"
# PROP BASE Intermediate_Dir "SmallAMX___Win32_GCDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "GCDebug"
# PROP Intermediate_Dir "GCDebug"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /D "_FANGDEF_PLATFORM_WIN" /D "_FANGDEF_WINGC" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\lib\smallamxwin_d.lib"
# ADD LIB32 /nologo /out:"..\..\..\lib\smallamxwin_gcd.lib"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 GCRelease"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SmallAMX___Win32_GCRelease"
# PROP BASE Intermediate_Dir "SmallAMX___Win32_GCRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "GCRelease"
# PROP Intermediate_Dir "GCRelease"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /G6 /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /D "_FANGDEF_PLATFORM_WIN" /D "_FANGDEF_WINGC" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\lib\smallamxwin_r.lib"
# ADD LIB32 /nologo /out:"..\..\..\lib\smallamxwin_gcr.lib"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SmallAMX___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "SmallAMX___Win32_Debug_Unicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "SmallAMX___Win32_Debug_Unicode"
# PROP Intermediate_Dir "SmallAMX___Win32_Debug_Unicode"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /D "_FANGDEF_PLATFORM_WIN" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /D "_FANGDEF_PLATFORM_WIN" /D "WIN32" /D "_DEBUG" /D "_UNICODE" /D "UNICODE" /D "_LIB" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\lib\smallamxwin_d.lib"
# ADD LIB32 /nologo /out:"..\..\..\lib\smallamxwin_d.lib"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SmallAMX___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "SmallAMX___Win32_Release_Unicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "SmallAMX___Win32_Release_Unicode"
# PROP Intermediate_Dir "SmallAMX___Win32_Release_Unicode"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /G6 /MD /W3 /GX /O2 /D "_FANGDEF_PLATFORM_WIN" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /D "_FANGDEF_PLATFORM_WIN" /D "WIN32" /D "NDEBUG" /D "_UNICODE" /D "UNICODE" /D "_LIB" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\lib\smallamxwin_r.lib"
# ADD LIB32 /nologo /out:"..\..\..\lib\smallamxwin_r.lib"

!ENDIF 

# Begin Target

# Name "SmallAMX - Win32 Release"
# Name "SmallAMX - Win32 Debug"
# Name "SmallAMX - Xbox Release"
# Name "SmallAMX - Xbox Debug"
# Name "SmallAMX - Win32 Test"
# Name "SmallAMX - Win32 Production"
# Name "SmallAMX - Xbox Test"
# Name "SmallAMX - Xbox Production"
# Name "SmallAMX - Win32 GCDebug"
# Name "SmallAMX - Win32 GCRelease"
# Name "SmallAMX - Win32 Debug Unicode"
# Name "SmallAMX - Win32 Release Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\amx.c

!IF  "$(CFG)" == "SmallAMX - Win32 Release"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Debug"

!ELSEIF  "$(CFG)" == "SmallAMX - Xbox Release"

!ELSEIF  "$(CFG)" == "SmallAMX - Xbox Debug"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Test"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Production"

!ELSEIF  "$(CFG)" == "SmallAMX - Xbox Test"

!ELSEIF  "$(CFG)" == "SmallAMX - Xbox Production"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 GCDebug"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 GCRelease"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Debug Unicode"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Release Unicode"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\amxcons.c

!IF  "$(CFG)" == "SmallAMX - Win32 Release"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Debug"

!ELSEIF  "$(CFG)" == "SmallAMX - Xbox Release"

!ELSEIF  "$(CFG)" == "SmallAMX - Xbox Debug"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Test"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Production"

!ELSEIF  "$(CFG)" == "SmallAMX - Xbox Test"

!ELSEIF  "$(CFG)" == "SmallAMX - Xbox Production"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 GCDebug"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 GCRelease"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Debug Unicode"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Release Unicode"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\amxcore.c

!IF  "$(CFG)" == "SmallAMX - Win32 Release"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Debug"

!ELSEIF  "$(CFG)" == "SmallAMX - Xbox Release"

!ELSEIF  "$(CFG)" == "SmallAMX - Xbox Debug"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Test"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Production"

!ELSEIF  "$(CFG)" == "SmallAMX - Xbox Test"

!ELSEIF  "$(CFG)" == "SmallAMX - Xbox Production"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 GCDebug"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 GCRelease"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Debug Unicode"

!ELSEIF  "$(CFG)" == "SmallAMX - Win32 Release Unicode"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\amx.h
# End Source File
# Begin Source File

SOURCE=.\osdefs.h
# End Source File
# End Group
# End Target
# End Project

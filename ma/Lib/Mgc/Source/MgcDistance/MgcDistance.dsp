# Microsoft Developer Studio Project File - Name="MgcDistance" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=MgcDistance - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MgcDistance.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MgcDistance.mak" CFG="MgcDistance - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MgcDistance - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MgcDistance - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/Src/Lib/Mgc/Source/MgcDistance", MIEAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MgcDistance - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /Zi /O2 /Ob2 /I "..\MgcCore" /I "..\MgcNumerics" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "STRICT" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\MgcDistance_R.lib"

!ELSEIF  "$(CFG)" == "MgcDistance - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /W3 /Gm /GX /ZI /Od /I "..\MgcCore" /I "..\MgcNumerics" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "STRICT" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\MgcDistance_D.lib"

!ENDIF 

# Begin Target

# Name "MgcDistance - Win32 Release"
# Name "MgcDistance - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\MgcDistLin3Lin3.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\MgcDistCir3Cir3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistLin3Box3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistLin3Cir3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistLin3Lin3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistLin3Pgm3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistLin3Rct3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistLin3Tri3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistPgm3Pgm3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistRct3Rct3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistTri3Pgm3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistTri3Rct3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistTri3Tri3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistVec2Elp2.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistVec2Qdr2.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistVec3Box3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistVec3Cir3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistVec3Elp3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistVec3Frustum.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistVec3Lin3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistVec3Pgm3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistVec3Pln3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistVec3Qdr3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistVec3Rct3.h
# End Source File
# Begin Source File

SOURCE=.\MgcDistVec3Tri3.h
# End Source File
# End Group
# End Target
# End Project

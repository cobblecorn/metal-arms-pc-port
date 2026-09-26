# Microsoft Developer Studio Project File - Name="MgcCore" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=MgcCore - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MgcCore.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MgcCore.mak" CFG="MgcCore - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MgcCore - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MgcCore - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/Src/Lib/Mgc/Source/MgcCore", QIEAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MgcCore - Win32 Release"

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
# ADD CPP /nologo /G5 /W3 /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "STRICT" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\MgcCore_R.lib"

!ELSEIF  "$(CFG)" == "MgcCore - Win32 Debug"

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
# ADD CPP /nologo /G5 /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "STRICT" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\MgcCore_D.lib"

!ENDIF 

# Begin Target

# Name "MgcCore - Win32 Release"
# Name "MgcCore - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\MgcMath.cpp
# End Source File
# Begin Source File

SOURCE=.\MgcVector3.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\MgcBox2.h
# End Source File
# Begin Source File

SOURCE=.\MgcBox3.h
# End Source File
# Begin Source File

SOURCE=.\MgcCapsule.h
# End Source File
# Begin Source File

SOURCE=.\MgcCircle2.h
# End Source File
# Begin Source File

SOURCE=.\MgcCircle3.h
# End Source File
# Begin Source File

SOURCE=.\MgcColor.h
# End Source File
# Begin Source File

SOURCE=.\MgcCylinder.h
# End Source File
# Begin Source File

SOURCE=.\MgcDisk2.h
# End Source File
# Begin Source File

SOURCE=.\MgcDisk3.h
# End Source File
# Begin Source File

SOURCE=.\MgcEllipse.h
# End Source File
# Begin Source File

SOURCE=.\MgcEllipsoid.h
# End Source File
# Begin Source File

SOURCE=.\MgcExtraSpin.h
# End Source File
# Begin Source File

SOURCE=.\MgcFrustum.h
# End Source File
# Begin Source File

SOURCE=.\MgcLine3.h
# End Source File
# Begin Source File

SOURCE=.\MgcLozenge.h
# End Source File
# Begin Source File

SOURCE=.\MgcMath.h
# End Source File
# Begin Source File

SOURCE=.\MgcMatrix2.h
# End Source File
# Begin Source File

SOURCE=.\MgcMatrix3.h
# End Source File
# Begin Source File

SOURCE=.\MgcParallelogram3.h
# End Source File
# Begin Source File

SOURCE=.\MgcPlane.h
# End Source File
# Begin Source File

SOURCE=.\MgcPolynomial.h
# End Source File
# Begin Source File

SOURCE=.\MgcQuaternion.h
# End Source File
# Begin Source File

SOURCE=.\MgcRay3.h
# End Source File
# Begin Source File

SOURCE=.\MgcRectangle3.h
# End Source File
# Begin Source File

SOURCE=.\MgcRTLib.h
# End Source File
# Begin Source File

SOURCE=.\MgcSegment3.h
# End Source File
# Begin Source File

SOURCE=.\MgcSphere.h
# End Source File
# Begin Source File

SOURCE=.\MgcString.h
# End Source File
# Begin Source File

SOURCE=.\MgcTArray.h
# End Source File
# Begin Source File

SOURCE=.\MgcTClassArray.h
# End Source File
# Begin Source File

SOURCE=.\MgcTClassHeap.h
# End Source File
# Begin Source File

SOURCE=.\MgcTClassList.h
# End Source File
# Begin Source File

SOURCE=.\MgcTClassMap.h
# End Source File
# Begin Source File

SOURCE=.\MgcTClassStorage.h
# End Source File
# Begin Source File

SOURCE=.\MgcTHeap.h
# End Source File
# Begin Source File

SOURCE=.\MgcTList.h
# End Source File
# Begin Source File

SOURCE=.\MgcTMap.h
# End Source File
# Begin Source File

SOURCE=.\MgcTriangle3.h
# End Source File
# Begin Source File

SOURCE=.\MgcTStorage.h
# End Source File
# Begin Source File

SOURCE=.\MgcVector2.h
# End Source File
# Begin Source File

SOURCE=.\MgcVector3.h
# End Source File
# Begin Source File

SOURCE=.\MgcVetTable.h
# End Source File
# Begin Source File

SOURCE=.\MgcXFormInterp.h
# End Source File
# End Group
# Begin Group "Inline Files"

# PROP Default_Filter "inl"
# Begin Source File

SOURCE=.\MgcBox2.inl
# End Source File
# Begin Source File

SOURCE=.\MgcBox3.inl
# End Source File
# Begin Source File

SOURCE=.\MgcCapsule.inl
# End Source File
# Begin Source File

SOURCE=.\MgcCircle2.inl
# End Source File
# Begin Source File

SOURCE=.\MgcCircle3.inl
# End Source File
# Begin Source File

SOURCE=.\MgcCylinder.inl
# End Source File
# Begin Source File

SOURCE=.\MgcDisk2.inl
# End Source File
# Begin Source File

SOURCE=.\MgcDisk3.inl
# End Source File
# Begin Source File

SOURCE=.\MgcEllipse.inl
# End Source File
# Begin Source File

SOURCE=.\MgcEllipsoid.inl
# End Source File
# Begin Source File

SOURCE=.\MgcExtraSpin.inl
# End Source File
# Begin Source File

SOURCE=.\MgcFrustum.inl
# End Source File
# Begin Source File

SOURCE=.\MgcLine3.inl
# End Source File
# Begin Source File

SOURCE=.\MgcLozenge.inl
# End Source File
# Begin Source File

SOURCE=.\MgcParallelogram3.inl
# End Source File
# Begin Source File

SOURCE=.\MgcPlane.inl
# End Source File
# Begin Source File

SOURCE=.\MgcRay3.inl
# End Source File
# Begin Source File

SOURCE=.\MgcRectangle3.inl
# End Source File
# Begin Source File

SOURCE=.\MgcSegment3.inl
# End Source File
# Begin Source File

SOURCE=.\MgcSphere.inl
# End Source File
# Begin Source File

SOURCE=.\MgcTArray.inl
# End Source File
# Begin Source File

SOURCE=.\MgcTClassArray.inl
# End Source File
# Begin Source File

SOURCE=.\MgcTClassHeap.inl
# End Source File
# Begin Source File

SOURCE=.\MgcTClassList.inl
# End Source File
# Begin Source File

SOURCE=.\MgcTClassMap.inl
# End Source File
# Begin Source File

SOURCE=.\MgcTClassStorage.inl
# End Source File
# Begin Source File

SOURCE=.\MgcTHeap.inl
# End Source File
# Begin Source File

SOURCE=.\MgcTList.inl
# End Source File
# Begin Source File

SOURCE=.\MgcTMap.inl
# End Source File
# Begin Source File

SOURCE=.\MgcTriangle3.inl
# End Source File
# Begin Source File

SOURCE=.\MgcTStorage.inl
# End Source File
# Begin Source File

SOURCE=.\MgcVetTable.inl
# End Source File
# End Group
# End Target
# End Project

# Microsoft Developer Studio Project File - Name="fang2win" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=fang2win - Win32 GCDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fang2win.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fang2win.mak" CFG="fang2win - Win32 GCDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fang2win - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "fang2win - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "fang2win - Win32 Test" (based on "Win32 (x86) Static Library")
!MESSAGE "fang2win - Win32 Production" (based on "Win32 (x86) Static Library")
!MESSAGE "fang2win - Win32 Debug Unicode" (based on "Win32 (x86) Static Library")
!MESSAGE "fang2win - Win32 Release Unicode" (based on "Win32 (x86) Static Library")
!MESSAGE "fang2win - Win32 GCDebug" (based on "Win32 (x86) Static Library")
!MESSAGE "fang2win - Win32 GCRelease" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/Src/Lib/fang2/dx/win", WHAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fang2win - Win32 Release"

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
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /Gi /GX /O2 /Ob2 /I "..\SmallAmx" /I "..\..\..\SmallAmx" /I ".." /I "..\.." /D "NDEBUG" /D "_AFXDLL" /D "_FANGDEF_PLATFORM_WIN" /D "_FANGDEF_RELEASE_BUILD" /D "WIN32" /D "_WINDOWS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\fang2win_r.lib"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

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
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "..\SmallAmx" /I "..\..\..\SmallAmx" /I ".." /I "..\.." /D "_DEBUG" /D "_AFXDLL" /D "_FANGDEF_PLATFORM_WIN" /D "WIN32" /D "_WINDOWS" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\fang2win_d.lib"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Test"
# PROP BASE Intermediate_Dir "Test"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Test"
# PROP Intermediate_Dir "Test"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /Gi /GX /O2 /Ob2 /I "..\SmallAmx" /I "..\..\..\SmallAmx" /I ".." /I "..\.." /D "NDEBUG" /D "_AFXDLL" /D "_FANGDEF_PLATFORM_WIN" /D "_FANGDEF_TEST_BUILD" /D "WIN32" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\..\..\lib\fang2win_r.lib"
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\fang2win_t.lib"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Production"
# PROP BASE Intermediate_Dir "Production"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Production"
# PROP Intermediate_Dir "Production"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /Gi /GX /O2 /Ob2 /I "..\SmallAmx" /I "..\..\..\SmallAmx" /I ".." /I "..\.." /D "NDEBUG" /D "_AFXDLL" /D "_FANGDEF_PLATFORM_WIN" /D "_FANGDEF_PRODUCTION_BUILD" /D "WIN32" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\..\..\lib\fang2win_r.lib"
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\fang2win_p.lib"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "fang2win___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "fang2win___Win32_Debug_Unicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "fang2win___Win32_Debug_Unicode"
# PROP Intermediate_Dir "fang2win___Win32_Debug_Unicode"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "..\SmallAmx" /I "..\..\..\SmallAmx" /I ".." /I "..\.." /D "_DEBUG" /D "_AFXDLL" /D "_FANGDEF_PLATFORM_WIN" /D "WIN32" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "..\..\..\lib\SmallAMX" /D "_DEBUG" /D "_AFXDLL" /D "_FANGDEF_PLATFORM_WIN" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\..\..\lib\fang2win_d.lib"
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\fang2win_d.lib"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fang2win___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "fang2win___Win32_Release_Unicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "fang2win___Win32_Release_Unicode"
# PROP Intermediate_Dir "fang2win___Win32_Release_Unicode"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /Gi /GX /O2 /Ob2 /I "..\SmallAmx" /I "..\..\..\SmallAmx" /I ".." /I "..\.." /D "NDEBUG" /D "_AFXDLL" /D "_FANGDEF_PLATFORM_WIN" /D "_FANGDEF_RELEASE_BUILD" /D "WIN32" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /Gi /GX /O2 /Ob2 /I "..\..\..\lib\SmallAMX" /D "NDEBUG" /D "_AFXDLL" /D "_FANGDEF_PLATFORM_WIN" /D "_FANGDEF_RELEASE_BUILD" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\..\..\lib\fang2win_r.lib"
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\fang2win_r.lib"

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "fang2win___Win32_GCDebug"
# PROP BASE Intermediate_Dir "fang2win___Win32_GCDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "GCDebug"
# PROP Intermediate_Dir "GCDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "..\SmallAmx" /I "..\..\..\SmallAmx" /I ".." /I "..\.." /D "_DEBUG" /D "_AFXDLL" /D "_FANGDEF_PLATFORM_WIN" /D "WIN32" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "..\SmallAmx" /I "..\..\..\SmallAmx" /I ".." /I "..\.." /D "_DEBUG" /D "_AFXDLL" /D "_FANGDEF_PLATFORM_WIN" /D "_FANGDEF_WINGC" /D "WIN32" /D "_WINDOWS" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\..\..\lib\fang2win_d.lib"
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\fang2win_gcd.lib"

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fang2win___Win32_GCRelease"
# PROP BASE Intermediate_Dir "fang2win___Win32_GCRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "GCRelease"
# PROP Intermediate_Dir "GCRelease"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /Gi /GX /O2 /Ob2 /I "..\SmallAmx" /I "..\..\..\SmallAmx" /I ".." /I "..\.." /D "NDEBUG" /D "_AFXDLL" /D "_FANGDEF_PLATFORM_WIN" /D "_FANGDEF_RELEASE_BUILD" /D "WIN32" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /Gi /GX /O2 /Ob2 /I "..\SmallAmx" /I "..\..\..\SmallAmx" /I ".." /I "..\.." /D "NDEBUG" /D "_AFXDLL" /D "_FANGDEF_PLATFORM_WIN" /D "_FANGDEF_RELEASE_BUILD" /D "_FANGDEF_WINGC" /D "WIN32" /D "_WINDOWS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\..\..\lib\fang2win_r.lib"
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\fang2win_gcr.lib"

!ENDIF 

# Begin Target

# Name "fang2win - Win32 Release"
# Name "fang2win - Win32 Debug"
# Name "fang2win - Win32 Test"
# Name "fang2win - Win32 Production"
# Name "fang2win - Win32 Debug Unicode"
# Name "fang2win - Win32 Release Unicode"
# Name "fang2win - Win32 GCDebug"
# Name "fang2win - Win32 GCRelease"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;vsh;psh;asm"
# Begin Group "FQuatObj Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\FQOTang1.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FQuatComp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FQuatConst.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FQuatLookAt1.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FQuatObj.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FQuatSLERP.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FQuatTang2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FQuatTang3.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FQuatTwirl.cpp
# End Source File
# End Group
# Begin Group "FVec3Obj Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\FMulPath1.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FPointPath1.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FPointPath2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FSumPath1.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FV3OCircleXZ.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FV3OConst.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FV3OLine1.cpp
# End Source File
# End Group
# Begin Group "FScalarObj Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\FScalarBlend.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FScalarConst.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FScalarSinus.cpp
# End Source File
# End Group
# Begin Group "Shaders"

# PROP Default_Filter "nvv;nvp"
# Begin Source File

SOURCE=..\fdx8Color.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Color.nvp
InputName=fdx8Color

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Color.nvp
InputName=fdx8Color

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Color.nvp
InputName=fdx8Color

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Color.nvp
InputName=fdx8Color

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Color.nvp
InputName=fdx8Color

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Color.nvp
InputName=fdx8Color

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Color.nvp
InputName=fdx8Color

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Color.nvp
InputName=fdx8Color

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8ColorAlphaMask.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorAlphaMask.nvp
InputName=fdx8ColorAlphaMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorAlphaMask.nvp
InputName=fdx8ColorAlphaMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorAlphaMask.nvp
InputName=fdx8ColorAlphaMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorAlphaMask.nvp
InputName=fdx8ColorAlphaMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorAlphaMask.nvp
InputName=fdx8ColorAlphaMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorAlphaMask.nvp
InputName=fdx8ColorAlphaMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorAlphaMask.nvp
InputName=fdx8ColorAlphaMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorAlphaMask.nvp
InputName=fdx8ColorAlphaMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8ColorEMask.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorEMask.nvp
InputName=fdx8ColorEMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorEMask.nvp
InputName=fdx8ColorEMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorEMask.nvp
InputName=fdx8ColorEMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorEMask.nvp
InputName=fdx8ColorEMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorEMask.nvp
InputName=fdx8ColorEMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorEMask.nvp
InputName=fdx8ColorEMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorEMask.nvp
InputName=fdx8ColorEMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorEMask.nvp
InputName=fdx8ColorEMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8ColorMask.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorMask.nvp
InputName=fdx8ColorMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorMask.nvp
InputName=fdx8ColorMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorMask.nvp
InputName=fdx8ColorMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorMask.nvp
InputName=fdx8ColorMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorMask.nvp
InputName=fdx8ColorMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorMask.nvp
InputName=fdx8ColorMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorMask.nvp
InputName=fdx8ColorMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8ColorMask.nvp
InputName=fdx8ColorMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8Detail.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail.nvp
InputName=fdx8Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail.nvp
InputName=fdx8Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail.nvp
InputName=fdx8Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail.nvp
InputName=fdx8Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail.nvp
InputName=fdx8Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail.nvp
InputName=fdx8Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail.nvp
InputName=fdx8Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail.nvp
InputName=fdx8Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8Detail.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail.nvv
InputName=fdx8Detail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail.nvv
InputName=fdx8Detail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail.nvv
InputName=fdx8Detail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail.nvv
InputName=fdx8Detail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail.nvv
InputName=fdx8Detail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail.nvv
InputName=fdx8Detail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail.nvv
InputName=fdx8Detail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail.nvv
InputName=fdx8Detail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8Detail_2tc.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail_2tc.nvv
InputName=fdx8Detail_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail_2tc.nvv
InputName=fdx8Detail_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail_2tc.nvv
InputName=fdx8Detail_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail_2tc.nvv
InputName=fdx8Detail_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail_2tc.nvv
InputName=fdx8Detail_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail_2tc.nvv
InputName=fdx8Detail_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail_2tc.nvv
InputName=fdx8Detail_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Detail_2tc.nvv
InputName=fdx8Detail_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8Directional1_Specular.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Directional1_Specular.nvv
InputName=fdx8Directional1_Specular

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Directional1_Specular.nvv
InputName=fdx8Directional1_Specular

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Directional1_Specular.nvv
InputName=fdx8Directional1_Specular

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Directional1_Specular.nvv
InputName=fdx8Directional1_Specular

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Directional1_Specular.nvv
InputName=fdx8Directional1_Specular

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Directional1_Specular.nvv
InputName=fdx8Directional1_Specular

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Directional1_Specular.nvv
InputName=fdx8Directional1_Specular

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Directional1_Specular.nvv
InputName=fdx8Directional1_Specular

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8Intensity.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Intensity.nvp
InputName=fdx8Intensity

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Intensity.nvp
InputName=fdx8Intensity

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Intensity.nvp
InputName=fdx8Intensity

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Intensity.nvp
InputName=fdx8Intensity

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Intensity.nvp
InputName=fdx8Intensity

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Intensity.nvp
InputName=fdx8Intensity

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Intensity.nvp
InputName=fdx8Intensity

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Intensity.nvp
InputName=fdx8Intensity

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8oBase_Add_rbENV.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV.nvp
InputName=fdx8oBase_Add_rbENV

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV.nvp
InputName=fdx8oBase_Add_rbENV

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV.nvp
InputName=fdx8oBase_Add_rbENV

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV.nvp
InputName=fdx8oBase_Add_rbENV

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV.nvp
InputName=fdx8oBase_Add_rbENV

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV.nvp
InputName=fdx8oBase_Add_rbENV

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV.nvp
InputName=fdx8oBase_Add_rbENV

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV.nvp
InputName=fdx8oBase_Add_rbENV

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8oBase_Add_rbENV.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV.nvv
InputName=fdx8oBase_Add_rbENV

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8oBase_Add_rbENV_blend.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_blend.nvv
InputName=fdx8oBase_Add_rbENV_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_blend.nvv
InputName=fdx8oBase_Add_rbENV_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_blend.nvv
InputName=fdx8oBase_Add_rbENV_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_blend.nvv
InputName=fdx8oBase_Add_rbENV_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_blend.nvv
InputName=fdx8oBase_Add_rbENV_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_blend.nvv
InputName=fdx8oBase_Add_rbENV_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_blend.nvv
InputName=fdx8oBase_Add_rbENV_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_blend.nvv
InputName=fdx8oBase_Add_rbENV_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8oBase_Add_rbENV_Detail.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_Detail.nvp
InputName=fdx8oBase_Add_rbENV_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_Detail.nvp
InputName=fdx8oBase_Add_rbENV_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_Detail.nvp
InputName=fdx8oBase_Add_rbENV_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_Detail.nvp
InputName=fdx8oBase_Add_rbENV_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_Detail.nvp
InputName=fdx8oBase_Add_rbENV_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_Detail.nvp
InputName=fdx8oBase_Add_rbENV_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_Detail.nvp
InputName=fdx8oBase_Add_rbENV_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_Detail.nvp
InputName=fdx8oBase_Add_rbENV_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8oBase_Add_rbENV_Detail.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_Detail.nvv
InputName=fdx8oBase_Add_rbENV_Detail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_Detail.nvv
InputName=fdx8oBase_Add_rbENV_Detail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_Detail.nvv
InputName=fdx8oBase_Add_rbENV_Detail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_Detail.nvv
InputName=fdx8oBase_Add_rbENV_Detail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_Detail.nvv
InputName=fdx8oBase_Add_rbENV_Detail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_Detail.nvv
InputName=fdx8oBase_Add_rbENV_Detail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_Detail.nvv
InputName=fdx8oBase_Add_rbENV_Detail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Add_rbENV_Detail.nvv
InputName=fdx8oBase_Add_rbENV_Detail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8oBase_Lerp_pLayer.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_pLayer.nvp
InputName=fdx8oBase_Lerp_pLayer

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8oBase_Lerp_pLayer_Detail.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_pLayer_Detail.nvp
InputName=fdx8oBase_Lerp_pLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_pLayer_Detail.nvp
InputName=fdx8oBase_Lerp_pLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_pLayer_Detail.nvp
InputName=fdx8oBase_Lerp_pLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_pLayer_Detail.nvp
InputName=fdx8oBase_Lerp_pLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_pLayer_Detail.nvp
InputName=fdx8oBase_Lerp_pLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_pLayer_Detail.nvp
InputName=fdx8oBase_Lerp_pLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_pLayer_Detail.nvp
InputName=fdx8oBase_Lerp_pLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_pLayer_Detail.nvp
InputName=fdx8oBase_Lerp_pLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8oBase_Lerp_tLayer.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_tLayer.nvp
InputName=fdx8oBase_Lerp_tLayer

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8oBase_Lerp_tLayer_Detail.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_tLayer_Detail.nvp
InputName=fdx8oBase_Lerp_tLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_tLayer_Detail.nvp
InputName=fdx8oBase_Lerp_tLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_tLayer_Detail.nvp
InputName=fdx8oBase_Lerp_tLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_tLayer_Detail.nvp
InputName=fdx8oBase_Lerp_tLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_tLayer_Detail.nvp
InputName=fdx8oBase_Lerp_tLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_tLayer_Detail.nvp
InputName=fdx8oBase_Lerp_tLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_tLayer_Detail.nvp
InputName=fdx8oBase_Lerp_tLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_tLayer_Detail.nvp
InputName=fdx8oBase_Lerp_tLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8oBase_Lerp_vLayer.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_vLayer.nvp
InputName=fdx8oBase_Lerp_vLayer

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8oBase_Lerp_vLayer_Detail.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_vLayer_Detail.nvp
InputName=fdx8oBase_Lerp_vLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_vLayer_Detail.nvp
InputName=fdx8oBase_Lerp_vLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_vLayer_Detail.nvp
InputName=fdx8oBase_Lerp_vLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_vLayer_Detail.nvp
InputName=fdx8oBase_Lerp_vLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_vLayer_Detail.nvp
InputName=fdx8oBase_Lerp_vLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_vLayer_Detail.nvp
InputName=fdx8oBase_Lerp_vLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_vLayer_Detail.nvp
InputName=fdx8oBase_Lerp_vLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8oBase_Lerp_vLayer_Detail.nvp
InputName=fdx8oBase_Lerp_vLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PassThru.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru.nvp
InputName=fdx8PassThru

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru.nvp
InputName=fdx8PassThru

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru.nvp
InputName=fdx8PassThru

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru.nvp
InputName=fdx8PassThru

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru.nvp
InputName=fdx8PassThru

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru.nvp
InputName=fdx8PassThru

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru.nvp
InputName=fdx8PassThru

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru.nvp
InputName=fdx8PassThru

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PassThru_1tc.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_1tc.nvv
InputName=fdx8PassThru_1tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_1tc.nvv
InputName=fdx8PassThru_1tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_1tc.nvv
InputName=fdx8PassThru_1tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_1tc.nvv
InputName=fdx8PassThru_1tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_1tc.nvv
InputName=fdx8PassThru_1tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_1tc.nvv
InputName=fdx8PassThru_1tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_1tc.nvv
InputName=fdx8PassThru_1tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_1tc.nvv
InputName=fdx8PassThru_1tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PassThru_1tc_blend.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_1tc_blend.nvv
InputName=fdx8PassThru_1tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_1tc_blend.nvv
InputName=fdx8PassThru_1tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_1tc_blend.nvv
InputName=fdx8PassThru_1tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_1tc_blend.nvv
InputName=fdx8PassThru_1tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_1tc_blend.nvv
InputName=fdx8PassThru_1tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_1tc_blend.nvv
InputName=fdx8PassThru_1tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_1tc_blend.nvv
InputName=fdx8PassThru_1tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_1tc_blend.nvv
InputName=fdx8PassThru_1tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PassThru_2tc.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_2tc.nvv
InputName=fdx8PassThru_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_2tc.nvv
InputName=fdx8PassThru_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_2tc.nvv
InputName=fdx8PassThru_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_2tc.nvv
InputName=fdx8PassThru_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_2tc.nvv
InputName=fdx8PassThru_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_2tc.nvv
InputName=fdx8PassThru_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_2tc.nvv
InputName=fdx8PassThru_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_2tc.nvv
InputName=fdx8PassThru_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PassThru_2tc_blend.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_2tc_blend.nvv
InputName=fdx8PassThru_2tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_2tc_blend.nvv
InputName=fdx8PassThru_2tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_2tc_blend.nvv
InputName=fdx8PassThru_2tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_2tc_blend.nvv
InputName=fdx8PassThru_2tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_2tc_blend.nvv
InputName=fdx8PassThru_2tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_2tc_blend.nvv
InputName=fdx8PassThru_2tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_2tc_blend.nvv
InputName=fdx8PassThru_2tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PassThru_2tc_blend.nvv
InputName=fdx8PassThru_2tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PerPixelDirectional1_Specular_Bump.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelDirectional1_Specular_Bump.nvp
InputName=fdx8PerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelDirectional1_Specular_Bump.nvp
InputName=fdx8PerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelDirectional1_Specular_Bump.nvp
InputName=fdx8PerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelDirectional1_Specular_Bump.nvp
InputName=fdx8PerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelDirectional1_Specular_Bump.nvp
InputName=fdx8PerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelDirectional1_Specular_Bump.nvp
InputName=fdx8PerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelDirectional1_Specular_Bump.nvp
InputName=fdx8PerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelDirectional1_Specular_Bump.nvp
InputName=fdx8PerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PerPixelDirectional1_Specular_Bump.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelDirectional1_Specular_Bump.nvv
InputName=fdx8PerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelDirectional1_Specular_Bump.nvv
InputName=fdx8PerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelDirectional1_Specular_Bump.nvv
InputName=fdx8PerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelDirectional1_Specular_Bump.nvv
InputName=fdx8PerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelDirectional1_Specular_Bump.nvv
InputName=fdx8PerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelDirectional1_Specular_Bump.nvv
InputName=fdx8PerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelDirectional1_Specular_Bump.nvv
InputName=fdx8PerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelDirectional1_Specular_Bump.nvv
InputName=fdx8PerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PerPixelPoint1_Diffuse.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse.nvp
InputName=fdx8PerPixelPoint1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse.nvp
InputName=fdx8PerPixelPoint1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse.nvp
InputName=fdx8PerPixelPoint1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse.nvp
InputName=fdx8PerPixelPoint1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse.nvp
InputName=fdx8PerPixelPoint1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse.nvp
InputName=fdx8PerPixelPoint1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse.nvp
InputName=fdx8PerPixelPoint1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse.nvp
InputName=fdx8PerPixelPoint1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PerPixelPoint1_Diffuse.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse.nvv
InputName=fdx8PerPixelPoint1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse.nvv
InputName=fdx8PerPixelPoint1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse.nvv
InputName=fdx8PerPixelPoint1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse.nvv
InputName=fdx8PerPixelPoint1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse.nvv
InputName=fdx8PerPixelPoint1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse.nvv
InputName=fdx8PerPixelPoint1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse.nvv
InputName=fdx8PerPixelPoint1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse.nvv
InputName=fdx8PerPixelPoint1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PerPixelPoint1_Diffuse_Bump.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse_Bump.nvp
InputName=fdx8PerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse_Bump.nvp
InputName=fdx8PerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse_Bump.nvp
InputName=fdx8PerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse_Bump.nvp
InputName=fdx8PerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse_Bump.nvp
InputName=fdx8PerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse_Bump.nvp
InputName=fdx8PerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse_Bump.nvp
InputName=fdx8PerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse_Bump.nvp
InputName=fdx8PerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PerPixelPoint1_Diffuse_Bump.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse_Bump.nvv
InputName=fdx8PerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse_Bump.nvv
InputName=fdx8PerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse_Bump.nvv
InputName=fdx8PerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse_Bump.nvv
InputName=fdx8PerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse_Bump.nvv
InputName=fdx8PerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse_Bump.nvv
InputName=fdx8PerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse_Bump.nvv
InputName=fdx8PerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint1_Diffuse_Bump.nvv
InputName=fdx8PerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PerPixelPoint2_Diffuse.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint2_Diffuse.nvp
InputName=fdx8PerPixelPoint2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint2_Diffuse.nvp
InputName=fdx8PerPixelPoint2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint2_Diffuse.nvp
InputName=fdx8PerPixelPoint2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint2_Diffuse.nvp
InputName=fdx8PerPixelPoint2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint2_Diffuse.nvp
InputName=fdx8PerPixelPoint2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint2_Diffuse.nvp
InputName=fdx8PerPixelPoint2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint2_Diffuse.nvp
InputName=fdx8PerPixelPoint2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint2_Diffuse.nvp
InputName=fdx8PerPixelPoint2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PerPixelPoint2_Diffuse.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint2_Diffuse.nvv
InputName=fdx8PerPixelPoint2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint2_Diffuse.nvv
InputName=fdx8PerPixelPoint2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint2_Diffuse.nvv
InputName=fdx8PerPixelPoint2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint2_Diffuse.nvv
InputName=fdx8PerPixelPoint2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint2_Diffuse.nvv
InputName=fdx8PerPixelPoint2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint2_Diffuse.nvv
InputName=fdx8PerPixelPoint2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint2_Diffuse.nvv
InputName=fdx8PerPixelPoint2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelPoint2_Diffuse.nvv
InputName=fdx8PerPixelPoint2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PerPixelSpot1_Diffuse.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse.nvp
InputName=fdx8PerPixelSpot1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse.nvp
InputName=fdx8PerPixelSpot1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse.nvp
InputName=fdx8PerPixelSpot1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse.nvp
InputName=fdx8PerPixelSpot1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse.nvp
InputName=fdx8PerPixelSpot1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse.nvp
InputName=fdx8PerPixelSpot1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse.nvp
InputName=fdx8PerPixelSpot1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse.nvp
InputName=fdx8PerPixelSpot1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PerPixelSpot1_Diffuse.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse.nvv
InputName=fdx8PerPixelSpot1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse.nvv
InputName=fdx8PerPixelSpot1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse.nvv
InputName=fdx8PerPixelSpot1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse.nvv
InputName=fdx8PerPixelSpot1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse.nvv
InputName=fdx8PerPixelSpot1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse.nvv
InputName=fdx8PerPixelSpot1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse.nvv
InputName=fdx8PerPixelSpot1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse.nvv
InputName=fdx8PerPixelSpot1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PerPixelSpot1_Diffuse_Bump.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse_Bump.nvp
InputName=fdx8PerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse_Bump.nvp
InputName=fdx8PerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse_Bump.nvp
InputName=fdx8PerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse_Bump.nvp
InputName=fdx8PerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse_Bump.nvp
InputName=fdx8PerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse_Bump.nvp
InputName=fdx8PerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse_Bump.nvp
InputName=fdx8PerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse_Bump.nvp
InputName=fdx8PerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PerPixelSpot1_Diffuse_Bump.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse_Bump.nvv
InputName=fdx8PerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse_Bump.nvv
InputName=fdx8PerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse_Bump.nvv
InputName=fdx8PerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse_Bump.nvv
InputName=fdx8PerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse_Bump.nvv
InputName=fdx8PerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse_Bump.nvv
InputName=fdx8PerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse_Bump.nvv
InputName=fdx8PerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot1_Diffuse_Bump.nvv
InputName=fdx8PerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PerPixelSpot2_Diffuse.nvp

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot2_Diffuse.nvp
InputName=fdx8PerPixelSpot2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot2_Diffuse.nvp
InputName=fdx8PerPixelSpot2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot2_Diffuse.nvp
InputName=fdx8PerPixelSpot2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot2_Diffuse.nvp
InputName=fdx8PerPixelSpot2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot2_Diffuse.nvp
InputName=fdx8PerPixelSpot2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot2_Diffuse.nvp
InputName=fdx8PerPixelSpot2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot2_Diffuse.nvp
InputName=fdx8PerPixelSpot2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot2_Diffuse.nvp
InputName=fdx8PerPixelSpot2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvp $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8PerPixelSpot2_Diffuse.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot2_Diffuse.nvv
InputName=fdx8PerPixelSpot2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot2_Diffuse.nvv
InputName=fdx8PerPixelSpot2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot2_Diffuse.nvv
InputName=fdx8PerPixelSpot2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot2_Diffuse.nvv
InputName=fdx8PerPixelSpot2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot2_Diffuse.nvv
InputName=fdx8PerPixelSpot2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot2_Diffuse.nvv
InputName=fdx8PerPixelSpot2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot2_Diffuse.nvv
InputName=fdx8PerPixelSpot2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8PerPixelSpot2_Diffuse.nvv
InputName=fdx8PerPixelSpot2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8Point1Dir1_Diffuse.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point1Dir1_Diffuse.nvv
InputName=fdx8Point1Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point1Dir1_Diffuse.nvv
InputName=fdx8Point1Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point1Dir1_Diffuse.nvv
InputName=fdx8Point1Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point1Dir1_Diffuse.nvv
InputName=fdx8Point1Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point1Dir1_Diffuse.nvv
InputName=fdx8Point1Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point1Dir1_Diffuse.nvv
InputName=fdx8Point1Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point1Dir1_Diffuse.nvv
InputName=fdx8Point1Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point1Dir1_Diffuse.nvv
InputName=fdx8Point1Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8Point1Dir1_Diffuse_Blend.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point1Dir1_Diffuse_Blend.nvv
InputName=fdx8Point1Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point1Dir1_Diffuse_Blend.nvv
InputName=fdx8Point1Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point1Dir1_Diffuse_Blend.nvv
InputName=fdx8Point1Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point1Dir1_Diffuse_Blend.nvv
InputName=fdx8Point1Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point1Dir1_Diffuse_Blend.nvv
InputName=fdx8Point1Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point1Dir1_Diffuse_Blend.nvv
InputName=fdx8Point1Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point1Dir1_Diffuse_Blend.nvv
InputName=fdx8Point1Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point1Dir1_Diffuse_Blend.nvv
InputName=fdx8Point1Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8Point2Dir1_Diffuse.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point2Dir1_Diffuse.nvv
InputName=fdx8Point2Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point2Dir1_Diffuse.nvv
InputName=fdx8Point2Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point2Dir1_Diffuse.nvv
InputName=fdx8Point2Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point2Dir1_Diffuse.nvv
InputName=fdx8Point2Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point2Dir1_Diffuse.nvv
InputName=fdx8Point2Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point2Dir1_Diffuse.nvv
InputName=fdx8Point2Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point2Dir1_Diffuse.nvv
InputName=fdx8Point2Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point2Dir1_Diffuse.nvv
InputName=fdx8Point2Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8Point2Dir1_Diffuse_Blend.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point2Dir1_Diffuse_Blend.nvv
InputName=fdx8Point2Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point2Dir1_Diffuse_Blend.nvv
InputName=fdx8Point2Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point2Dir1_Diffuse_Blend.nvv
InputName=fdx8Point2Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point2Dir1_Diffuse_Blend.nvv
InputName=fdx8Point2Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point2Dir1_Diffuse_Blend.nvv
InputName=fdx8Point2Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point2Dir1_Diffuse_Blend.nvv
InputName=fdx8Point2Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point2Dir1_Diffuse_Blend.nvv
InputName=fdx8Point2Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point2Dir1_Diffuse_Blend.nvv
InputName=fdx8Point2Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8Point4Dir1_Diffuse.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point4Dir1_Diffuse.nvv
InputName=fdx8Point4Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point4Dir1_Diffuse.nvv
InputName=fdx8Point4Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point4Dir1_Diffuse.nvv
InputName=fdx8Point4Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point4Dir1_Diffuse.nvv
InputName=fdx8Point4Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point4Dir1_Diffuse.nvv
InputName=fdx8Point4Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point4Dir1_Diffuse.nvv
InputName=fdx8Point4Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point4Dir1_Diffuse.nvv
InputName=fdx8Point4Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point4Dir1_Diffuse.nvv
InputName=fdx8Point4Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8Point4Dir1_Diffuse_Blend.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point4Dir1_Diffuse_Blend.nvv
InputName=fdx8Point4Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point4Dir1_Diffuse_Blend.nvv
InputName=fdx8Point4Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point4Dir1_Diffuse_Blend.nvv
InputName=fdx8Point4Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point4Dir1_Diffuse_Blend.nvv
InputName=fdx8Point4Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point4Dir1_Diffuse_Blend.nvv
InputName=fdx8Point4Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point4Dir1_Diffuse_Blend.nvv
InputName=fdx8Point4Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point4Dir1_Diffuse_Blend.nvv
InputName=fdx8Point4Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point4Dir1_Diffuse_Blend.nvv
InputName=fdx8Point4Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8Point6Dir1_Diffuse.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point6Dir1_Diffuse.nvv
InputName=fdx8Point6Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point6Dir1_Diffuse.nvv
InputName=fdx8Point6Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point6Dir1_Diffuse.nvv
InputName=fdx8Point6Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point6Dir1_Diffuse.nvv
InputName=fdx8Point6Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point6Dir1_Diffuse.nvv
InputName=fdx8Point6Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point6Dir1_Diffuse.nvv
InputName=fdx8Point6Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point6Dir1_Diffuse.nvv
InputName=fdx8Point6Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point6Dir1_Diffuse.nvv
InputName=fdx8Point6Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8Point6Dir1_Diffuse_Blend.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point6Dir1_Diffuse_Blend.nvv
InputName=fdx8Point6Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point6Dir1_Diffuse_Blend.nvv
InputName=fdx8Point6Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point6Dir1_Diffuse_Blend.nvv
InputName=fdx8Point6Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point6Dir1_Diffuse_Blend.nvv
InputName=fdx8Point6Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point6Dir1_Diffuse_Blend.nvv
InputName=fdx8Point6Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point6Dir1_Diffuse_Blend.nvv
InputName=fdx8Point6Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point6Dir1_Diffuse_Blend.nvv
InputName=fdx8Point6Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point6Dir1_Diffuse_Blend.nvv
InputName=fdx8Point6Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8Point7Dir1_Diffuse.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point7Dir1_Diffuse.nvv
InputName=fdx8Point7Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point7Dir1_Diffuse.nvv
InputName=fdx8Point7Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point7Dir1_Diffuse.nvv
InputName=fdx8Point7Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point7Dir1_Diffuse.nvv
InputName=fdx8Point7Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point7Dir1_Diffuse.nvv
InputName=fdx8Point7Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point7Dir1_Diffuse.nvv
InputName=fdx8Point7Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point7Dir1_Diffuse.nvv
InputName=fdx8Point7Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point7Dir1_Diffuse.nvv
InputName=fdx8Point7Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8Point7Dir1_Diffuse_Blend.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point7Dir1_Diffuse_Blend.nvv
InputName=fdx8Point7Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point7Dir1_Diffuse_Blend.nvv
InputName=fdx8Point7Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point7Dir1_Diffuse_Blend.nvv
InputName=fdx8Point7Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point7Dir1_Diffuse_Blend.nvv
InputName=fdx8Point7Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point7Dir1_Diffuse_Blend.nvv
InputName=fdx8Point7Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point7Dir1_Diffuse_Blend.nvv
InputName=fdx8Point7Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point7Dir1_Diffuse_Blend.nvv
InputName=fdx8Point7Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point7Dir1_Diffuse_Blend.nvv
InputName=fdx8Point7Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8Point8_Diffuse.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point8_Diffuse.nvv
InputName=fdx8Point8_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point8_Diffuse.nvv
InputName=fdx8Point8_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point8_Diffuse.nvv
InputName=fdx8Point8_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point8_Diffuse.nvv
InputName=fdx8Point8_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point8_Diffuse.nvv
InputName=fdx8Point8_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point8_Diffuse.nvv
InputName=fdx8Point8_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point8_Diffuse.nvv
InputName=fdx8Point8_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8Point8_Diffuse.nvv
InputName=fdx8Point8_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8VtxColor.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8VtxColor.nvv
InputName=fdx8VtxColor

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8VtxColor.nvv
InputName=fdx8VtxColor

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8VtxColor.nvv
InputName=fdx8VtxColor

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8VtxColor.nvv
InputName=fdx8VtxColor

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8VtxColor.nvv
InputName=fdx8VtxColor

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8VtxColor.nvv
InputName=fdx8VtxColor

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8VtxColor.nvv
InputName=fdx8VtxColor

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8VtxColor.nvv
InputName=fdx8VtxColor

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8VtxColor_blend.nvv

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8VtxColor_blend.nvv
InputName=fdx8VtxColor_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8VtxColor_blend.nvv
InputName=fdx8VtxColor_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8VtxColor_blend.nvv
InputName=fdx8VtxColor_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8VtxColor_blend.nvv
InputName=fdx8VtxColor_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8VtxColor_blend.nvv
InputName=fdx8VtxColor_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8VtxColor_blend.nvv
InputName=fdx8VtxColor_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8VtxColor_blend.nvv
InputName=fdx8VtxColor_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdx8VtxColor_blend.nvv
InputName=fdx8VtxColor_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	nvasm -h $(InputName).nvv $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=..\..\fang.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fanim.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fbitstream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fboxfilter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fcamera.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fclib.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fcoll.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fcolor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fdatapool.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fdev.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Fdraw.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8.cpp
# End Source File
# Begin Source File

SOURCE=..\fDX8AMem.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8anim.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8audio.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8collasm.asm

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=..\fdx8collasm.asm
InputName=fdx8collasm

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /coff /c /Fo$(IntDir)\$(InputName).obj /Fl$(IntDir)\$(InputName).lst /Sa /Sc /Zd /Zf /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=..\fdx8collasm.asm
InputName=fdx8collasm

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /coff /c /Fo$(IntDir)\$(InputName).obj /Fl$(IntDir)\$(InputName).lst /Sa /Sc /Zd /Zf /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Test
InputPath=..\fdx8collasm.asm
InputName=fdx8collasm

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /coff /c /Fo$(IntDir)\$(InputName).obj /Fl$(IntDir)\$(InputName).lst /Sa /Sc /Zd /Zf /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Production
InputPath=..\fdx8collasm.asm
InputName=fdx8collasm

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /coff /c /Fo$(IntDir)\$(InputName).obj /Fl$(IntDir)\$(InputName).lst /Sa /Sc /Zd /Zf /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build - Assembling $(InputPath)
IntDir=.\fang2win___Win32_Debug_Unicode
InputPath=..\fdx8collasm.asm
InputName=fdx8collasm

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /coff /c /Fo$(IntDir)\$(InputName).obj /Fl$(IntDir)\$(InputName).lst /Sa /Sc /Zd /Zf /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build - Assembling $(InputPath)
IntDir=.\fang2win___Win32_Release_Unicode
InputPath=..\fdx8collasm.asm
InputName=fdx8collasm

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /coff /c /Fo$(IntDir)\$(InputName).obj /Fl$(IntDir)\$(InputName).lst /Sa /Sc /Zd /Zf /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build - Assembling $(InputPath)
IntDir=.\GCDebug
InputPath=..\fdx8collasm.asm
InputName=fdx8collasm

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /coff /c /Fo$(IntDir)\$(InputName).obj /Fl$(IntDir)\$(InputName).lst /Sa /Sc /Zd /Zf /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build - Assembling $(InputPath)
IntDir=.\GCRelease
InputPath=..\fdx8collasm.asm
InputName=fdx8collasm

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /coff /c /Fo$(IntDir)\$(InputName).obj /Fl$(IntDir)\$(InputName).lst /Sa /Sc /Zd /Zf /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fdx8cpu.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8draw.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8file.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8force.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8load.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8loop.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8math.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8math_mtx.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8math_quat.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8math_vec.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8mesh.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8mesh_coll.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8movie.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8padio.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8psprite.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8sh.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8shadow.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8storage.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8sysinfo.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8tex.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8timer.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8vb.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8vid.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8viewport.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8xfm.cpp
# End Source File
# Begin Source File

SOURCE=..\fdxsh_psprite.vsh

!IF  "$(CFG)" == "fang2win - Win32 Release"

# Begin Custom Build - Assembling Shader $(InputPath)
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdxsh_psprite.vsh
InputName=fdxsh_psprite

BuildCmds= \
	nvasm -bhl $(InputPath) $(InputDir)\win\$(InputName).cvs $(InputDir)\win\$(InputName).txt

"$(InputDir)\win\$(InputName).cvs" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\win\$(InputName).txt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug"

# Begin Custom Build - Assembling Shader $(InputPath)
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdxsh_psprite.vsh
InputName=fdxsh_psprite

BuildCmds= \
	nvasm -bhl $(InputPath) $(InputDir)\win\$(InputName).cvs $(InputDir)\win\$(InputName).txt

"$(InputDir)\win\$(InputName).cvs" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\win\$(InputName).txt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Test"

# Begin Custom Build - Assembling Shader $(InputPath)
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdxsh_psprite.vsh
InputName=fdxsh_psprite

BuildCmds= \
	nvasm -bhl $(InputPath) $(InputDir)\win\$(InputName).cvs $(InputDir)\win\$(InputName).txt

"$(InputDir)\win\$(InputName).cvs" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\win\$(InputName).txt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Production"

# Begin Custom Build - Assembling Shader $(InputPath)
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdxsh_psprite.vsh
InputName=fdxsh_psprite

BuildCmds= \
	nvasm -bhl $(InputPath) $(InputDir)\win\$(InputName).cvs $(InputDir)\win\$(InputName).txt

"$(InputDir)\win\$(InputName).cvs" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\win\$(InputName).txt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Debug Unicode"

# Begin Custom Build - Assembling Shader $(InputPath)
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdxsh_psprite.vsh
InputName=fdxsh_psprite

BuildCmds= \
	nvasm -bhl $(InputPath) $(InputDir)\win\$(InputName).cvs $(InputDir)\win\$(InputName).txt

"$(InputDir)\win\$(InputName).cvs" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\win\$(InputName).txt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 Release Unicode"

# Begin Custom Build - Assembling Shader $(InputPath)
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdxsh_psprite.vsh
InputName=fdxsh_psprite

BuildCmds= \
	nvasm -bhl $(InputPath) $(InputDir)\win\$(InputName).cvs $(InputDir)\win\$(InputName).txt

"$(InputDir)\win\$(InputName).cvs" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\win\$(InputName).txt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCDebug"

# Begin Custom Build - Assembling Shader $(InputPath)
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdxsh_psprite.vsh
InputName=fdxsh_psprite

BuildCmds= \
	nvasm -bhl $(InputPath) $(InputDir)\win\$(InputName).cvs $(InputDir)\win\$(InputName).txt

"$(InputDir)\win\$(InputName).cvs" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\win\$(InputName).txt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2win - Win32 GCRelease"

# Begin Custom Build - Assembling Shader $(InputPath)
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdxsh_psprite.vsh
InputName=fdxsh_psprite

BuildCmds= \
	nvasm -bhl $(InputPath) $(InputDir)\win\$(InputName).cvs $(InputDir)\win\$(InputName).txt

"$(InputDir)\win\$(InputName).cvs" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\win\$(InputName).txt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\FEventListener.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ff32hash.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ffile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fgamedata.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fguid.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fhash.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fheap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FkDOP.cpp
# End Source File
# Begin Source File

SOURCE=..\..\flight.cpp
# End Source File
# Begin Source File

SOURCE=..\..\flinklist.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fmasterfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fmath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fmath_geo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Fmesh.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fmesh_coll.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fmotion.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FMotionObj.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FNativeUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fpad.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fpart.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fparticle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fperf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fpsprite.cpp
# End Source File
# Begin Source File

SOURCE=..\..\frenderer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fRenderSort.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fres.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fresload.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FScalarObj.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FScript.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FScriptInst.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FScriptSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FScriptTypes.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fshaders.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fshadow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fsintbl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fsndfx.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fstringtable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ftext.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ftextmon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FVec3Obj.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fversion.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fviewport.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fvis.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Fworld.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fworld_coll.cpp
# End Source File
# Begin Source File

SOURCE=..\..\fxfm.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "FVec3Obj Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\FMulPath1.h
# End Source File
# Begin Source File

SOURCE=..\..\FPointPath1.h
# End Source File
# Begin Source File

SOURCE=..\..\FPointPath2.h
# End Source File
# Begin Source File

SOURCE=..\..\FSumPath1.h
# End Source File
# Begin Source File

SOURCE=..\..\FV3OCircleXZ.h
# End Source File
# Begin Source File

SOURCE=..\..\FV3OConst.h
# End Source File
# Begin Source File

SOURCE=..\..\FV3OLine1.h
# End Source File
# End Group
# Begin Group "FQuatObj Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\FQOTang1.h
# End Source File
# Begin Source File

SOURCE=..\..\FQuatComp.h
# End Source File
# Begin Source File

SOURCE=..\..\FQuatConst.h
# End Source File
# Begin Source File

SOURCE=..\..\FQuatLookAt1.h
# End Source File
# Begin Source File

SOURCE=..\..\FQuatObj.h
# End Source File
# Begin Source File

SOURCE=..\..\FQuatSLERP.h
# End Source File
# Begin Source File

SOURCE=..\..\FQuatTang2.h
# End Source File
# Begin Source File

SOURCE=..\..\FQuatTang3.h
# End Source File
# Begin Source File

SOURCE=..\..\FQuatTwirl.h
# End Source File
# End Group
# Begin Group "FScalarObj Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\FScalarBlend.h
# End Source File
# Begin Source File

SOURCE=..\..\FScalarConst.h
# End Source File
# Begin Source File

SOURCE=..\..\FScalarSinus.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\fang.h
# End Source File
# Begin Source File

SOURCE=..\..\fangalign.h
# End Source File
# Begin Source File

SOURCE=..\..\fangmacros.h
# End Source File
# Begin Source File

SOURCE=..\..\fangtypes.h
# End Source File
# Begin Source File

SOURCE=..\..\fanim.h
# End Source File
# Begin Source File

SOURCE=..\..\faudio.h
# End Source File
# Begin Source File

SOURCE=..\..\fbitstream.h
# End Source File
# Begin Source File

SOURCE=..\..\fboxfilter.h
# End Source File
# Begin Source File

SOURCE=..\..\fcamera.h
# End Source File
# Begin Source File

SOURCE=..\..\fcdrom.h
# End Source File
# Begin Source File

SOURCE=..\..\fclib.h
# End Source File
# Begin Source File

SOURCE=..\..\fcoll.h
# End Source File
# Begin Source File

SOURCE=..\..\fcolor.h
# End Source File
# Begin Source File

SOURCE=..\..\fcpu.h
# End Source File
# Begin Source File

SOURCE=..\..\fdata.h
# End Source File
# Begin Source File

SOURCE=..\..\fdatapool.h
# End Source File
# Begin Source File

SOURCE=..\..\fdev.h
# End Source File
# Begin Source File

SOURCE=..\..\fdraw.h
# End Source File
# Begin Source File

SOURCE=..\fdx8.h
# End Source File
# Begin Source File

SOURCE=..\fdx8anim.inl
# End Source File
# Begin Source File

SOURCE=..\fdx8data.h
# End Source File
# Begin Source File

SOURCE=..\fdx8draw.h
# End Source File
# Begin Source File

SOURCE=..\fdx8file.h
# End Source File
# Begin Source File

SOURCE=..\fdx8fontdata.h
# End Source File
# Begin Source File

SOURCE=..\fdx8fonttex.h
# End Source File
# Begin Source File

SOURCE=..\fdx8gcmath_mtx.inl
# End Source File
# Begin Source File

SOURCE=..\fdx8gcmath_quat.inl
# End Source File
# Begin Source File

SOURCE=..\fdx8gcmath_vec.inl
# End Source File
# Begin Source File

SOURCE=..\fdx8load.h
# End Source File
# Begin Source File

SOURCE=..\fdx8loop.h
# End Source File
# Begin Source File

SOURCE=..\fdx8math.h
# End Source File
# Begin Source File

SOURCE=..\fdx8math.inl
# End Source File
# Begin Source File

SOURCE=..\fdx8math_mtx.inl
# End Source File
# Begin Source File

SOURCE=..\fdx8math_quat.inl
# End Source File
# Begin Source File

SOURCE=..\fdx8math_vec.inl
# End Source File
# Begin Source File

SOURCE=..\fdx8mesh.h
# End Source File
# Begin Source File

SOURCE=..\fdx8sh.h
# End Source File
# Begin Source File

SOURCE=..\fdx8shaders.h
# End Source File
# Begin Source File

SOURCE=..\fdx8shadow.h
# End Source File
# Begin Source File

SOURCE=..\fdx8tex.h
# End Source File
# Begin Source File

SOURCE=..\fdx8vb.h
# End Source File
# Begin Source File

SOURCE=..\fdx8vid.h
# End Source File
# Begin Source File

SOURCE=..\fdx8viewport.h
# End Source File
# Begin Source File

SOURCE=..\fdx8vshader_const.h
# End Source File
# Begin Source File

SOURCE=..\fdx8xfm.h
# End Source File
# Begin Source File

SOURCE=..\..\FEventListener.h
# End Source File
# Begin Source File

SOURCE=..\..\ff32hash.h
# End Source File
# Begin Source File

SOURCE=..\..\ffile.h
# End Source File
# Begin Source File

SOURCE=..\..\fforce.h
# End Source File
# Begin Source File

SOURCE=..\..\fgamedata.h
# End Source File
# Begin Source File

SOURCE=..\..\gc\fGCdata.h
# End Source File
# Begin Source File

SOURCE=..\..\fguid.h
# End Source File
# Begin Source File

SOURCE=..\..\fhash.h
# End Source File
# Begin Source File

SOURCE=..\..\fheap.h
# End Source File
# Begin Source File

SOURCE=..\..\FkDOP.h
# End Source File
# Begin Source File

SOURCE=..\..\flight.h
# End Source File
# Begin Source File

SOURCE=..\..\flinklist.h
# End Source File
# Begin Source File

SOURCE=..\..\flinklist.inl
# End Source File
# Begin Source File

SOURCE=..\..\floop.h
# End Source File
# Begin Source File

SOURCE=..\..\fmasterfile.h
# End Source File
# Begin Source File

SOURCE=..\..\fmath.h
# End Source File
# Begin Source File

SOURCE=..\..\fmath_geo.h
# End Source File
# Begin Source File

SOURCE=..\..\fmath_geo.inl
# End Source File
# Begin Source File

SOURCE=..\..\fmath_mtx.h
# End Source File
# Begin Source File

SOURCE=..\..\fmath_quat.h
# End Source File
# Begin Source File

SOURCE=..\..\fmath_vec.h
# End Source File
# Begin Source File

SOURCE=..\..\fmesh.h
# End Source File
# Begin Source File

SOURCE=..\..\fmesh_coll.h
# End Source File
# Begin Source File

SOURCE=..\..\fmotion.h
# End Source File
# Begin Source File

SOURCE=..\..\FMotionObj.h
# End Source File
# Begin Source File

SOURCE=..\..\fmovie.h
# End Source File
# Begin Source File

SOURCE=..\..\FNativeUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\fpad.h
# End Source File
# Begin Source File

SOURCE=..\..\fpadio.h
# End Source File
# Begin Source File

SOURCE=..\..\fpart.h
# End Source File
# Begin Source File

SOURCE=..\..\fparticle.h
# End Source File
# Begin Source File

SOURCE=..\..\fperf.h
# End Source File
# Begin Source File

SOURCE=..\..\fpsprite.h
# End Source File
# Begin Source File

SOURCE=..\..\frenderer.h
# End Source File
# Begin Source File

SOURCE=..\..\fRenderSort.h
# End Source File
# Begin Source File

SOURCE=..\..\fres.h
# End Source File
# Begin Source File

SOURCE=..\..\fresload.h
# End Source File
# Begin Source File

SOURCE=..\..\FScalarObj.h
# End Source File
# Begin Source File

SOURCE=..\..\FScript.h
# End Source File
# Begin Source File

SOURCE=..\..\FScriptInst.h
# End Source File
# Begin Source File

SOURCE=..\..\FScriptSystem.h
# End Source File
# Begin Source File

SOURCE=..\..\FScriptTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\fsh.h
# End Source File
# Begin Source File

SOURCE=..\..\fshaders.h
# End Source File
# Begin Source File

SOURCE=..\..\fshadow.h
# End Source File
# Begin Source File

SOURCE=..\..\fsintbl.h
# End Source File
# Begin Source File

SOURCE=..\..\fsndfx.h
# End Source File
# Begin Source File

SOURCE=..\..\fstorage.h
# End Source File
# Begin Source File

SOURCE=..\..\fstringtable.h
# End Source File
# Begin Source File

SOURCE=..\..\fsysinfo.h
# End Source File
# Begin Source File

SOURCE=..\..\ftex.h
# End Source File
# Begin Source File

SOURCE=..\..\ftext.h
# End Source File
# Begin Source File

SOURCE=..\..\ftextmon.h
# End Source File
# Begin Source File

SOURCE=..\..\ftimer.h
# End Source File
# Begin Source File

SOURCE=..\..\FVec3Obj.h
# End Source File
# Begin Source File

SOURCE=..\..\fversion.h
# End Source File
# Begin Source File

SOURCE=..\..\fvid.h
# End Source File
# Begin Source File

SOURCE=..\..\fviewport.h
# End Source File
# Begin Source File

SOURCE=..\..\fvis.h
# End Source File
# Begin Source File

SOURCE=..\..\fworld.h
# End Source File
# Begin Source File

SOURCE=..\..\fworld_coll.h
# End Source File
# Begin Source File

SOURCE=..\..\fxfm.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# End Target
# End Project

# Microsoft Developer Studio Project File - Name="fang2xb" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Xbox Static Library" 0x0b04

CFG=fang2xb - Xbox Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fang2xb.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fang2xb.mak" CFG="fang2xb - Xbox Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fang2xb - Xbox Release" (based on "Xbox Static Library")
!MESSAGE "fang2xb - Xbox Debug" (based on "Xbox Static Library")
!MESSAGE "fang2xb - Xbox Test" (based on "Xbox Static Library")
!MESSAGE "fang2xb - Xbox Production" (based on "Xbox Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/Src/Lib/fang2/dx/xb", XHAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe

!IF  "$(CFG)" == "fang2xb - Xbox Release"

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
RSC=rc.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /I "..\..\..\SmallAmx" /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "_FANGDEF_PLATFORM_XB" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\fang2xb_r.lib"

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

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
RSC=rc.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\.." /I "..\SmallAmx" /I "..\..\..\SmallAmx" /I ".." /D "WIN32" /D "_XBOX" /D "_DEBUG" /D "_FANGDEF_PLATFORM_XB" /FR /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\fang2xb_d.lib"

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fang2xb___Xbox_Test"
# PROP BASE Intermediate_Dir "fang2xb___Xbox_Test"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Test"
# PROP Intermediate_Dir "Test"
# PROP Target_Dir ""
RSC=rc.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "_FANGDEF_PLATFORM_XB" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /Gi /GX /O2 /Ob2 /I "..\.." /I "..\SmallAmx" /I "..\..\..\SmallAmx" /I ".." /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "_FANGDEF_PLATFORM_XB" /D "_FANGDEF_TEST_BUILD" /D "_AFXDLL" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\..\..\lib\fang2xb_r.lib"
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\fang2xb_t.lib"

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fang2xb___Xbox_Production"
# PROP BASE Intermediate_Dir "fang2xb___Xbox_Production"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Production"
# PROP Intermediate_Dir "Production"
# PROP Target_Dir ""
RSC=rc.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "_FANGDEF_PLATFORM_XB" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /Gi /GX /O2 /Ob2 /I "..\.." /I "..\SmallAmx" /I "..\..\..\SmallAmx" /I ".." /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "_FANGDEF_PLATFORM_XB" /D "_FANGDEF_PRODUCTION_BUILD" /D "_AFXDLL" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\..\..\lib\fang2xb_r.lib"
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\fang2xb_p.lib"

!ENDIF 

# Begin Target

# Name "fang2xb - Xbox Release"
# Name "fang2xb - Xbox Debug"
# Name "fang2xb - Xbox Test"
# Name "fang2xb - Xbox Production"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;vsh;psh;asm"
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
# Begin Group "FQuatObj Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\FQOTang1.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FQuatComp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FQuatTang2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FQuatTang3.cpp
# End Source File
# End Group
# Begin Group "Shaders"

# PROP Default_Filter "vsh;psh"
# Begin Source File

SOURCE=..\fxbColor.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbColor.psh
InputName=fxbColor

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbColor.psh
InputName=fxbColor

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbColor.psh
InputName=fxbColor

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbColor.psh
InputName=fxbColor

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbColorAlphaMask.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbColorAlphaMask.psh
InputName=fxbColorAlphaMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbColorAlphaMask.psh
InputName=fxbColorAlphaMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbColorAlphaMask.psh
InputName=fxbColorAlphaMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbColorAlphaMask.psh
InputName=fxbColorAlphaMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbColorEMask.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbColorEMask.psh
InputName=fxbColorEMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbColorEMask.psh
InputName=fxbColorEMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbColorEMask.psh
InputName=fxbColorEMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbColorEMask.psh
InputName=fxbColorEMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbColorMask.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbColorMask.psh
InputName=fxbColorMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbColorMask.psh
InputName=fxbColorMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbColorMask.psh
InputName=fxbColorMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbColorMask.psh
InputName=fxbColorMask

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbDetail.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbDetail.psh
InputName=fxbDetail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbDetail.psh
InputName=fxbDetail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbDetail.psh
InputName=fxbDetail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbDetail.psh
InputName=fxbDetail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbDetail.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\CODE\Src\Lib\Fang2\dx
InputPath=..\fxbDetail.vsh
InputName=fxbDetail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbDetail_2tc.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbDetail_2tc.vsh
InputName=fxbDetail_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\CODE\Src\Lib\Fang2\dx
InputPath=..\fxbDetail.vsh
InputName=fxbDetail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\CODE\Src\Lib\Fang2\dx
InputPath=..\fxbDetail.vsh
InputName=fxbDetail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\CODE\Src\Lib\Fang2\dx
InputPath=..\fxbDetail.vsh
InputName=fxbDetail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbDirectional1_Specular.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbDirectional1_Specular.vsh
InputName=fxbDirectional1_Specular

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbDirectional1_Specular.vsh
InputName=fxbDirectional1_Specular

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbDirectional1_Specular.vsh
InputName=fxbDirectional1_Specular

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbDirectional1_Specular.vsh
InputName=fxbDirectional1_Specular

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbIntensity.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbIntensity.psh
InputName=fxbIntensity

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbIntensity.psh
InputName=fxbIntensity

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbIntensity.psh
InputName=fxbIntensity

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbIntensity.psh
InputName=fxbIntensity

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxboBase_Add_rbENV.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV.psh
InputName=fxboBase_Add_rbENV

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV.psh
InputName=fxboBase_Add_rbENV

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV.psh
InputName=fxboBase_Add_rbENV

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV.psh
InputName=fxboBase_Add_rbENV

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxboBase_Add_rbENV.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV.vsh
InputName=fxboBase_Add_rbENV

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV.vsh
InputName=fxboBase_Add_rbENV

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV.vsh
InputName=fxboBase_Add_rbENV

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV.vsh
InputName=fxboBase_Add_rbENV

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxboBase_Add_rbENV_blend.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV_blend.vsh
InputName=fxboBase_Add_rbENV_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV_blend.vsh
InputName=fxboBase_Add_rbENV_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV_blend.vsh
InputName=fxboBase_Add_rbENV_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV_blend.vsh
InputName=fxboBase_Add_rbENV_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxboBase_Add_rbENV_Detail.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV_Detail.psh
InputName=fxboBase_Add_rbENV_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV_Detail.psh
InputName=fxboBase_Add_rbENV_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV_Detail.psh
InputName=fxboBase_Add_rbENV_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV_Detail.psh
InputName=fxboBase_Add_rbENV_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxboBase_Add_rbENV_Detail.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV_Detail.vsh
InputName=fxboBase_Add_rbENV_Detail

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\CODE\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV_blend.vsh
InputName=fxboBase_Add_rbENV_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\CODE\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV_blend.vsh
InputName=fxboBase_Add_rbENV_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\CODE\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Add_rbENV_blend.vsh
InputName=fxboBase_Add_rbENV_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxboBase_Lerp_pLayer.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_pLayer.psh
InputName=fxboBase_Lerp_pLayer

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_pLayer.psh
InputName=fxboBase_Lerp_pLayer

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_pLayer.psh
InputName=fxboBase_Lerp_pLayer

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_pLayer.psh
InputName=fxboBase_Lerp_pLayer

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxboBase_Lerp_pLayer_Detail.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_pLayer_Detail.psh
InputName=fxboBase_Lerp_pLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_pLayer_Detail.psh
InputName=fxboBase_Lerp_pLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_pLayer_Detail.psh
InputName=fxboBase_Lerp_pLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_pLayer_Detail.psh
InputName=fxboBase_Lerp_pLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxboBase_Lerp_tLayer.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_tLayer.psh
InputName=fxboBase_Lerp_tLayer

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_tLayer.psh
InputName=fxboBase_Lerp_tLayer

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_tLayer.psh
InputName=fxboBase_Lerp_tLayer

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_tLayer.psh
InputName=fxboBase_Lerp_tLayer

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxboBase_Lerp_tLayer_Detail.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_tLayer_Detail.psh
InputName=fxboBase_Lerp_tLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_tLayer_Detail.psh
InputName=fxboBase_Lerp_tLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_tLayer_Detail.psh
InputName=fxboBase_Lerp_tLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\CODE\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_tLayer.psh
InputName=fxboBase_Lerp_tLayer

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxboBase_Lerp_vLayer.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\CODE\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_vLayer.psh
InputName=fxboBase_Lerp_vLayer

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxboBase_Lerp_vLayer_Detail.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_vLayer_Detail.psh
InputName=fxboBase_Lerp_vLayer_Detail

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\CODE\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_vLayer.psh
InputName=fxboBase_Lerp_vLayer

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\CODE\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_vLayer.psh
InputName=fxboBase_Lerp_vLayer

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\CODE\Src\Lib\Fang2\dx
InputPath=..\fxboBase_Lerp_vLayer.psh
InputName=fxboBase_Lerp_vLayer

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPassThru.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru.psh
InputName=fxbPassThru

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru.psh
InputName=fxbPassThru

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru.psh
InputName=fxbPassThru

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru.psh
InputName=fxbPassThru

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPassThru_1tc.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru_1tc.vsh
InputName=fxbPassThru_1tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru_1tc.vsh
InputName=fxbPassThru_1tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru_1tc.vsh
InputName=fxbPassThru_1tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru_1tc.vsh
InputName=fxbPassThru_1tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPassThru_1tc_blend.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru_1tc_blend.vsh
InputName=fxbPassThru_1tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru_1tc_blend.vsh
InputName=fxbPassThru_1tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru_1tc_blend.vsh
InputName=fxbPassThru_1tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru_1tc_blend.vsh
InputName=fxbPassThru_1tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPassThru_2tc.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru_2tc.vsh
InputName=fxbPassThru_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru_2tc.vsh
InputName=fxbPassThru_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru_2tc.vsh
InputName=fxbPassThru_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru_2tc.vsh
InputName=fxbPassThru_2tc

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPassThru_2tc_blend.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru_2tc_blend.vsh
InputName=fxbPassThru_2tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru_2tc_blend.vsh
InputName=fxbPassThru_2tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru_2tc_blend.vsh
InputName=fxbPassThru_2tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPassThru_2tc_blend.vsh
InputName=fxbPassThru_2tc_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPerPixelDirectional1_Specular_Bump.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelDirectional1_Specular_Bump.psh
InputName=fxbPerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelDirectional1_Specular_Bump.psh
InputName=fxbPerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelDirectional1_Specular_Bump.psh
InputName=fxbPerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelDirectional1_Specular_Bump.psh
InputName=fxbPerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPerPixelDirectional1_Specular_Bump.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelDirectional1_Specular_Bump.vsh
InputName=fxbPerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelDirectional1_Specular_Bump.vsh
InputName=fxbPerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelDirectional1_Specular_Bump.vsh
InputName=fxbPerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelDirectional1_Specular_Bump.vsh
InputName=fxbPerPixelDirectional1_Specular_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPerPixelPoint1_Diffuse.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint1_Diffuse.psh
InputName=fxbPerPixelPoint1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint1_Diffuse.psh
InputName=fxbPerPixelPoint1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint1_Diffuse.psh
InputName=fxbPerPixelPoint1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint1_Diffuse.psh
InputName=fxbPerPixelPoint1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPerPixelPoint1_Diffuse.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint1_Diffuse.vsh
InputName=fxbPerPixelPoint1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint1_Diffuse.vsh
InputName=fxbPerPixelPoint1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint1_Diffuse.vsh
InputName=fxbPerPixelPoint1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint1_Diffuse.vsh
InputName=fxbPerPixelPoint1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPerPixelPoint1_Diffuse_Bump.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint1_Diffuse_Bump.psh
InputName=fxbPerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint1_Diffuse_Bump.psh
InputName=fxbPerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint1_Diffuse_Bump.psh
InputName=fxbPerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint1_Diffuse_Bump.psh
InputName=fxbPerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPerPixelPoint1_Diffuse_Bump.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint1_Diffuse_Bump.vsh
InputName=fxbPerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint1_Diffuse_Bump.vsh
InputName=fxbPerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint1_Diffuse_Bump.vsh
InputName=fxbPerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint1_Diffuse_Bump.vsh
InputName=fxbPerPixelPoint1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPerPixelPoint2_Diffuse.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint2_Diffuse.psh
InputName=fxbPerPixelPoint2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint2_Diffuse.psh
InputName=fxbPerPixelPoint2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint2_Diffuse.psh
InputName=fxbPerPixelPoint2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint2_Diffuse.psh
InputName=fxbPerPixelPoint2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPerPixelPoint2_Diffuse.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint2_Diffuse.vsh
InputName=fxbPerPixelPoint2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint2_Diffuse.vsh
InputName=fxbPerPixelPoint2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint2_Diffuse.vsh
InputName=fxbPerPixelPoint2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelPoint2_Diffuse.vsh
InputName=fxbPerPixelPoint2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPerPixelSpot1_Diffuse.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot1_Diffuse.psh
InputName=fxbPerPixelSpot1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot1_Diffuse.psh
InputName=fxbPerPixelSpot1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot1_Diffuse.psh
InputName=fxbPerPixelSpot1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot1_Diffuse.psh
InputName=fxbPerPixelSpot1_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPerPixelSpot1_Diffuse.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot1_Diffuse.vsh
InputName=fxbPerPixelSpot1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot1_Diffuse.vsh
InputName=fxbPerPixelSpot1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot1_Diffuse.vsh
InputName=fxbPerPixelSpot1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot1_Diffuse.vsh
InputName=fxbPerPixelSpot1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPerPixelSpot1_Diffuse_Bump.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot1_Diffuse_Bump.psh
InputName=fxbPerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot1_Diffuse_Bump.psh
InputName=fxbPerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot1_Diffuse_Bump.psh
InputName=fxbPerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot1_Diffuse_Bump.psh
InputName=fxbPerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPerPixelSpot1_Diffuse_Bump.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot1_Diffuse_Bump.vsh
InputName=fxbPerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot1_Diffuse_Bump.vsh
InputName=fxbPerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot1_Diffuse_Bump.vsh
InputName=fxbPerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot1_Diffuse_Bump.vsh
InputName=fxbPerPixelSpot1_Diffuse_Bump

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPerPixelSpot2_Diffuse.psh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot2_Diffuse.psh
InputName=fxbPerPixelSpot2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot2_Diffuse.psh
InputName=fxbPerPixelSpot2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot2_Diffuse.psh
InputName=fxbPerPixelSpot2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot2_Diffuse.psh
InputName=fxbPerPixelSpot2_Diffuse

"$(InputDir)\CompiledPShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).psh $(InputDir)\CompiledPShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPerPixelSpot2_Diffuse.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot2_Diffuse.vsh
InputName=fxbPerPixelSpot2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot2_Diffuse.vsh
InputName=fxbPerPixelSpot2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot2_Diffuse.vsh
InputName=fxbPerPixelSpot2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPerPixelSpot2_Diffuse.vsh
InputName=fxbPerPixelSpot2_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPoint1Dir1_Diffuse.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint1Dir1_Diffuse.vsh
InputName=fxbPoint1Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint1Dir1_Diffuse.vsh
InputName=fxbPoint1Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint1Dir1_Diffuse.vsh
InputName=fxbPoint1Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint1Dir1_Diffuse.vsh
InputName=fxbPoint1Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPoint1Dir1_Diffuse_Blend.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint1Dir1_Diffuse_Blend.vsh
InputName=fxbPoint1Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint1Dir1_Diffuse_Blend.vsh
InputName=fxbPoint1Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint1Dir1_Diffuse_Blend.vsh
InputName=fxbPoint1Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint1Dir1_Diffuse_Blend.vsh
InputName=fxbPoint1Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPoint2Dir1_Diffuse.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint2Dir1_Diffuse.vsh
InputName=fxbPoint2Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint2Dir1_Diffuse.vsh
InputName=fxbPoint2Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint2Dir1_Diffuse.vsh
InputName=fxbPoint2Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint2Dir1_Diffuse.vsh
InputName=fxbPoint2Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPoint2Dir1_Diffuse_Blend.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint2Dir1_Diffuse_Blend.vsh
InputName=fxbPoint2Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint2Dir1_Diffuse_Blend.vsh
InputName=fxbPoint2Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint2Dir1_Diffuse_Blend.vsh
InputName=fxbPoint2Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint2Dir1_Diffuse_Blend.vsh
InputName=fxbPoint2Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPoint4Dir1_Diffuse.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint4Dir1_Diffuse.vsh
InputName=fxbPoint4Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint4Dir1_Diffuse.vsh
InputName=fxbPoint4Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint4Dir1_Diffuse.vsh
InputName=fxbPoint4Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint4Dir1_Diffuse.vsh
InputName=fxbPoint4Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPoint4Dir1_Diffuse_Blend.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint4Dir1_Diffuse_Blend.vsh
InputName=fxbPoint4Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint4Dir1_Diffuse_Blend.vsh
InputName=fxbPoint4Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint4Dir1_Diffuse_Blend.vsh
InputName=fxbPoint4Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint4Dir1_Diffuse_Blend.vsh
InputName=fxbPoint4Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPoint6Dir1_Diffuse.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint6Dir1_Diffuse.vsh
InputName=fxbPoint6Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint6Dir1_Diffuse.vsh
InputName=fxbPoint6Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint6Dir1_Diffuse.vsh
InputName=fxbPoint6Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint6Dir1_Diffuse.vsh
InputName=fxbPoint6Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPoint6Dir1_Diffuse_Blend.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint6Dir1_Diffuse_Blend.vsh
InputName=fxbPoint6Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint6Dir1_Diffuse_Blend.vsh
InputName=fxbPoint6Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint6Dir1_Diffuse_Blend.vsh
InputName=fxbPoint6Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint6Dir1_Diffuse_Blend.vsh
InputName=fxbPoint6Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPoint7Dir1_Diffuse.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint7Dir1_Diffuse.vsh
InputName=fxbPoint7Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint7Dir1_Diffuse.vsh
InputName=fxbPoint7Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint7Dir1_Diffuse.vsh
InputName=fxbPoint7Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint7Dir1_Diffuse.vsh
InputName=fxbPoint7Dir1_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPoint7Dir1_Diffuse_Blend.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint7Dir1_Diffuse_Blend.vsh
InputName=fxbPoint7Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint7Dir1_Diffuse_Blend.vsh
InputName=fxbPoint7Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint7Dir1_Diffuse_Blend.vsh
InputName=fxbPoint7Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint7Dir1_Diffuse_Blend.vsh
InputName=fxbPoint7Dir1_Diffuse_Blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbPoint8_Diffuse.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint8_Diffuse.vsh
InputName=fxbPoint8_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint8_Diffuse.vsh
InputName=fxbPoint8_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint8_Diffuse.vsh
InputName=fxbPoint8_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbPoint8_Diffuse.vsh
InputName=fxbPoint8_Diffuse

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbVtxColor.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbVtxColor.vsh
InputName=fxbVtxColor

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbVtxColor.vsh
InputName=fxbVtxColor

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbVtxColor.vsh
InputName=fxbVtxColor

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbVtxColor.vsh
InputName=fxbVtxColor

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fxbVtxColor_blend.vsh

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbVtxColor_blend.vsh
InputName=fxbVtxColor_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbVtxColor_blend.vsh
InputName=fxbVtxColor_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbVtxColor_blend.vsh
InputName=fxbVtxColor_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fxbVtxColor_blend.vsh
InputName=fxbVtxColor_blend

"$(InputDir)\CompiledVShader$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd\  
	cd $(InputDir) 
	xsasm -h $(InputName).vsh $(InputDir)\CompiledVShader$(InputName).h 
	
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

SOURCE=..\fdx8collasm.asm

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=..\fdx8collasm.asm
InputName=fdx8collasm

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /coff /c /Fo$(IntDir)\$(InputName).obj /Fl$(IntDir)\$(InputName).lst /Sa /Sc /Zd /Zf /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=..\fdx8collasm.asm
InputName=fdx8collasm

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /coff /c /Fo$(IntDir)\$(InputName).obj /Fl$(IntDir)\$(InputName).lst /Sa /Sc /Zd /Zf /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Test
InputPath=..\fdx8collasm.asm
InputName=fdx8collasm

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /coff /c /Fo$(IntDir)\$(InputName).obj /Fl$(IntDir)\$(InputName).lst /Sa /Sc /Zd /Zf /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Production
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

SOURCE=..\fdx8psprite.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8sh.cpp
# End Source File
# Begin Source File

SOURCE=..\fdx8shadow.cpp
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

!IF  "$(CFG)" == "fang2xb - Xbox Release"

# Begin Custom Build - Compiling Shader $(InputPath) $(InputDir)
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdxsh_psprite.vsh
InputName=fdxsh_psprite

BuildCmds= \
	xsasm -nologo -l -h $(InputPath) $(InputDir)\xb\$(InputName).cvs $(InputDir)\xb\$(InputName).xsc $(InputDir)\xb\$(InputName).txt

"$(InputDir)\xb\$(InputName).cvs" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\xb\$(InputName).txt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Debug"

# Begin Custom Build - Compiling Shader $(InputPath) $(InputDir)
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdxsh_psprite.vsh
InputName=fdxsh_psprite

BuildCmds= \
	xsasm -nologo -l -h $(InputPath) $(InputDir)\xb\$(InputName).cvs $(InputDir)\xb\$(InputName).xsc $(InputDir)\xb\$(InputName).txt

"$(InputDir)\xb\$(InputName).cvs" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\xb\$(InputName).txt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Test"

# Begin Custom Build - Compiling Shader $(InputPath) $(InputDir)
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdxsh_psprite.vsh
InputName=fdxsh_psprite

BuildCmds= \
	xsasm -nologo -l -h $(InputPath) $(InputDir)\xb\$(InputName).cvs $(InputDir)\xb\$(InputName).xsc $(InputDir)\xb\$(InputName).txt

"$(InputDir)\xb\$(InputName).cvs" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\xb\$(InputName).txt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "fang2xb - Xbox Production"

# Begin Custom Build - Compiling Shader $(InputPath) $(InputDir)
InputDir=\code\Src\Lib\Fang2\dx
InputPath=..\fdxsh_psprite.vsh
InputName=fdxsh_psprite

BuildCmds= \
	xsasm -nologo -l -h $(InputPath) $(InputDir)\xb\$(InputName).cvs $(InputDir)\xb\$(InputName).xsc $(InputDir)\xb\$(InputName).txt

"$(InputDir)\xb\$(InputName).cvs" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\xb\$(InputName).txt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
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

SOURCE=..\..\FQuatTwirl.cpp
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

SOURCE=..\..\FScalarBlend.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FScalarConst.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FScalarObj.cpp
# End Source File
# Begin Source File

SOURCE=..\..\FScalarSinus.cpp
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

SOURCE=..\fserver.cpp
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

SOURCE=..\fxbaudio.cpp
# End Source File
# Begin Source File

SOURCE=..\fxbforce.cpp
# End Source File
# Begin Source File

SOURCE=..\fxbpadio.cpp
# End Source File
# Begin Source File

SOURCE=..\fxbstorage.cpp
# End Source File
# Begin Source File

SOURCE=..\fxbsysinfo.cpp
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

SOURCE=..\..\FQuatTang2.h
# End Source File
# Begin Source File

SOURCE=..\..\FQuatTang3.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\fAMem.h
# End Source File
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

SOURCE=..\..\FQuatTwirl.h
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

SOURCE=..\..\FScalarBlend.h
# End Source File
# Begin Source File

SOURCE=..\..\FScalarConst.h
# End Source File
# Begin Source File

SOURCE=..\..\FScalarObj.h
# End Source File
# Begin Source File

SOURCE=..\..\FScalarSinus.h
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

SOURCE=..\fserver.h
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

SOURCE=..\fxbfontdata.h
# End Source File
# Begin Source File

SOURCE=..\fxbfonttex.h
# End Source File
# Begin Source File

SOURCE=..\..\fxfm.h
# End Source File
# End Group
# End Target
# End Project

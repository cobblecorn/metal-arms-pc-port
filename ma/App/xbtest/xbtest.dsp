# Microsoft Developer Studio Project File - Name="xbtest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Xbox Application" 0x0b01

CFG=xbtest - Xbox Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "xbtest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xbtest.mak" CFG="xbtest - Xbox Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xbtest - Xbox Release" (based on "Xbox Application")
!MESSAGE "xbtest - Xbox Debug" (based on "Xbox Application")
!MESSAGE "xbtest - Xbox Test" (based on "Xbox Application")
!MESSAGE "xbtest - Xbox Production" (based on "Xbox Application")
!MESSAGE "xbtest - Xbox VTune" (based on "Xbox Application")
!MESSAGE "xbtest - Xbox Sample" (based on "Xbox Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/Src/App/xbtest", XCBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe

!IF  "$(CFG)" == "xbtest - Xbox Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\.." /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "_FANGDEF_PLATFORM_XB" /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /machine:I386 /subsystem:xbox /fixed:no /debugtype:vc6
# ADD LINK32 ..\..\..\lib\fangxb_r.lib xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /machine:I386 /out:"Release/xbtest_r.exe" /subsystem:xbox /fixed:no /debugtype:vc6
XBE=imagebld.exe
# ADD BASE XBE /nologo /stack:0x10000
# ADD XBE /nologo /testid:"0x00000001" /testname:"Xbox Test" /testpubname:"Swingin' Ape Studios" /stack:0x10000 /out:"Release/xbtest_r.xbe"
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO

!ELSEIF  "$(CFG)" == "xbtest - Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\.." /D "WIN32" /D "_XBOX" /D "_DEBUG" /D "_FANGDEF_PLATFORM_XB" /FR /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 xapilibd.lib d3d8d.lib d3dx8d.lib xgraphicsd.lib dsoundd.lib dmusicd.lib xnetd.lib xboxkrnl.lib /nologo /incremental:no /debug /machine:I386 /subsystem:xbox /fixed:no /debugtype:vc6
# ADD LINK32 ..\..\..\lib\fangxb_d.lib xapilibd.lib d3d8d.lib d3dx8d.lib xgraphicsd.lib dsoundd.lib dmusicd.lib xnetd.lib xboxkrnl.lib /nologo /incremental:no /debug /machine:I386 /out:"Debug/xbtest_d.exe" /subsystem:xbox /fixed:no /debugtype:vc6
XBE=imagebld.exe
# ADD BASE XBE /nologo /stack:0x10000 /debug
# ADD XBE /nologo /testid:"0x00000001" /testname:"Xbox Test" /testpubname:"Swingin' Ape Studios" /stack:0x10000 /debug /out:"Debug/xbtest_d.xbe"
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO

!ELSEIF  "$(CFG)" == "xbtest - Xbox Test"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "xbtest___Xbox_Test"
# PROP BASE Intermediate_Dir "xbtest___Xbox_Test"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Test"
# PROP Intermediate_Dir "Test"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "_FANGDEF_PLATFORM_XB" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\.." /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "_FANGDEF_PLATFORM_XB" /D "_FANGDEF_TEST_BUILD" /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /machine:I386 /out:"Release/xbtest_r.exe" /subsystem:xbox /fixed:no /debugtype:vc6
# ADD LINK32 ..\..\..\lib\fangxb_t.lib xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /machine:I386 /out:"Test/xbtest_t.exe" /subsystem:xbox /fixed:no /debugtype:vc6
XBE=imagebld.exe
# ADD BASE XBE /nologo /testid:"0x00000001" /testname:"Xbox Test" /testpubname:"Swingin' Ape Studios" /stack:0x10000 /out:"Release/xbtest_r.xbe"
# ADD XBE /nologo /testid:"0x00000001" /testname:"Xbox Test" /testpubname:"Swingin' Ape Studios" /stack:0x10000 /out:"Test/xbtest_t.xbe"
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO

!ELSEIF  "$(CFG)" == "xbtest - Xbox Production"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "xbtest___Xbox_Production"
# PROP BASE Intermediate_Dir "xbtest___Xbox_Production"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Production"
# PROP Intermediate_Dir "Production"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "_FANGDEF_PLATFORM_XB" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\.." /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "_FANGDEF_PLATFORM_XB" /D "_FANGDEF_PRODUCTION_BUILD" /FR /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /machine:I386 /out:"Release/xbtest_r.exe" /subsystem:xbox /fixed:no /debugtype:vc6
# ADD LINK32 ..\..\..\lib\fangxb_p.lib xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /machine:I386 /out:"Production/xbtest_p.exe" /subsystem:xbox /fixed:no /debugtype:vc6
XBE=imagebld.exe
# ADD BASE XBE /nologo /testid:"0x00000001" /testname:"Xbox Test" /testpubname:"Swingin' Ape Studios" /stack:0x10000 /out:"Release/xbtest_r.xbe"
# ADD XBE /nologo /testid:"0x00000001" /testname:"Xbox Test" /testpubname:"Swingin' Ape Studios" /stack:0x10000 /out:"Production/xbtest_p.xbe"
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO

!ELSEIF  "$(CFG)" == "xbtest - Xbox VTune"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "xbtest___Xbox_VTune"
# PROP BASE Intermediate_Dir "xbtest___Xbox_VTune"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "VTune"
# PROP Intermediate_Dir "VTune"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "_FANGDEF_PLATFORM_XB" /D "_FANGDEF_PRODUCTION_BUILD" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /I "..\.." /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "_FANGDEF_PLATFORM_XB" /D "_FANGDEF_PRODUCTION_BUILD" /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /machine:I386 /out:"Release/xbtest_p.exe" /subsystem:xbox /fixed:no /debugtype:vc6
# ADD LINK32 ..\..\..\lib\fangxb_v.lib xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /debug /machine:I386 /out:"VTune/xbtest_v.exe" /subsystem:xbox /fixed:no /debugtype:vc6
XBE=imagebld.exe
# ADD BASE XBE /nologo /testid:"0x00000001" /testname:"Xbox Test" /testpubname:"Swingin' Ape Studios" /stack:0x10000 /out:"Release/xbtest_p.xbe"
# ADD XBE /nologo /testid:"0x00000001" /testname:"Xbox Test" /testpubname:"Swingin' Ape Studios" /stack:0x10000 /out:"VTune/xbtest_v.xbe"
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO

!ELSEIF  "$(CFG)" == "xbtest - Xbox Sample"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "xbtest___Xbox_Sample"
# PROP BASE Intermediate_Dir "xbtest___Xbox_Sample"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Sample"
# PROP Intermediate_Dir "Sample"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "_FANGDEF_PLATFORM_XB" /D "_FANGDEF_TEST_BUILD" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\.." /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "_FANGDEF_PLATFORM_XB" /D "_FANGDEF_SAMPLE_BUILD" /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /machine:I386 /out:"Test/xbtest_t.exe" /subsystem:xbox /fixed:no /debugtype:vc6
# ADD LINK32 ..\..\..\lib\fangxb_s.lib xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /machine:I386 /out:"Sample/xbtest_s.exe" /subsystem:xbox /fixed:no /debugtype:vc6
XBE=imagebld.exe
# ADD BASE XBE /nologo /testid:"0x00000001" /testname:"Xbox Test" /testpubname:"Swingin' Ape Studios" /stack:0x10000 /out:"Test/xbtest_t.xbe"
# ADD XBE /nologo /testid:"0x00000001" /testname:"Xbox Test" /testpubname:"Swingin' Ape Studios" /stack:0x10000 /out:"Sample/xbtest_s.xbe"
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO

!ENDIF 

# Begin Target

# Name "xbtest - Xbox Release"
# Name "xbtest - Xbox Debug"
# Name "xbtest - Xbox Test"
# Name "xbtest - Xbox Production"
# Name "xbtest - Xbox VTune"
# Name "xbtest - Xbox Sample"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\xbtest.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project

# Microsoft Developer Studio Project File - Name="pasm2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=pasm2 - Win32 GCDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pasm2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pasm2.mak" CFG="pasm2 - Win32 GCDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pasm2 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "pasm2 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "pasm2 - Win32 GCDebug" (based on "Win32 (x86) Application")
!MESSAGE "pasm2 - Win32 GCRelease" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/Src/Exp/Pasm2", SJBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pasm2 - Win32 Release"

# PROP BASE Use_MFC 5
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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /Ot /Og /Oi /Oy /Gf /Gy /I "..\..\lib\SortedListCtrl" /I "..\..\lib\strip_actc" /I "..\..\lib\SmallCompiler" /I "..\..\lib\ZipArchive" /I "..\..\lib\GCTexLib" /I "..\..\lib\Fang2MeshLib" /I "..\..\lib\nv_norm_lib" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /D AIGRAPH_EDITOR_ENABLED=2 /FR /YX /FD /Gs /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 fang2win_r.lib strip_actc_r.lib SortedListCtrl_r.lib Small_Compiler_r.lib ZipArchive_r.lib GCTexLib_r.lib Fang2MeshLib_r.lib AIGraphConvert_R.lib dsound.lib dinput8.lib dxguid.lib D3dxof.lib d3d8.lib d3dx8.lib winmm.lib nvDXTlib.lib msacm32.lib nv_norm_lib_r.lib /nologo /subsystem:windows /profile /machine:I386 /out:"Release/pasm.exe" /libpath:"..\..\lib\GCTexLib" /libpath:"..\..\lib\nv_norm_lib"

!ELSEIF  "$(CFG)" == "pasm2 - Win32 Debug"

# PROP BASE Use_MFC 5
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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I "..\..\lib\SortedListCtrl" /I "..\..\lib\strip_actc" /I "..\..\lib\SmallCompiler" /I "..\..\lib\ZipArchive" /I "..\..\lib\GCTexLib" /I "..\..\lib\Fang2MeshLib" /I "..\..\lib\nv_norm_lib" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /D AIGRAPH_EDITOR_ENABLED=2 /Fr /YX"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 fang2win_d.lib strip_actc_d.lib SortedListCtrl_d.lib Small_Compiler_d.lib ZipArchive_d.lib GCTexLib_d.lib Fang2MeshLib_d.lib AIGraphConvert_D.lib dsound.lib dinput8.lib dxguid.lib D3dxof.lib d3d8.lib d3dx8.lib winmm.lib nvDXTlib.lib msacm32.lib nv_norm_lib_d.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/pasm.exe" /libpath:"..\..\lib\GCTexLib" /libpath:"..\..\lib\nv_norm_lib"
# SUBTRACT LINK32 /verbose /profile /nodefaultlib

!ELSEIF  "$(CFG)" == "pasm2 - Win32 GCDebug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pasm2___Win32_GCDebug"
# PROP BASE Intermediate_Dir "pasm2___Win32_GCDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "GCDebug"
# PROP Intermediate_Dir "GCDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I "..\..\lib\SortedListCtrl" /I "..\..\lib\quant\quantlib" /I "..\..\lib\strip_actc" /I "..\..\lib\SmallCompiler" /I "..\..\lib\ZipArchive" /I "..\..\lib\GCTexLib" /I "..\..\lib\GCMeshLib" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /D AIGRAPH_EDITOR_ENABLED=2 /Fr /YX"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I "..\..\lib\SortedListCtrl" /I "..\..\lib\strip_actc" /I "..\..\lib\SmallCompiler" /I "..\..\lib\ZipArchive" /I "..\..\lib\GCTexLib" /I "..\..\lib\Fang2MeshLib" /I "..\..\lib\nv_norm_lib" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /D AIGRAPH_EDITOR_ENABLED=2 /D "_FANGDEF_WINGC" /Fr /YX"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 fang2win_d.lib quantlib_d.lib strip_actc_d.lib SortedListCtrl_d.lib dsound.lib dinput8.lib dxguid.lib D3dxof.lib d3d8.lib d3dx8.lib winmm.lib Small_Compiler_d.lib ZipArchive_d.lib nvDXTlib.lib GCTexLib_d.lib GCMeshLib_d.lib AIGraphConvert_D.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/pasm.exe" /libpath:"..\..\lib\GCTexLib"
# SUBTRACT BASE LINK32 /verbose /profile /nodefaultlib
# ADD LINK32 fang2win_gcd.lib strip_actc_gcd.lib SortedListCtrl_gcd.lib Small_Compiler_gcd.lib ZipArchive_gcd.lib GCTexLib_gcd.lib Fang2MeshLib_gcd.lib AIGraphConvert_gcD.lib dsound.lib dinput8.lib dxguid.lib D3dxof.lib d3d8.lib d3dx8.lib winmm.lib nvDXTlib.lib msacm32.lib nv_norm_lib_gcd.lib /nologo /subsystem:windows /debug /machine:I386 /out:"GCDebug/pasmGC.exe" /libpath:"..\..\lib\GCTexLib" /libpath:"..\..\lib\nv_norm_lib"
# SUBTRACT LINK32 /verbose /profile /nodefaultlib

!ELSEIF  "$(CFG)" == "pasm2 - Win32 GCRelease"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pasm2___Win32_GCRelease"
# PROP BASE Intermediate_Dir "pasm2___Win32_GCRelease"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "GCRelease"
# PROP Intermediate_Dir "GCRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /GX /O2 /I "..\..\lib\SortedListCtrl" /I "..\..\lib\quant\quantlib" /I "..\..\lib\strip_actc" /I "..\..\lib\SmallCompiler" /I "..\..\lib\ZipArchive" /I "..\..\lib\GCTexLib" /I "..\..\lib\GCMeshLib" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /D AIGRAPH_EDITOR_ENABLED=2 /FR /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /Ot /Og /Oi /Oy /Ob2 /Gf /Gy /I "..\..\lib\SortedListCtrl" /I "..\..\lib\strip_actc" /I "..\..\lib\SmallCompiler" /I "..\..\lib\ZipArchive" /I "..\..\lib\GCTexLib" /I "..\..\lib\Fang2MeshLib" /I "..\..\lib\nv_norm_lib" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /D AIGRAPH_EDITOR_ENABLED=2 /D "_FANGDEF_WINGC" /FR /YX /FD /Gs /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 fang2win_r.lib quantlib_r.lib strip_actc_r.lib SortedListCtrl_r.lib dsound.lib dinput8.lib dxguid.lib D3dxof.lib d3d8.lib d3dx8.lib winmm.lib Small_Compiler_r.lib ZipArchive_r.lib nvDXTlib.lib GCTexLib_r.lib GCMeshLib_r.lib AIGraphConvert_R.lib /nologo /subsystem:windows /profile /machine:I386 /out:"Release/pasm.exe" /libpath:"..\..\lib\GCTexLib"
# ADD LINK32 fang2win_gcr.lib strip_actc_gcr.lib SortedListCtrl_gcr.lib Small_Compiler_gcr.lib ZipArchive_gcr.lib GCTexLib_gcr.lib Fang2MeshLib_gcr.lib AIGraphConvert_gcr.lib dsound.lib dinput8.lib dxguid.lib D3dxof.lib d3d8.lib d3dx8.lib winmm.lib nvDXTlib.lib msacm32.lib nv_norm_lib_gcr.lib /nologo /subsystem:windows /debug /machine:I386 /out:"GCRelease/pasmGC.exe" /libpath:"..\..\lib\GCTexLib" /libpath:"..\..\lib\nv_norm_lib"
# SUBTRACT LINK32 /profile

!ENDIF 

# Begin Target

# Name "pasm2 - Win32 Release"
# Name "pasm2 - Win32 Debug"
# Name "pasm2 - Win32 GCDebug"
# Name "pasm2 - Win32 GCRelease"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AidFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ApeFileLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\ApeToGCFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ApeToGCWorldFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ApeToKongFormat.cpp
# End Source File
# Begin Source File

SOURCE=.\ApeToXBoxFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ApeToXBoxFile2.cpp
# End Source File
# Begin Source File

SOURCE=.\ApeToXBoxWorldFile.cpp
# End Source File
# Begin Source File

SOURCE=.\CompileDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ConvertApeFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ConvertCsvFiles.cpp
# End Source File
# Begin Source File

SOURCE=.\ConvertFntFiles.cpp
# End Source File
# Begin Source File

SOURCE=.\ConvertFprFiles.cpp
# End Source File
# Begin Source File

SOURCE=.\ConvertGtFiles.cpp
# End Source File
# Begin Source File

SOURCE=.\ConvertMtxFiles.cpp
# End Source File
# Begin Source File

SOURCE=.\ConvertSfbFiles.cpp
# End Source File
# Begin Source File

SOURCE=.\ConvertSmaFiles.cpp
# End Source File
# Begin Source File

SOURCE=.\ConvertWvbFile_XBox.cpp
# End Source File
# Begin Source File

SOURCE=.\CreateGCTgaFile.cpp
# End Source File
# Begin Source File

SOURCE=.\CreateXBoxTgaFile.cpp
# End Source File
# Begin Source File

SOURCE=.\CShellFileOp.cpp
# End Source File
# Begin Source File

SOURCE=.\DeleteDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DisplayList.cpp
# End Source File
# Begin Source File

SOURCE=.\ErrorLog.cpp
# End Source File
# Begin Source File

SOURCE=.\FileInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\FileLock.cpp
# End Source File
# Begin Source File

SOURCE=.\FixedData.cpp
# End Source File
# Begin Source File

SOURCE=.\GenMipMaps.cpp
# End Source File
# Begin Source File

SOURCE=.\KongToChimp.cpp
# End Source File
# Begin Source File

SOURCE=.\KongToVisFile.cpp
# End Source File
# Begin Source File

SOURCE=.\KongToWorldFile.cpp
# End Source File
# Begin Source File

SOURCE=.\KongToWorldInitFile.cpp
# End Source File
# Begin Source File

SOURCE=.\KongTriClipper.cpp
# End Source File
# Begin Source File

SOURCE=.\Leaf.cpp
# End Source File
# Begin Source File

SOURCE=.\MasterFileCompile.cpp
# End Source File
# Begin Source File

SOURCE=.\MyItemInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\MyListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\pasm.cpp
# End Source File
# Begin Source File

SOURCE=.\pasmDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\pickdir.cpp
# End Source File
# Begin Source File

SOURCE=.\PortalData.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\SeqBank.cpp
# End Source File
# Begin Source File

SOURCE=.\Settings.cpp
# End Source File
# Begin Source File

SOURCE=.\SettingsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StringInput.cpp
# End Source File
# Begin Source File

SOURCE=.\SyncDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\TGAFileLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\ToolTipButton.cpp
# End Source File
# Begin Source File

SOURCE=.\TriCollection.cpp
# End Source File
# Begin Source File

SOURCE=.\TriPacket.cpp
# End Source File
# Begin Source File

SOURCE=.\TriPacketList.cpp
# End Source File
# Begin Source File

SOURCE=.\utils.cpp
# End Source File
# Begin Source File

SOURCE=.\WavBank.cpp
# End Source File
# Begin Source File

SOURCE=.\WavFile.cpp
# End Source File
# Begin Source File

SOURCE=.\WavToADPCM_Xbox.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AidFile.h
# End Source File
# Begin Source File

SOURCE=.\ape_file_def.h
# End Source File
# Begin Source File

SOURCE=.\ApeFileLoader.h
# End Source File
# Begin Source File

SOURCE=.\ApeToGCFile.h
# End Source File
# Begin Source File

SOURCE=.\ApeToGCWorldFile.h
# End Source File
# Begin Source File

SOURCE=.\ApeToKongFormat.h
# End Source File
# Begin Source File

SOURCE=.\ApeToXBoxFile.h
# End Source File
# Begin Source File

SOURCE=.\ApeToXBoxWorldFile.h
# End Source File
# Begin Source File

SOURCE=.\CompileDlg.h
# End Source File
# Begin Source File

SOURCE=.\ConfigFile.h
# End Source File
# Begin Source File

SOURCE=.\ConvertApeFile.h
# End Source File
# Begin Source File

SOURCE=.\ConvertCsvFiles.h
# End Source File
# Begin Source File

SOURCE=.\ConvertFile.h
# End Source File
# Begin Source File

SOURCE=.\ConvertFntFiles.h
# End Source File
# Begin Source File

SOURCE=.\ConvertFprFiles.h
# End Source File
# Begin Source File

SOURCE=.\ConvertGtFiles.h
# End Source File
# Begin Source File

SOURCE=.\ConvertMtxFiles.h
# End Source File
# Begin Source File

SOURCE=.\ConvertSfbFiles.h
# End Source File
# Begin Source File

SOURCE=.\ConvertSmaFiles.h
# End Source File
# Begin Source File

SOURCE=.\ConvertWvbFile_XBox.h
# End Source File
# Begin Source File

SOURCE=.\CreateGCTgaFile.h
# End Source File
# Begin Source File

SOURCE=.\CreateXBoxTgaFile.h
# End Source File
# Begin Source File

SOURCE=.\CSharedStruct.h
# End Source File
# Begin Source File

SOURCE=.\CShellFileOp.h
# End Source File
# Begin Source File

SOURCE=.\CSpreadSheet.h
# End Source File
# Begin Source File

SOURCE=.\dds.h
# End Source File
# Begin Source File

SOURCE=.\DeleteDlg.h
# End Source File
# Begin Source File

SOURCE=.\DisplayList.h
# End Source File
# Begin Source File

SOURCE=.\dxtlib.h
# End Source File
# Begin Source File

SOURCE=.\ErrorLog.h
# End Source File
# Begin Source File

SOURCE=.\FileInfo.h
# End Source File
# Begin Source File

SOURCE=.\FileLock.h
# End Source File
# Begin Source File

SOURCE=.\FixedData.h
# End Source File
# Begin Source File

SOURCE=.\GenMipMaps.h
# End Source File
# Begin Source File

SOURCE=.\InterProcessData.h
# End Source File
# Begin Source File

SOURCE=.\KongDef.h
# End Source File
# Begin Source File

SOURCE=.\KongToChimp.h
# End Source File
# Begin Source File

SOURCE=.\KongToVisFile.h
# End Source File
# Begin Source File

SOURCE=.\KongToWorldFile.h
# End Source File
# Begin Source File

SOURCE=.\KongToWorldInitFile.h
# End Source File
# Begin Source File

SOURCE=.\KongTriClipper.h
# End Source File
# Begin Source File

SOURCE=.\Leaf.h
# End Source File
# Begin Source File

SOURCE=.\MasterFileCompile.h
# End Source File
# Begin Source File

SOURCE=.\MyItemInfo.h
# End Source File
# Begin Source File

SOURCE=.\MyListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\nvdxt_options.h
# End Source File
# Begin Source File

SOURCE=.\pasm.h
# End Source File
# Begin Source File

SOURCE=.\pasmDlg.h
# End Source File
# Begin Source File

SOURCE=.\pickdir.h
# End Source File
# Begin Source File

SOURCE=.\PortalData.h
# End Source File
# Begin Source File

SOURCE=.\ProgressWnd.h
# End Source File
# Begin Source File

SOURCE=.\SeqBank.h
# End Source File
# Begin Source File

SOURCE=.\Settings.h
# End Source File
# Begin Source File

SOURCE=.\SettingsDlg.h
# End Source File
# Begin Source File

SOURCE=.\Shaders.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\StringInput.h
# End Source File
# Begin Source File

SOURCE=.\SyncDialog.h
# End Source File
# Begin Source File

SOURCE=.\TGAFileLoader.h
# End Source File
# Begin Source File

SOURCE=.\ToolTipButton.h
# End Source File
# Begin Source File

SOURCE=.\TriCollection.h
# End Source File
# Begin Source File

SOURCE=.\TriPacket.h
# End Source File
# Begin Source File

SOURCE=.\TriPacketList.h
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# Begin Source File

SOURCE=.\WavBank.h
# End Source File
# Begin Source File

SOURCE=.\WavFile.h
# End Source File
# Begin Source File

SOURCE=.\WavToADPCM_Xbox.h
# End Source File
# Begin Source File

SOURCE=.\WorldDef.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\pasm2.ico
# End Source File
# Begin Source File

SOURCE=.\pasm2.rc
# End Source File
# Begin Source File

SOURCE=.\res\pasm2.rc2
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\nvDXTlib.lib
# End Source File
# End Target
# End Project

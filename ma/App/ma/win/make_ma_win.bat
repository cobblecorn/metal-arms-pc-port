cd \code\src\app\ma\win
echo off
SET ALLUSERSPROFILE=C:\Documents and Settings\All Users
SET APPDATA=C:\Documents and Settings\pmackellar\Application Data
SET CommonProgramFiles=C:\Program Files\Common Files
SET COMPUTERNAME=PAT-P4
SET ComSpec=C:\WINNT\system32\cmd.exe
SET HOMEDRIVE=C:
SET HOMEPATH=\
SET INCLUDE=C:\sdk\DXSDK\include;C:\sdk\Microsoft SDK\include;C:\Program Files\Microsoft Visual Studio .NET\Vc7\include;C:\Program Files\Microsoft Visual Studio .NET\Vc7\atlmfc\include;C:\Program Files\Microsoft Visual Studio .NET\Vc7\PlatformSDK\include\prerelease;C:\Program Files\Microsoft Visual Studio .NET\Vc7\PlatformSDK\include;C:\Program Files\Microsoft Visual Studio .NET\FrameworkSDK\include;
SET LIB=C:\sdk\Microsoft Xbox SDK\Lib;C:\Program Files\Microsoft Visual Studio .NET\Vc7\lib;C:\Program Files\Microsoft Visual Studio .NET\Vc7\atlmfc\lib;C:\Program Files\Microsoft Visual Studio .NET\Vc7\PlatformSDK\lib\prerelease;C:\Program Files\Microsoft Visual Studio .NET\Vc7\PlatformSDK\lib;C:\Program Files\Microsoft Visual Studio .NET\FrameworkSDK\lib;
SET LIBPATH=
SET LOGONSERVER=\\PAT-P4
SET NUMBER_OF_PROCESSORS=1
SET OS=Windows_NT
SET Os2LibPath=C:\WINNT\system32\os2\dll;
SET Path=C:\Program Files\Microsoft Visual Studio .NET\Vc7\bin;C:\Program Files\Microsoft Visual Studio .NET\Common7\Tools\bin\prerelease;C:\Program Files\Microsoft Visual Studio .NET\Common7\Tools\bin;C:\Program Files\Microsoft Visual Studio .NET\Common7\tools;C:\Program Files\Microsoft Visual Studio .NET\Common7\ide;C:\Program Files\HTML Help Workshop\;C:\Program Files\Microsoft Visual Studio .NET\FrameworkSDK\bin;C:\WINNT\Microsoft.NET\Framework\v1.0.3705;C:\WINNT\system32;C:\WINNT;C:\WINNT\System32\Wbem;
SET PATHEXT=.COM;.EXE;.BAT;.CMD;.VBS;.VBE;.JS;.JSE;.WSF;.WSH
SET PROCESSOR_ARCHITECTURE=x86
SET PROCESSOR_IDENTIFIER=x86 Family 15 Model 1 Stepping 2, GenuineIntel
SET PROCESSOR_LEVEL=15
SET PROCESSOR_REVISION=0102
SET ProgramFiles=C:\Program Files
SET SystemDrive=C:
SET SystemRoot=C:\WINNT
SET TEMP=C:\DOCUME~1\PMACKE~1\LOCALS~1\Temp
SET TMP=C:\DOCUME~1\PMACKE~1\LOCALS~1\Tmp
SET USERDOMAIN=PAT-P4
SET USERNAME=pmackellar
SET USERPROFILE=C:\Documents and Settings\pmackellar
SET VSCOMNTOOLS="C:\Program Files\Microsoft Visual Studio .NET\Common7\Tools\"
SET windir=C:\WINNT
SET XDK=C:\sdk\Microsoft Xbox SDK
SET _ACP_ATLPROV=C:\Program Files\Microsoft Visual Studio .NET\Vc7\bin\ATLPROV.DLL
SET _ACP_INCLUDE=C:\sdk\DXSDK\include;C:\sdk\Microsoft SDK\include;C:\Program Files\Microsoft Visual Studio .NET\Vc7\include;C:\Program Files\Microsoft Visual Studio .NET\Vc7\atlmfc\include;C:\Program Files\Microsoft Visual Studio .NET\Vc7\PlatformSDK\include\prerelease;C:\Program Files\Microsoft Visual Studio .NET\Vc7\PlatformSDK\include;C:\Program Files\Microsoft Visual Studio .NET\FrameworkSDK\include;
SET _ACP_LIB=C:\sdk\Microsoft Xbox SDK\Lib;C:\Program Files\Microsoft Visual Studio .NET\Vc7\lib;C:\Program Files\Microsoft Visual Studio .NET\Vc7\atlmfc\lib;C:\Program Files\Microsoft Visual Studio .NET\Vc7\PlatformSDK\lib\prerelease;C:\Program Files\Microsoft Visual Studio .NET\Vc7\PlatformSDK\lib;C:\Program Files\Microsoft Visual Studio .NET\FrameworkSDK\lib;;cl.exe
SET _ACP_PATH=C:\Program Files\Microsoft Visual Studio .NET\Vc7\bin;C:\Program Files\Microsoft Visual Studio .NET\Common7\Tools\bin\prerelease;C:\Program Files\Microsoft Visual Studio .NET\Common7\Tools\bin;C:\Program Files\Microsoft Visual Studio .NET\Common7\tools;C:\Program Files\Microsoft Visual Studio .NET\Common7\ide;C:\Program Files\HTML Help Workshop\;C:\Program Files\Microsoft Visual Studio .NET\FrameworkSDK\bin;C:\WINNT\Microsoft.NET\Framework\v1.0.3705;C:\WINNT\system32;C:\WINNT;C:\WINNT\System32\Wbem;
echo on

del /q %1\*.obj
del /q %1\*.exe
cl.exe @compflags_win_%1.rss @ma_win_sourcefiles.rss /nologo
link.exe @linkflags_win_%1.rss @ma_win_%1_objfiles.rss


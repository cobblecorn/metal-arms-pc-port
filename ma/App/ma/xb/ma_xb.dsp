# Microsoft Developer Studio Project File - Name="ma_xb" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Xbox Application" 0x0b01

CFG=ma_xb - Xbox Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ma_xb.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ma_xb.mak" CFG="ma_xb - Xbox Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ma_xb - Xbox Release" (based on "Xbox Application")
!MESSAGE "ma_xb - Xbox Debug" (based on "Xbox Application")
!MESSAGE "ma_xb - Xbox Test" (based on "Xbox Application")
!MESSAGE "ma_xb - Xbox Production" (based on "Xbox Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/Src/App/ma/xb/ma_xb", UTDAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe

!IF  "$(CFG)" == "ma_xb - Xbox Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /Gi /GX /Zi /O2 /Ob2 /I "..\..\..\lib\SmallAmx" /I "..\\" /D "NDEBUG" /D "WIN32" /D "_XBOX" /D "_FANGDEF_PLATFORM_XB" /YX /FD /G6 /Ztmp /c
# SUBTRACT CPP /Fr
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /machine:I386 /subsystem:xbox /fixed:no /tmp /OPT:REF
# ADD LINK32 fang2xb_r.lib smallamxxb_r.lib xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib XbDm.lib /nologo /debug /machine:I386 /out:"Release/ma_xb_r.exe" /libpath:"c:\branch\lib" /subsystem:xbox /fixed:no /tmp /OPT:REF
# SUBTRACT LINK32 /pdb:none
XBE=imagebld.exe
# ADD BASE XBE /nologo /stack:0x10000
# ADD XBE /nologo /testname:"Mettle Arms MS12 RELEASE" /stack:0x10000 /out:"Release/ma_xb_r.xbe" /limitmem /testratings:0xFFFFFFFF
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO

!ELSEIF  "$(CFG)" == "ma_xb - Xbox Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /Gm /Gi /GX /Zi /Od /Gf /Gy /I "..\..\..\lib\SmallAmx" /I "..\\" /D "_DEBUG" /D "_FANGDEF_PLATFORM_XB" /D "WIN32" /D "_XBOX" /FR /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 xapilibd.lib d3d8d.lib d3dx8d.lib xgraphicsd.lib dsoundd.lib dmusicd.lib xnetd.lib xboxkrnl.lib /nologo /incremental:no /debug /machine:I386 /subsystem:xbox /fixed:no /tmp
# ADD LINK32 fang2xb_d.lib smallamxxb_d.lib xapilibd.lib d3d8d.lib d3dx8d.lib xgraphicsd.lib dsoundd.lib dmusicd.lib xnetd.lib xboxkrnl.lib XbDm.lib /nologo /incremental:no /debug /machine:I386 /out:"Debug/ma_xb_d.exe" /libpath:"..\..\..\..\lib" /subsystem:xbox /fixed:no /tmp
# SUBTRACT LINK32 /pdb:none
XBE=imagebld.exe
# ADD BASE XBE /nologo /stack:0x10000 /debug
# ADD XBE /nologo /testname:"Mettle Arms MS12 DEBUG" /stack:0x10000 /debug /out:"Debug/ma_xb_d.xbe" /limitmem /testratings:0xFFFFFFFF
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO

!ELSEIF  "$(CFG)" == "ma_xb - Xbox Test"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ma_xb___Xbox_Test"
# PROP BASE Intermediate_Dir "ma_xb___Xbox_Test"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Test"
# PROP Intermediate_Dir "Test"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gi /GX /O2 /Ob2 /D "NDEBUG" /D "WIN32" /D "_XBOX" /D "_FANGDEF_PLATFORM_XB" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /Gi /GX /Zi /O2 /Ob2 /I "..\..\..\lib\SmallAmx" /I "..\\" /D "NDEBUG" /D "WIN32" /D "_XBOX" /D "_FANGDEF_PLATFORM_XB" /D "_FANGDEF_TEST_BUILD" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /machine:I386 /out:"Release/ma_xb_r.exe" /subsystem:xbox /fixed:no /tmp /OPT:REF
# SUBTRACT BASE LINK32 /incremental:yes
# ADD LINK32 xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /debug /machine:I386 /out:"Test/ma_xb_t.exe" /subsystem:xbox /fixed:no /tmp /OPT:REF
# SUBTRACT LINK32 /pdb:none
XBE=imagebld.exe
# ADD BASE XBE /nologo /testname:"Mettle Arms RELEASE" /stack:0x10000 /out:"Release/ma_xb_r.xbe" /limitmem /testratings:3
# ADD XBE /nologo /testname:"Mettle Arms TEST" /stack:0x10000 /out:"Test/ma_xb_t.xbe" /limitmem /testratings:3
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO

!ELSEIF  "$(CFG)" == "ma_xb - Xbox Production"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ma_xb___Xbox_Production"
# PROP BASE Intermediate_Dir "ma_xb___Xbox_Production"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Production"
# PROP Intermediate_Dir "Production"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gi /GX /O2 /Ob2 /D "NDEBUG" /D "WIN32" /D "_XBOX" /D "_FANGDEF_PLATFORM_XB" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /Gi /GX /O2 /Ob2 /I "..\..\..\lib\SmallAmx" /I "..\\" /D "NDEBUG" /D "WIN32" /D "_XBOX" /D "_FANGDEF_PLATFORM_XB" /D "_FANGDEF_PRODUCTION_BUILD" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /machine:I386 /out:"Release/ma_xb_r.exe" /subsystem:xbox /fixed:no /tmp /OPT:REF
# SUBTRACT BASE LINK32 /incremental:yes
# ADD LINK32 xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /machine:I386 /out:"Production/ma_xb_p.exe" /subsystem:xbox /fixed:no /tmp /OPT:REF
# SUBTRACT LINK32 /pdb:none
XBE=imagebld.exe
# ADD BASE XBE /nologo /testname:"Mettle Arms RELEASE" /stack:0x10000 /out:"Release/ma_xb_r.xbe" /limitmem /testratings:3
# ADD XBE /nologo /testname:"Mettle Arms MS12 PRODUCTION" /stack:0x10000 /out:"Production/ma_xb_p.xbe" /limitmem /testratings:3
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO

!ENDIF 

# Begin Target

# Name "ma_xb - Xbox Release"
# Name "ma_xb - Xbox Debug"
# Name "ma_xb - Xbox Test"
# Name "ma_xb - Xbox Production"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "AI Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Ai\AI.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AI3DBotMover.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBotMover.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBrain.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBrainman.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBrainReact.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBrainUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBTATable.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIControlGoals.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIEdgeLock.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIEnviro.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIFormations.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIFSM.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIGameUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIGraph.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AiGraphSearcher.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\aigroup.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIHazard.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIKnowledge.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIMain.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIMover.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIPath.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIPatrolPath.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIRooms.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIThought.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIThoughtBiped.cpp
# End Source File
# Begin Source File

SOURCE=..\Ai\AIWeaponCtrl.cpp
# End Source File
# End Group
# Begin Group "Entity Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\bot.cpp
# End Source File
# Begin Source File

SOURCE=..\bot_data.cpp
# End Source File
# Begin Source File

SOURCE=..\botanim.cpp
# End Source File
# Begin Source File

SOURCE=..\botblink.cpp
# End Source File
# Begin Source File

SOURCE=..\botblink_data.cpp
# End Source File
# Begin Source File

SOURCE=..\botgrunt.cpp
# End Source File
# Begin Source File

SOURCE=..\botgrunt_data.cpp
# End Source File
# Begin Source File

SOURCE=..\botpred.cpp
# End Source File
# Begin Source File

SOURCE=..\botpred_data.cpp
# End Source File
# Begin Source File

SOURCE=..\bottitan.cpp
# End Source File
# Begin Source File

SOURCE=..\bottitan_data.cpp
# End Source File
# Begin Source File

SOURCE=..\DestructEntity.cpp
# End Source File
# Begin Source File

SOURCE=..\Door.cpp
# End Source File
# Begin Source File

SOURCE=..\ebox.cpp
# End Source File
# Begin Source File

SOURCE=..\econsole.cpp
# End Source File
# Begin Source File

SOURCE=..\eline.cpp
# End Source File
# Begin Source File

SOURCE=..\entity.cpp
# End Source File
# Begin Source File

SOURCE=..\entitycontrol.cpp
# End Source File
# Begin Source File

SOURCE=..\EParticle.cpp
# End Source File
# Begin Source File

SOURCE=..\epoint.cpp
# End Source File
# Begin Source File

SOURCE=..\eproj.cpp
# End Source File
# Begin Source File

SOURCE=..\eproj_arrow.cpp
# End Source File
# Begin Source File

SOURCE=..\eproj_cleaner.cpp
# End Source File
# Begin Source File

SOURCE=..\eproj_grenade.cpp
# End Source File
# Begin Source File

SOURCE=..\eproj_linear.cpp
# End Source File
# Begin Source File

SOURCE=..\eproj_saw.cpp
# End Source File
# Begin Source File

SOURCE=..\eproj_swarmer.cpp
# End Source File
# Begin Source File

SOURCE=..\esphere.cpp
# End Source File
# Begin Source File

SOURCE=..\espline.cpp
# End Source File
# Begin Source File

SOURCE=..\ESwitch.cpp
# End Source File
# Begin Source File

SOURCE=..\meshentity.cpp
# End Source File
# Begin Source File

SOURCE=..\Tripwire.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_blaster.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_chaingun.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_cleaner.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_flamer.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_gren.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_hand.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_laser.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_mortar.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_ripper.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_rivet.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_rocket.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_scope.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_spew.cpp
# End Source File
# Begin Source File

SOURCE=..\weapon_tether.cpp
# End Source File
# Begin Source File

SOURCE=..\ZipLine.cpp
# End Source File
# End Group
# Begin Group "User Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\user_albert.cpp
# End Source File
# Begin Source File

SOURCE=..\user_john.cpp
# End Source File
# Begin Source File

SOURCE=..\user_justin.cpp
# End Source File
# Begin Source File

SOURCE=..\user_mike.cpp
# End Source File
# Begin Source File

SOURCE=..\user_pat.cpp
# End Source File
# Begin Source File

SOURCE=..\user_steve.cpp
# End Source File
# End Group
# Begin Group "BotTalk Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\BotTalkAction.cpp
# End Source File
# Begin Source File

SOURCE=..\BotTalkData.cpp
# End Source File
# Begin Source File

SOURCE=..\BotTalkInst.cpp
# End Source File
# Begin Source File

SOURCE=..\TalkSystem2.cpp
# End Source File
# End Group
# Begin Group "Barter Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\BarterSound.cpp
# End Source File
# Begin Source File

SOURCE=..\BarterSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\BarterTypes.cpp
# End Source File
# Begin Source File

SOURCE=..\CamBarter.cpp
# End Source File
# Begin Source File

SOURCE=..\Shady.cpp
# End Source File
# Begin Source File

SOURCE=..\ShadyAnim.cpp
# End Source File
# Begin Source File

SOURCE=..\Slim.cpp
# End Source File
# Begin Source File

SOURCE=..\SSTable.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\Actor.cpp
# End Source File
# Begin Source File

SOURCE=..\BlinkGlow.cpp
# End Source File
# Begin Source File

SOURCE=..\BlinkShell.cpp
# End Source File
# Begin Source File

SOURCE=..\BlinkSpeed.cpp
# End Source File
# Begin Source File

SOURCE=..\BotFx.cpp
# End Source File
# Begin Source File

SOURCE=..\CamBot.cpp
# End Source File
# Begin Source File

SOURCE=..\CamDebug.cpp
# End Source File
# Begin Source File

SOURCE=..\CamManual.cpp
# End Source File
# Begin Source File

SOURCE=..\CamSimple.cpp
# End Source File
# Begin Source File

SOURCE=..\DamageSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\debris.cpp
# End Source File
# Begin Source File

SOURCE=..\EGoodie.cpp
# End Source File
# Begin Source File

SOURCE=..\explosion.cpp
# End Source File
# Begin Source File

SOURCE=..\flamer.cpp
# End Source File
# Begin Source File

SOURCE=..\FXStreamer.cpp
# End Source File
# Begin Source File

SOURCE=..\game.cpp
# End Source File
# Begin Source File

SOURCE=..\gamecam.cpp
# End Source File
# Begin Source File

SOURCE=..\GameError.cpp
# End Source File
# Begin Source File

SOURCE=..\gameloop.cpp
# End Source File
# Begin Source File

SOURCE=..\gamepad.cpp
# End Source File
# Begin Source File

SOURCE=..\GamePools.cpp
# End Source File
# Begin Source File

SOURCE=..\gamesave.cpp
# End Source File
# Begin Source File

SOURCE=..\GoodieBag.cpp
# End Source File
# Begin Source File

SOURCE=..\GoodieProps.cpp
# End Source File
# Begin Source File

SOURCE=..\gstring.cpp
# End Source File
# Begin Source File

SOURCE=..\guid.cpp
# End Source File
# Begin Source File

SOURCE=..\Hud2.cpp
# End Source File
# Begin Source File

SOURCE=..\HudWeaponProfile.cpp
# End Source File
# Begin Source File

SOURCE=..\Item.cpp
# End Source File
# Begin Source File

SOURCE=..\ItemInst.cpp
# End Source File
# Begin Source File

SOURCE=..\ItemRepository.cpp
# End Source File
# Begin Source File

SOURCE=..\LaserBeam.cpp
# End Source File
# Begin Source File

SOURCE=..\letterbox.cpp
# End Source File
# Begin Source File

SOURCE=..\level.cpp
# End Source File
# Begin Source File

SOURCE=..\leveltest.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=..\MAScriptTypes.cpp
# End Source File
# Begin Source File

SOURCE=..\MenuTypes.cpp
# End Source File
# Begin Source File

SOURCE=..\MuzzleFlash.cpp
# End Source File
# Begin Source File

SOURCE=..\PauseScreen.cpp
# End Source File
# Begin Source File

SOURCE=..\PickLevel.cpp
# End Source File
# Begin Source File

SOURCE=..\player.cpp
# End Source File
# Begin Source File

SOURCE=..\potmark.cpp
# End Source File
# Begin Source File

SOURCE=..\ProTrack.cpp
# End Source File
# Begin Source File

SOURCE=..\PsPool.cpp
# End Source File
# Begin Source File

SOURCE=..\reticle.cpp
# End Source File
# Begin Source File

SOURCE=..\RoboBuddy.cpp
# End Source File
# Begin Source File

SOURCE=..\save.cpp
# End Source File
# Begin Source File

SOURCE=..\skybox.cpp
# End Source File
# Begin Source File

SOURCE=..\smoketrail.cpp
# End Source File
# Begin Source File

SOURCE=..\SpaceDock.cpp
# End Source File
# Begin Source File

SOURCE=..\tether.cpp
# End Source File
# Begin Source File

SOURCE=..\tracer.cpp
# End Source File
# Begin Source File

SOURCE=..\VtxPool.cpp
# End Source File
# Begin Source File

SOURCE=..\Workable.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Ai Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Ai\AI.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AI3DBotMover.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBotMover.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBrain.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBrainMan.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIBrainUtils.h
# End Source File
# Begin Source File

SOURCE=..\Ai\aiBuilder.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIControlGoals.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIEdgeLock.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIEnviro.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIFormations.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIGameUtils.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIGraph.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIGraphSearcher.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIHazard.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIKnowledge.h
# End Source File
# Begin Source File

SOURCE=..\Ai\aimain.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIMath.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIMover.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIMover.inl
# End Source File
# Begin Source File

SOURCE=..\Ai\AINodePools.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIPath.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIPath.inl
# End Source File
# Begin Source File

SOURCE=..\Ai\AIPatrolPath.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIRooms.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIThought.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIThoughtBiped.h
# End Source File
# Begin Source File

SOURCE=..\Ai\AIWeaponCtrl.h
# End Source File
# Begin Source File

SOURCE=..\Ai\apenew.h
# End Source File
# Begin Source File

SOURCE=..\Ai\ftl.h
# End Source File
# End Group
# Begin Group "Entity Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\bot.h
# End Source File
# Begin Source File

SOURCE=..\botanim.h
# End Source File
# Begin Source File

SOURCE=..\botblink.h
# End Source File
# Begin Source File

SOURCE=..\botgrunt.h
# End Source File
# Begin Source File

SOURCE=..\botpred.h
# End Source File
# Begin Source File

SOURCE=..\bottitan.h
# End Source File
# Begin Source File

SOURCE=..\DestructEntity.h
# End Source File
# Begin Source File

SOURCE=..\Door.h
# End Source File
# Begin Source File

SOURCE=..\ebox.h
# End Source File
# Begin Source File

SOURCE=..\econsole.h
# End Source File
# Begin Source File

SOURCE=..\eline.h
# End Source File
# Begin Source File

SOURCE=..\entity.h
# End Source File
# Begin Source File

SOURCE=..\entitycontrol.h
# End Source File
# Begin Source File

SOURCE=..\EParticle.h
# End Source File
# Begin Source File

SOURCE=..\epoint.h
# End Source File
# Begin Source File

SOURCE=..\eproj.h
# End Source File
# Begin Source File

SOURCE=..\eproj_arrow.h
# End Source File
# Begin Source File

SOURCE=..\eproj_cleaner.h
# End Source File
# Begin Source File

SOURCE=..\eproj_grenade.h
# End Source File
# Begin Source File

SOURCE=..\eproj_linear.h
# End Source File
# Begin Source File

SOURCE=..\eproj_saw.h
# End Source File
# Begin Source File

SOURCE=..\eproj_swarmer.h
# End Source File
# Begin Source File

SOURCE=..\esphere.h
# End Source File
# Begin Source File

SOURCE=..\espline.h
# End Source File
# Begin Source File

SOURCE=..\ESwitch.h
# End Source File
# Begin Source File

SOURCE=..\meshentity.h
# End Source File
# Begin Source File

SOURCE=..\weapon.h
# End Source File
# Begin Source File

SOURCE=..\weapon_blaster.h
# End Source File
# Begin Source File

SOURCE=..\weapon_chaingun.h
# End Source File
# Begin Source File

SOURCE=..\weapon_cleaner.h
# End Source File
# Begin Source File

SOURCE=..\weapon_flamer.h
# End Source File
# Begin Source File

SOURCE=..\weapon_gren.h
# End Source File
# Begin Source File

SOURCE=..\weapon_hand.h
# End Source File
# Begin Source File

SOURCE=..\weapon_laser.h
# End Source File
# Begin Source File

SOURCE=..\weapon_mortar.h
# End Source File
# Begin Source File

SOURCE=..\weapon_ripper.h
# End Source File
# Begin Source File

SOURCE=..\weapon_rivet.h
# End Source File
# Begin Source File

SOURCE=..\weapon_rocket.h
# End Source File
# Begin Source File

SOURCE=..\weapon_scope.h
# End Source File
# Begin Source File

SOURCE=..\weapon_spew.h
# End Source File
# Begin Source File

SOURCE=..\weapon_tether.h
# End Source File
# Begin Source File

SOURCE=..\weapons.h
# End Source File
# Begin Source File

SOURCE=..\ZipLine.h
# End Source File
# End Group
# Begin Group "User Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\sas_user.h
# End Source File
# Begin Source File

SOURCE=..\user_albert.h
# End Source File
# Begin Source File

SOURCE=..\user_john.h
# End Source File
# Begin Source File

SOURCE=..\user_justin.h
# End Source File
# Begin Source File

SOURCE=..\user_mike.h
# End Source File
# Begin Source File

SOURCE=..\user_pat.h
# End Source File
# Begin Source File

SOURCE=..\user_steve.h
# End Source File
# End Group
# Begin Group "BotTalk Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\BotTalkAction.h
# End Source File
# Begin Source File

SOURCE=..\BotTalkCsvFlags.h
# End Source File
# Begin Source File

SOURCE=..\BotTalkData.h
# End Source File
# Begin Source File

SOURCE=..\BotTalkInst.h
# End Source File
# Begin Source File

SOURCE=..\TalkSystem2.h
# End Source File
# End Group
# Begin Group "Barter Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\BarterSound.h
# End Source File
# Begin Source File

SOURCE=..\BarterSystem.h
# End Source File
# Begin Source File

SOURCE=..\BarterTypes.h
# End Source File
# Begin Source File

SOURCE=..\CamBarter.h
# End Source File
# Begin Source File

SOURCE=..\Shady.h
# End Source File
# Begin Source File

SOURCE=..\ShadyAnim.h
# End Source File
# Begin Source File

SOURCE=..\Slim.h
# End Source File
# Begin Source File

SOURCE=..\SSTable.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\Actor.h
# End Source File
# Begin Source File

SOURCE=..\BlinkGlow.h
# End Source File
# Begin Source File

SOURCE=..\BlinkShell.h
# End Source File
# Begin Source File

SOURCE=..\BlinkSpeed.h
# End Source File
# Begin Source File

SOURCE=..\BotFx.h
# End Source File
# Begin Source File

SOURCE=..\CamBot.h
# End Source File
# Begin Source File

SOURCE=..\CamDebug.h
# End Source File
# Begin Source File

SOURCE=..\CamManual.h
# End Source File
# Begin Source File

SOURCE=..\CamSimple.h
# End Source File
# Begin Source File

SOURCE=..\DamageSystem.h
# End Source File
# Begin Source File

SOURCE=..\debris.h
# End Source File
# Begin Source File

SOURCE=..\explosion.h
# End Source File
# Begin Source File

SOURCE=..\flamer.h
# End Source File
# Begin Source File

SOURCE=..\FXStreamer.h
# End Source File
# Begin Source File

SOURCE=..\game.h
# End Source File
# Begin Source File

SOURCE=..\gamecam.h
# End Source File
# Begin Source File

SOURCE=..\gameloop.h
# End Source File
# Begin Source File

SOURCE=..\gamepad.h
# End Source File
# Begin Source File

SOURCE=..\GamePools.h
# End Source File
# Begin Source File

SOURCE=..\gamesave.h
# End Source File
# Begin Source File

SOURCE=..\GoodieBag.h
# End Source File
# Begin Source File

SOURCE=..\GoodieProps.h
# End Source File
# Begin Source File

SOURCE=..\gstring.h
# End Source File
# Begin Source File

SOURCE=..\guid.h
# End Source File
# Begin Source File

SOURCE=..\Hud2.h
# End Source File
# Begin Source File

SOURCE=..\HudWeaponProfile.h
# End Source File
# Begin Source File

SOURCE=..\Item.h
# End Source File
# Begin Source File

SOURCE=..\ItemInst.h
# End Source File
# Begin Source File

SOURCE=..\ItemRepository.h
# End Source File
# Begin Source File

SOURCE=..\LaserBeam.h
# End Source File
# Begin Source File

SOURCE=..\letterbox.h
# End Source File
# Begin Source File

SOURCE=..\level.h
# End Source File
# Begin Source File

SOURCE=..\leveltest.h
# End Source File
# Begin Source File

SOURCE=..\MAScriptTypes.h
# End Source File
# Begin Source File

SOURCE=..\MenuTypes.h
# End Source File
# Begin Source File

SOURCE=..\MeshTypes.h
# End Source File
# Begin Source File

SOURCE=..\MuzzleFlash.h
# End Source File
# Begin Source File

SOURCE=..\PauseScreen.h
# End Source File
# Begin Source File

SOURCE=..\PickLevel.h
# End Source File
# Begin Source File

SOURCE=..\player.h
# End Source File
# Begin Source File

SOURCE=..\potmark.h
# End Source File
# Begin Source File

SOURCE=..\ProTrack.h
# End Source File
# Begin Source File

SOURCE=..\PsPool.h
# End Source File
# Begin Source File

SOURCE=..\reticle.h
# End Source File
# Begin Source File

SOURCE=..\RoboBuddy.h
# End Source File
# Begin Source File

SOURCE=..\save.h
# End Source File
# Begin Source File

SOURCE=..\skybox.h
# End Source File
# Begin Source File

SOURCE=..\smoketrail.h
# End Source File
# Begin Source File

SOURCE=..\tether.h
# End Source File
# Begin Source File

SOURCE=..\tracer.h
# End Source File
# Begin Source File

SOURCE=..\TripWire.h
# End Source File
# Begin Source File

SOURCE=..\VtxPool.h
# End Source File
# Begin Source File

SOURCE=..\Workable.h
# End Source File
# End Group
# End Target
# End Project

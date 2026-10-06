"""Offline PvP join/controller handoff checks; no game, device or profile access."""
from pathlib import Path
import subprocess
from test_coop_checkpoint_rat import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-pvp-join-routing"
FIXTURE = r'''
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <cstdlib>
#include "pc_pad_routing.h"
using BOOL=int;using u32=unsigned;using s32=int;using LONG=long;using DWORD=unsigned;
constexpr BOOL TRUE=1,FALSE=0;constexpr unsigned MAX_PLAYERS=4,FPADIO_MAX_DEVICES=4;
#define MA_PC_INPUT 1
#define FASSERT(x) ((void)0)
#define FASSERT_NOW ((void)0)
enum PcInputLayout {PCINPUT_LAYOUT_SHARED,PCINPUT_LAYOUT_SEPARATE,PCINPUT_LAYOUT_AUTO};
PcInputLayout s_layout=PCINPUT_LAYOUT_SHARED,s_localCoopLayout=PCINPUT_LAYOUT_SHARED;
bool s_localCoopSession=false,s_padLockReady=true,s_explicitLocalJoin=false;int s_padLock;
PcSessionPadRouting s_joinRouting,s_soloRouting;
LONG s_soloPortPad[4]={},s_blockedPadButtons[4]={},s_portPromptsForPad[4]={},s_sampledPortPad[4]={};
LONG s_autoPlayers=4,s_autoPortPad[4]={},s_connected[4]={},s_autoDealt=0,s_padAssignmentSerial=0;
DWORD s_lastProbe[4]={},now=1000;constexpr DWORD XINPUT_REPROBE_MS=250,ERROR_SUCCESS=0;
struct XINPUT_GAMEPAD{unsigned wButtons=0,bLeftTrigger=0,bRightTrigger=0;int sThumbLX=0,sThumbLY=0,sThumbRX=0,sThumbRY=0;};
struct XINPUT_STATE {XINPUT_GAMEPAD Gamepad;};unsigned physicalPads=0;
constexpr int XINPUT_GAMEPAD_A=4096,XINPUT_GAMEPAD_TRIGGER_THRESHOLD=30,XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE=7849,XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE=8689;
unsigned physicalButtons[4]={};
DWORD fakeGetState(u32 pad,XINPUT_STATE*state){state->Gamepad.wButtons=physicalButtons[pad];return (physicalPads&(1u<<pad))?0:1;}
auto s_getState=fakeGetState;
DWORD GetTickCount(){return now;}LONG InterlockedCompareExchange(volatile LONG*p,LONG n,LONG old){LONG result=*p;if(result==old)*p=n;return result;}
LONG InterlockedExchange(volatile LONG*p,LONG n){LONG old=*p;*p=n;return old;}LONG InterlockedIncrement(volatile LONG*p){return ++*p;}
void EnterCriticalSection(int*){}void LeaveCriticalSection(int*){}
void pcinput_SetLocalCoopSession(bool,PcInputLayout=PCINPUT_LAYOUT_AUTO,u32=2);
PcInputLayout pcinput_Layout();
void pcinput_BeginLocalJoin(u32);
void pcinput_FinishLocalJoin(u32);
BOOL _bPcCoopJoin=FALSE,_bPcCoopLaunch=FALSE;float _fPcCoopNeedPlayersTimer=0;
enum Wpr_DataTypes_NavCode_e {WPR_DATATYPES_NAV_CODE_BACK,WPR_DATATYPES_NAV_CODE_FORWARD};
enum {WPR_DATATYPES_MODES_MAIN_MENU,WPR_DATATYPES_MODES_MULTIPLAYER,WPR_DATATYPES_SCREENS_MAIN_MENU,
 WPR_DATATYPES_SCREENS_MULTI_JOIN,WPR_DATATYPES_SCREENS_MULTI_TYPE,WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN,
 _MENU_ITEMS_MM_COOP,_MENU_ITEMS_MM_MULTI,WPR_DATATYPES_SOUNDS_SUCCESS,_MULTI_JOIN_STATE_ENTER};
struct Profile {u32 m_nControllerIndex=99;};Profile _paProfiles[4];
struct _JoinInfo_t {int nDefaultKey=8;BOOL bWaitingForOtherPlayers=FALSE;};
struct _MultiJoin_SectionData_t {int nState=0,nProfileKey=0,nTeamID=0;Profile*pProfile=nullptr;BOOL bWorkDoneThisFrame=FALSE;};
struct Menu {int nMode=0,nCurItemIndex=0,nCurrentScreen=0,nLastScreen=0,nControllerIndex=0,nMTMode=0,nMTType=0,nMRTmpRuleType=0;
 unsigned nMPPlayerMask=3;float fModeTimer=0;_JoinInfo_t MJJoinInfo;_MultiJoin_SectionData_t aMJSections[4];} _MenuState;
void _MJ_UpdateJoinInfo(_JoinInfo_t*,BOOL){}int _ahSounds[1]={};void fsndfx_Play2D(int){}
int campaignMenus=0;void _PcCoopEnterCampaignMenu(){campaignMenus++;}
void _MJ_InitJoinInfoAndSections(_JoinInfo_t*,_MultiJoin_SectionData_t*,u32);
int checks=0;void require(bool x,const char*why){checks++;if(!x){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
'''
CHECKS = r'''
int main(){
 for(auto configured:{PCINPUT_LAYOUT_SHARED,PCINPUT_LAYOUT_SEPARATE}){
  s_layout=configured;physicalPads=1;pcinput_SetLocalCoopSession(false);
  if(configured==PCINPUT_LAYOUT_SHARED)require(pcinput_PadForPort(pcinput_Layout(),0)==0,"original default merged first pad into P1");
  for(int cycle=0;cycle<3;cycle++){
   // A real co-op session compacts AUTO to the highest joined port before returning.
   _PcBeginLocalJoinRouting(TRUE);pcinput_SetLocalCoopPlayers(2);
   _bPcCoopLaunch=TRUE;_bPcCoopJoin=FALSE;pcinput_SetLocalCoopSession(false);
   enterPvP();
   require(!_bPcCoopJoin&&!_bPcCoopLaunch,"PvP never inherits campaign mode flags");
   require(pcinput_Layout()==PCINPUT_LAYOUT_AUTO&&s_autoPlayers==4,"PvP starts a fresh four-port deal");
   require(pcinput_PadForPort(pcinput_Layout(),0)==-1&&pcinput_PadForPort(pcinput_Layout(),1)==0,"keyboard P1 and first pad P2 are independent");
   for(unsigned port=0;port<4;port++)require(_MenuState.aMJSections[port].pProfile->m_nControllerIndex==port,"profile binds its own join port");
   require(_MenuState.nCurrentScreen==WPR_DATATYPES_SCREENS_MULTI_JOIN,"PvP reaches retail join screen");
   pcinput_ClaimLocalJoinPort(0);pcinput_ClaimLocalJoinPort(1);
   _MultiJoin_MP_ExitDecisions(WPR_DATATYPES_NAV_CODE_FORWARD);
   require(_MenuState.nCurrentScreen==WPR_DATATYPES_SCREENS_MULTI_TYPE&&!campaignMenus,"PvP advances to rules rather than campaign launch");
   require(pcinput_Layout()==PCINPUT_LAYOUT_AUTO&&pcinput_PadForPort(pcinput_Layout(),1)==0,"keep join routing for rules and PvP gameplay");
   _MultiJoin_MP_ExitDecisions(WPR_DATATYPES_NAV_CODE_BACK);
   require(pcinput_Layout()==configured,"back restores configured solo/main-menu routing");
   require(_MenuState.nCurrentScreen==WPR_DATATYPES_SCREENS_MAIN_MENU,"back reaches main menu");
  }
 }
 for(unsigned mask=0;mask<16;mask++){
  physicalPads=mask;enterPvP();unsigned seen=0,count=0;
  for(unsigned port=0;port<4;port++){
   int pad=pcinput_PadForPort(pcinput_Layout(),port);if(pad<0)continue;
   require((mask&(1u<<pad))&&!(seen&(1u<<pad)),"connected pads route uniquely");seen|=1u<<pad;count++;
  }
  require(seen==mask,"every connected controller has a PvP join port");
  if(count<4)require(pcinput_PadForPort(pcinput_Layout(),0)==-1,"reserve keyboard slot when fewer than four controllers");
  if(count==4)require(pcinput_PadForPort(pcinput_Layout(),0)==3,"spare fourth pad previews keyboard P1 while first three can join partners");
 }
 physicalPads=1;enterPvP();physicalPads=9;now+=300;UpdateAutoPadAssignmentLocked();
 require(pcinput_PadForPort(pcinput_Layout(),1)==0&&pcinput_PadForPort(pcinput_Layout(),2)==3,"late second controller joins without stealing first");
 pcinput_ClaimLocalJoinPort(0);pcinput_ClaimLocalJoinPort(1);pcinput_ClaimLocalJoinPort(2);pcinput_FinishLocalJoin(7);
 physicalPads=8;now+=300;UpdateAutoPadAssignmentLocked();
 require(pcinput_PadForPort(pcinput_Layout(),1)==-1&&pcinput_PadForPort(pcinput_Layout(),2)==3,"disconnect does not move remaining player");
 physicalPads=9;now+=300;UpdateAutoPadAssignmentLocked();
 require(pcinput_PadForPort(pcinput_Layout(),1)==0&&pcinput_PadForPort(pcinput_Layout(),2)==3,"reconnect fills free port");
 _PcBeginLocalJoinRouting(TRUE);_MultiJoin_MP_ExitDecisions(WPR_DATATYPES_NAV_CODE_FORWARD);
 require(campaignMenus==1&&!_bPcCoopJoin,"co-op retains campaign path");
 _PcBeginLocalJoinRouting(TRUE);_MultiJoin_MP_ExitDecisions(WPR_DATATYPES_NAV_CODE_BACK);
 require(pcinput_Layout()==s_layout&&_MenuState.nCurItemIndex==_MENU_ITEMS_MM_COOP,"co-op cancel retains main-menu behavior");

 // Route changes suppress held accept/back until release, then accept a new press.
 s_explicitLocalJoin=true;s_joinRouting.Reset(true,-1);XINPUT_GAMEPAD buttons;buttons.wButtons=XINPUT_GAMEPAD_A;
 MaskRoutingButtons(1,0,&buttons);require(!buttons.wButtons,"new join route cannot reuse a held accept");
 buttons.wButtons=XINPUT_GAMEPAD_A;MaskRoutingButtons(1,0,&buttons);require(!buttons.wButtons,"held accept remains suppressed");
 buttons.wButtons=0;MaskRoutingButtons(1,0,&buttons);buttons.wButtons=XINPUT_GAMEPAD_A;MaskRoutingButtons(1,0,&buttons);require(buttons.wButtons==XINPUT_GAMEPAD_A,"release then fresh accept joins normally");
 buttons.wButtons=8192;MaskRoutingButtons(2,0,&buttons);require(!buttons.wButtons,"canceled device cannot replay back onto another join row");
 s_joinRouting.Finish(3);buttons.wButtons=XINPUT_GAMEPAD_A;MaskRoutingButtons(0,3,&buttons);require(buttons.wButtons==XINPUT_GAMEPAD_A,"in-game hot swap does not eat a fresh gameplay button");
 // Every XInput index can open selection as P1; keyboard stays on game port zero.
 s_layout=PCINPUT_LAYOUT_SHARED;
 for(unsigned pad=0;pad<4;++pad){
  physicalPads=1u<<pad;now+=300;pcinput_SetLocalCoopSession(false);
  s_portPromptsForPad[0]=1;pcinput_BeginLocalJoin(0);
  require(pcinput_PadForPort(pcinput_Layout(),0)==int(pad),"controller opening join retains P1 at every XInput slot");
  pcinput_ClaimLocalJoinPort(0);pcinput_FinishLocalJoin(1);
  physicalPads=0;now+=300;UpdateAutoPadAssignmentLocked();
  require(pcinput_PadForPort(pcinput_Layout(),0)==-1,"P1 may fall back to keyboard after unplug");
  physicalPads=1u<<((pad+1)%4);now+=300;UpdateAutoPadAssignmentLocked();
  require(pcinput_PadForPort(pcinput_Layout(),0)==int((pad+1)%4),"hotplug replacement controller returns to P1 without joining another player");
 }
 // An idle connected pad must not prevent P1 choosing another connected pad.
 physicalPads=5;physicalButtons[2]=XINPUT_GAMEPAD_A;now+=300;pcinput_SetLocalCoopSession(false);
 require(pcinput_PadForPort(pcinput_Layout(),0)==2,"solo P1 can use another controller while the first stays connected idle");
 s_portPromptsForPad[0]=1;pcinput_BeginLocalJoin(0);
 require(pcinput_PadForPort(pcinput_Layout(),0)==2&&s_blockedPadButtons[2]==XINPUT_GAMEPAD_A,"opening controller stays P1 and its held menu accept is suppressed");
 physicalButtons[2]=0;
 // Keyboard P1 plus one pad P2, followed by an unclaimed pad for P1.
 physicalPads=1;s_portPromptsForPad[0]=0;pcinput_BeginLocalJoin(0);
 require(s_joinRouting.owned[1]==-1,"connection previews P2 without claiming the device");
 pcinput_ClaimLocalJoinPort(0);pcinput_ClaimLocalJoinPort(1);pcinput_FinishLocalJoin(3);
 require(pcinput_PadForPort(pcinput_Layout(),1)==0&&pcinput_PadForPort(pcinput_Layout(),0)==-1,"keyboard P1 and explicitly joined controller P2 remain supported");
 physicalPads=9;now+=300;UpdateAutoPadAssignmentLocked();
 require(pcinput_PadForPort(pcinput_Layout(),0)==3&&pcinput_PadForPort(pcinput_Layout(),1)==0,"unclaimed hotplug goes to P1 without stealing P2");
 physicalPads=8;now+=300;UpdateAutoPadAssignmentLocked();
 require(pcinput_PadForPort(pcinput_Layout(),1)==-1&&s_joinRouting.owned[1]==0,"disconnected P2 retains device ownership");
 physicalPads=12;now+=300;UpdateAutoPadAssignmentLocked();
 require(pcinput_PadForPort(pcinput_Layout(),1)==-1&&pcinput_PadForPort(pcinput_Layout(),0)==3,"a spare device never silently fills disconnected P2");
 physicalPads=13;now+=300;UpdateAutoPadAssignmentLocked();
 require(pcinput_PadForPort(pcinput_Layout(),1)==0&&pcinput_PadForPort(pcinput_Layout(),0)==3,"P2 reconnect restores the same player while P1 stays stable");
 physicalPads=4;now+=300;pcinput_SetLocalCoopSession(false);
 require(pcinput_PadForPort(pcinput_Layout(),0)==2&&pcinput_PadForPort(pcinput_Layout(),1)==-1,"ordinary solo routes any connected slot only to P1");
 for(unsigned mask=0;mask<16;++mask)for(int owner=-1;owner<4;++owner){
  if(owner>=0 && !(mask&(1u<<owner)))continue;
  PcSessionPadRouting routing;routing.Reset(true,owner);routing.Update(mask);unsigned seen=0;
  for(unsigned port=0;port<4;++port){int pad=routing.routed[port];if(pad<0)continue;
   require((mask&(1u<<pad))&&!(seen&(1u<<pad)),"all opening devices route uniquely");seen|=1u<<pad;
  }
  require(seen==mask,"all attached devices remain available to join");
  if(owner>=0)require(routing.routed[0]==owner,"opening controller cannot be displaced by other devices");
  routing.Claim(0);routing.Claim(1);int partner=routing.owned[1];routing.Finish(3);routing.Update(mask);
  require(routing.owned[1]==partner,"launch commits actual partner owner");
  routing.Reset(true,-1);routing.Update(mask);routing.Claim(1);routing.KeepJoined(0);routing.Update(mask);
  require(routing.owned[1]==-1,"canceling a joined section releases its device for fresh selection");
 }
 // Sparse joined ports keep their owners; spare devices can only reach P1.
 for(unsigned selected=1;selected<16;selected+=2){
  PcSessionPadRouting routing;routing.Reset(true,0);routing.Update(15);
  for(unsigned port=0;port<4;++port)if(selected&(1u<<port))routing.Claim(port);
  routing.Finish(selected);routing.Update(15);
  for(unsigned port=1;port<4;++port)require(routing.routed[port]==((selected&(1u<<port))?int(port):-1),"unselected and sparse partner slots never become gameplay players");
 }
 std::printf("PASS: %d offline PvP/co-op join routing checks; game not launched.\n",checks);
}
'''

def main():
    source = (ROOT / "port/pc_input.cpp").read_text()
    wrapper = (ROOT / "ma/App/ma/wpr_system.cpp").read_text()
    code = FIXTURE
    for signature in ("static void UpdateAutoPadAssignmentLocked() {", "int pcinput_PadForPort(",
                      "PcInputLayout pcinput_Layout()", "void pcinput_SetLocalCoopSession(",
                      "void pcinput_SetLocalCoopPlayers(","void pcinput_BeginLocalJoin(","void pcinput_ClaimLocalJoinPort(","void pcinput_KeepLocalJoinPlayers(","void pcinput_FinishLocalJoin(","static void MaskRoutingButtons("):
        code += method(source, signature) + "\n"
    for signature in ("static void _PcBeginLocalJoinRouting(",
                      "static void _MJ_InitJoinInfoAndSections( _JoinInfo_t *pJoinInfo,",
                      "static void _MultiJoin_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {"):
        # Skip the multiline declaration before the actual definition.
        if "_MJ_Init" in signature:
            start = wrapper.index(signature, wrapper.index("static void _MultiJoin_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {"))
            code += method(wrapper[start:], signature) + "\n"
        else:
            code += method(wrapper, signature) + "\n"
    menu = method(wrapper, "static void _MainMenu_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {")
    case = menu.split("case _MENU_ITEMS_MM_MULTI:", 1)[1].split("break;", 1)[0]
    code += "void enterPvP(){" + case + "}\n"
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "join.cpp").write_text(code + CHECKS)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(pvp_join_routing LANGUAGES CXX)\n"
        "add_executable(pvp_join_routing join.cpp)\ntarget_compile_features(pvp_join_routing PRIVATE cxx_std_17)\n"
        + "target_include_directories(pvp_join_routing PRIVATE \""+str(ROOT/"port").replace("\\","/")+"\")\n")
    for command in (["cmake", "-S", str(OUT), "-B", str(OUT / "out"), "-A", "Win32"],
                    ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / "out/Release/pvp_join_routing.exe")], check=True)

if __name__ == "__main__":
    main()

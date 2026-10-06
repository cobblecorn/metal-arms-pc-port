"""Offline production pause-confirmation checks; never launches the game."""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-checkpoint-confirmation"


def block(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    end, depth = brace + 1, 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


PRELUDE = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
using u32=unsigned;using s32=int;using BOOL=int;using BOOL8=unsigned char;
constexpr int FALSE=0,TRUE=1;
#define FANG_WINGC 1
#define DEVPRINTF(...) std::snprintf(lastLog,sizeof(lastLog),__VA_ARGS__)
#define FASSERT(x) do{if(!(x))std::abort();}while(0)
#define FASSERT_NOW std::abort()
char lastLog[1024]{};
int checks=0,restores=0,lastSlot=-1,menuWork=0,movies=0,stops=0,saves=0;
bool checkpoint=true;
void require(bool ok,const char* why){checks++;if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
struct Screen{};
struct CMsgBox{
 enum Button_e{BUTTON_NONE,BUTTON_ACCEPT,BUTTON_CANCEL,BUTTON_ALTERNATE};
 static inline bool active=false;
 static inline Button_e button=BUTTON_NONE;
 static inline u32 context=0,displays=0;
 static BOOL IsActive(){return active;}
 static Button_e CheckForButtonPress(){return button;}
 static u32 GetContextData(){return context;}
 static void ClearButtonPress(){button=BUTTON_NONE;}
 template<class... Args>static void Display(const char*,int,int,int,int,void*,u32 ctx,Args...){
  context=ctx;active=true;button=BUTTON_NONE;displays++;
 }
};
enum{_MSGBOX_DATA_NONE,_MSGBOX_DATA_EXIT,_MSGBOX_DATA_RESTART,_MSGBOX_DATA_RESPAWN};
enum{PSSTATE_INACTIVE,PSSTATE_NORMAL};
enum{GAMEPHRASE_WARNING,GAMEPHRASE_GAME_WONT_BE_SAVED,GAMEPHRASE_ACCEPT,GAMEPHRASE_CANCEL};
int Game_apwszPhrases[4]{};
#define _LOSEDATA_MSGBOX "LoseDataWarning"
struct MenuMgr{Screen* m_pCurMS=nullptr;void GetControls(){};void Work(){menuWork++;}};
struct CPlayer{static inline int m_nPlayerCount=2;void* m_pPlayerProfile=nullptr;
 void UpdateProfileUserSettings(){};};
CPlayer Player_aPlayer[4];
void wpr_system_IG_SaveGame(void*){saves++;}
struct CFAudioEmitter{static void StopAll(){stops++;}};
BOOL checkpoint_Saved(int){return checkpoint;}
void checkpoint_SetUnsaved(int){checkpoint=false;}
BOOL checkpoint_Restore(int slot,BOOL,const char* reason){restores++;lastSlot=slot;
 require(std::strstr(reason,"pause:")==reason,"manual restore names its caller");return TRUE;}
void level_PlayIntroMovie(){movies++;}
struct CPauseScreen{
 static inline int m_eState=PSSTATE_INACTIVE;
 static inline BOOL8 m_bMsgBoxActive=FALSE;
 static inline bool m_bQuitNextFrame=false;
 static inline u32 m_uMsgBoxContext=_MSGBOX_DATA_NONE,m_nPlayer=0;
 static inline Screen m_aMS[1];static inline MenuMgr m_MenuMgr;
 static void ShowConfirmation(u32);static BOOL TakeConfirmation(u32&);
 static void GetControls(){}
 static BOOL ExitPause(){m_eState=PSSTATE_INACTIVE;m_bMsgBoxActive=FALSE;m_uMsgBoxContext=_MSGBOX_DATA_NONE;return TRUE;}
 static void Work();
};
void reset(int player){
 CPauseScreen::m_nPlayer=player;CPauseScreen::m_eState=PSSTATE_NORMAL;
 CPauseScreen::m_bMsgBoxActive=false;CPauseScreen::m_uMsgBoxContext=_MSGBOX_DATA_NONE;
 CPauseScreen::m_bQuitNextFrame=false;CPauseScreen::m_MenuMgr.m_pCurMS=&CPauseScreen::m_aMS[0];
 CMsgBox::active=false;CMsgBox::button=CMsgBox::BUTTON_NONE;CMsgBox::context=0;
 restores=menuWork=movies=stops=saves=0;lastSlot=-1;checkpoint=true;
}
'''

CHECKS = r'''
int main(){
 for(int player=0;player<4;player++){
  reset(player);CPauseScreen::m_eState=PSSTATE_INACTIVE;
  CMsgBox::context=_MSGBOX_DATA_RESPAWN;CMsgBox::button=CMsgBox::BUTTON_ACCEPT;
  for(int i=0;i<120;i++)CPauseScreen::Work();
  require(restores==0&&menuWork==0,"uninterrupted movement cannot process a stale menu accept");
  auto displays=CMsgBox::displays;CPauseScreen::ShowConfirmation(_MSGBOX_DATA_RESPAWN);
  require(CMsgBox::displays==displays,"inactive menu cannot open a confirmation");
  for(u32 action:{_MSGBOX_DATA_EXIT,_MSGBOX_DATA_RESTART,_MSGBOX_DATA_RESPAWN}){
   reset(player);CPauseScreen::ShowConfirmation(action);
   require(CPauseScreen::m_bMsgBoxActive&&CPauseScreen::m_uMsgBoxContext==action,"action owns its confirmation");
   for(int i=0;i<60;i++)CPauseScreen::Work();
   require(restores==0&&menuWork==0&&CPauseScreen::m_bMsgBoxActive,"active confirmation blocks menu processing");
   CMsgBox::active=false;CMsgBox::button=CMsgBox::BUTTON_ACCEPT;
   CPauseScreen::Work();
   require(!CPauseScreen::m_bMsgBoxActive,"accept consumes confirmation ownership");
   require(menuWork==0,"accepted action does not process the same input through menu again");
   require(action==_MSGBOX_DATA_EXIT ? CPauseScreen::m_bQuitNextFrame&&saves==2&&restores==0 :
     restores==1&&lastSlot==(action==_MSGBOX_DATA_RESTART?0:1)&&movies==1&&stops==1,"intended quit/restart/respawn remains functional");
   if(action!=_MSGBOX_DATA_EXIT){for(int i=0;i<120;i++)CPauseScreen::Work();require(restores==1,"respawn cannot repeat after menu closes");}
   else {u32 ctx=0;require(!CPauseScreen::TakeConfirmation(ctx),"quit result cannot be replayed");}
   reset(player);CPauseScreen::ShowConfirmation(action);CMsgBox::active=false;
   CMsgBox::button=CMsgBox::BUTTON_CANCEL;CPauseScreen::Work();
   require(restores==0&&menuWork==0&&!CPauseScreen::m_bMsgBoxActive,"cancel consumes input without performing action");
   reset(player);CPauseScreen::ShowConfirmation(action);CMsgBox::active=false;
   CMsgBox::context=action+100;CMsgBox::button=CMsgBox::BUTTON_ACCEPT;CPauseScreen::Work();
   require(restores==0&&!CPauseScreen::m_bQuitNextFrame&&!CPauseScreen::m_bMsgBoxActive,"unrelated global dialog cannot accept this action");
   reset(player);CPauseScreen::ShowConfirmation(action);CPauseScreen::ExitPause();
   CMsgBox::active=false;CMsgBox::button=CMsgBox::BUTTON_ACCEPT;CPauseScreen::Work();
   require(restores==0,"closed pause session discards outstanding confirmation");
  }
  reset(player);checkpoint=false;CPauseScreen::ShowConfirmation(_MSGBOX_DATA_RESPAWN);
  CMsgBox::active=false;CMsgBox::button=CMsgBox::BUTTON_ACCEPT;CPauseScreen::Work();
  require(restores==1&&lastSlot==0,"respawn before first checkpoint still restores level start");
  reset(player);Screen other;CPauseScreen::m_MenuMgr.m_pCurMS=&other;displays=CMsgBox::displays;
  CPauseScreen::ShowConfirmation(_MSGBOX_DATA_RESPAWN);
  require(CMsgBox::displays==displays,"non-options screen cannot open a respawn confirmation");
 }
 std::printf("PASS: %d production confirmation checks; game not launched.\n",checks);
}
'''


def main():
    source = (ROOT / "ma/App/ma/PauseScreen.cpp").read_text()
    work = block(source, "void CPauseScreen::Work(")
    confirmation = block(work, "if( m_bMsgBoxActive )")
    code = PRELUDE + "\n#include <initializer_list>\n"
    code += block(source, "void CPauseScreen::ShowConfirmation(")
    code += block(source, "BOOL CPauseScreen::TakeConfirmation(")
    code += "void CPauseScreen::Work(){s32 i;if(m_eState==PSSTATE_INACTIVE)return;\n"
    code += confirmation + "\nm_MenuMgr.Work();}\n" + CHECKS
    # Verify lifecycle and the real Work placement, not only the isolated branches.
    for signature in ("BOOL CPauseScreen::Start(", "BOOL CPauseScreen::ExitPause(",
                      "BOOL CPauseScreen::LevelInit(", "void CPauseScreen::LevelUninit("):
        method = block(source, signature)
        assert "m_bMsgBoxActive = FALSE;" in method
        assert "m_uMsgBoxContext = _MSGBOX_DATA_NONE;" in method
    assert work.index("if( m_eState == PSSTATE_INACTIVE )") < work.index("if( m_bMsgBoxActive )")
    assert "if( m_eState == PSSTATE_INACTIVE || m_bMsgBoxActive || CMsgBox::IsActive() ) return;" in work
    callback = block(source, "void CPauseScreen::OptionScreenCallback(")
    assert "m_eState != PSSTATE_NORMAL" in callback and "m_MenuMgr.m_pCurMS != &m_aMS[0]" in callback
    assert "CMsgBox::Display" not in callback
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "confirmation.cpp").write_text(code)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(checkpoint_confirmation LANGUAGES CXX)\n"
        "add_executable(checkpoint_confirmation confirmation.cpp)\n"
        "target_compile_features(checkpoint_confirmation PRIVATE cxx_std_17)\n")
    for command in (["cmake", "-S", str(OUT), "-B", str(OUT / "out"), "-A", "Win32"],
                    ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / "out/Release/checkpoint_confirmation.exe")], check=True)


if __name__ == "__main__":
    main()

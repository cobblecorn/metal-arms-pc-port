"""Offline production-code checks for tap/hold selection and menu texture lifetime.

Does not launch the game, sample physical input, or touch profiles.
"""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-weapon-select-menu"


def block(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth, end = 1, brace + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


PRELUDE = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
using u32=unsigned; using s32=int; using f32=float; using BOOL=int; using BOOL8=unsigned char;
constexpr int TRUE=1,FALSE=0;
#define MA_PC_INPUT 1
#define FASSERT(x) do{if(!(x))std::abort();}while(0)
#define FASSERT_NOW std::abort()
#define FMATH_FABS(x) std::fabs(x)
#define fclib_stricmp _stricmp
int checks=0;
void require(bool ok,const char* why){checks++;if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
enum { HUDMODE_GLITCH,HUDMODE_SLOSH,HUDMODE_KRUNK,HUDMODE_MIL,HUDMODE_MOZER,HUDMODE_OTHER };
enum { WEAPONSELECTSTATE_IDLE,WEAPONSELECTSTATE_SCROLLINGON,WEAPONSELECTSTATE_PAUSED,
       WEAPONSELECTSTATE_SCROLLINGTO,WEAPONSELECTSTATE_SCROLLINGOFF };
constexpr u32 DRAW_WEAPONSELECT=1, QSSTATE_WAITINGFORCLEANSTART=0;
constexpr u32 IREASON_RELOAD=1,ItemInst_uMaxInventoryWeapons=8,CHud2_uNumFlashes=4;
float FLoop_fRealPreviousLoopSecs=1.0f/60.0f,CHud2_fFadeOutAlpha=.5f;
constexpr float fWSScrollingOnTime=.15f,fWSScrollingToTime=.1f,fWSItemOmegaYaw=1;
float afWeaponLE[2]={0,0},afWeaponTE[2]={0,0},fWeaponIconHeight=.1f,fWSSpacing=.01f;
bool _bPauseToSwitchWeapons=false;
enum{FAUDIO_PAUSE_LEVEL_NONE,FAUDIO_PAUSE_LEVEL_1};
void floop_PauseGame(BOOL){}
struct CFAudioEmitter {static void SetGlobalPauseLevel(int){}};
struct Vec{float x=0,y=0,z=0;void Set(float a,float b,float c=0){x=a;y=b;z=c;}};
struct ItemView {Vec m_vecStartUL,m_vecCurUL,m_vecEndUL;void UpdateTextArea(int){}};
struct CItem {const char* m_pszCodeName;};
struct CItemInst {CItem* m_pItemData=nullptr;};
struct CInventory {
 u32 m_auNumWeapons[2]={4,4},m_auCurWeapon[2]={1,1};
 CItemInst m_aoWeapons[2][8]; bool available[2][8]{};
 int reloads[2]{},switches[2]{},attempts=0;
 BOOL (*m_pfcnCallback)(u32,CInventory*,u32,u32)=nullptr;
 BOOL SetCurWeapon(u32 side,u32 idx,BOOL,BOOL noReload){
  attempts++;if(!available[side][idx])return FALSE;
  if(idx!=m_auCurWeapon[side]){m_auCurWeapon[side]=idx;switches[side]++;}
  else if(!noReload&&m_pfcnCallback)m_pfcnCallback(IREASON_RELOAD,this,side,idx);
  return TRUE;
 }
};
BOOL callback(u32 reason,CInventory* inv,u32 side,u32){if(reason==IREASON_RELOAD)inv->reloads[side]++;return TRUE;}
struct CHud2 {
 bool m_bWSEnabled=true,drawEnabled=true,m_bWSScrollCleared=true,m_bAudioPause=false;
 u32 m_eCurHudMode=HUDMODE_GLITCH,m_eQSState=0,m_eWeaponSelectState=WEAPONSELECTSTATE_IDLE;
 int m_nWhichWeaponSelectIsActive=0,m_nWSPendingScroll=0,opens=0;
 u32 m_uButtons=0,m_uButtonsLatched=0,m_uWSInventorySize=4;
 u32 m_auWSCurSelected[2]={1,1},m_auFlashing[2]{};
 float m_afFlashAlpha[2]{},m_fWSTimeCountdown=0,m_fFIAlpha1=0,m_fFIAlpha2=0,m_fCurFIAlpha=0,m_fWSItemTheta=0;
 ItemView m_aWSItem[2][8];
 BOOL DrawFlagsEnabled(u32){return drawEnabled;}
 BOOL PcWeaponSelectTap(u32,CInventory*,BOOL);
 void WorkSelector(CInventory*);
 void SetDestinationsToOffScreen(){} void SetCursToDests(){} void WSBoxSetDest(float){}
 void _AddPendingScroll(int n){m_nWSPendingScroll+=n;}
 BOOL ProcessPendingScrolls(){if(!m_nWSPendingScroll)return FALSE;m_nWSPendingScroll=0;
  m_eWeaponSelectState=WEAPONSELECTSTATE_SCROLLINGTO;m_fWSTimeCountdown=0;return TRUE;}
 BOOL StartWeaponSelect(int,CInventory*,BOOL){opens++;return TRUE;}
};
'''

TEXTURES = r'''
constexpr int FTEX_RESNAME=1,FDRAW_COLORFUNC_DIFFUSETEX_AIAT=1,FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE=1,FDRAW_PRIMTYPE_TRILIST=1;
struct FTexDef_t {int generation;bool alive;};
struct CFTexInst {FTexDef_t* tex=nullptr;enum{FLAG_WRAP_S=1,FLAG_WRAP_T=2,FLAG_WRAP_U=4};
 void SetTexDef(FTexDef_t* t){tex=t;}FTexDef_t* GetTexDef(){return tex;}void ClearFlag(int){}};
FTexDef_t* currentAtlas=nullptr; int loads=0,binds=0,draws=0;
void* fresload_Load(int,const char*){loads++;return currentAtlas;}
void fdraw_SetTexture(CFTexInst* t){require(t->tex&&t->tex->alive&&t->tex==currentAtlas,"menu binds only the live wrapper atlas");binds++;}
void fdraw_Color_SetFunc(int){} void fdraw_Alpha_SetBlendOp(int){}
struct Color{void Set(float,float,float,float){}};
struct FDrawVtx_t {Vec Pos_MS,ST;Color ColorRGBA;};
void fdraw_PrimList(int,FDrawVtx_t* v,int n){for(int i=0;i<n;i++)require(std::isfinite(v[i].Pos_MS.x),"finite menu label vertices");draws++;}
'''

CHECKS = r'''
u32 on=FPAD_LATCH_ON|FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT;
u32 repeat=FPAD_LATCH_ON|FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY;
void tap(_WeaponSelectButton_t& b,int expected){
 require(b.Work(on,.016f)==_WSINPUT_NONE,"press alone never opens a selector");
 require(b.Work(FPAD_LATCH_CHANGED,.04f)==expected,"short release has the expected tap action");
}
int main(){
 for(int player=0;player<4;player++)for(int side=0;side<2;side++){
  _WeaponSelectButton_t b{};tap(b,_WSINPUT_TAP);b.Work(0,.08f);tap(b,_WSINPUT_CYCLE);
  for(int frame=0;frame<120;frame++)require(b.Work(0,.016f)==_WSINPUT_NONE,"movement after double tap has no swap request");
  tap(b,_WSINPUT_TAP);b.Work(0,.31f);tap(b,_WSINPUT_TAP);
  b.Reset(FALSE);require(b.Work(on,1.0f)==_WSINPUT_NONE,"hitch before press cannot bypass hold delay");
  for(int frame=0;frame<17;frame++)require(b.Work(repeat,.016f)==_WSINPUT_NONE,"button repeats cannot bypass hold delay");
  require(b.Work(repeat,.04f)==_WSINPUT_OPEN,"continuous hold opens once after 0.3 seconds");
  for(int frame=0;frame<120;frame++)require(b.Work(repeat,.016f)==_WSINPUT_NONE,"held button cannot repeatedly reopen");
  require(b.Work(0,.016f)==_WSINPUT_NONE,"release of long hold is not a tap");
  tap(b,_WSINPUT_TAP);b.Reset(TRUE);
  for(int frame=0;frame<60;frame++)require(b.Work(repeat,.016f)==_WSINPUT_NONE,"paused or borrowed held input requires a fresh press");
  require(b.Work(0,.016f)==_WSINPUT_NONE,"dropped input release has no reload");tap(b,_WSINPUT_TAP);
  b.Reset(FALSE);require(b.Work(FPAD_LATCH_SPIKED,.016f)==_WSINPUT_TAP,"one-frame spike is a tap");
  require(b.Work(FPAD_LATCH_SPIKED,.08f)==_WSINPUT_CYCLE,"second spike is a double tap");
  tap(b,_WSINPUT_TAP);b.Work(0,-1);tap(b,_WSINPUT_TAP);
 }
 _WeaponSelectButton_t independent[4][2]{};
 tap(independent[0][0],_WSINPUT_TAP);tap(independent[0][1],_WSINPUT_TAP);
 tap(independent[1][0],_WSINPUT_TAP);tap(independent[0][0],_WSINPUT_CYCLE);
 CItem empty[2]={{"Empty Primary"},{"Empty Secondary"}},weapons[3]={{"Rivet"},{"SPEW"},{"EMP"}};
 for(int player=0;player<4;player++)for(u32 side=0;side<2;side++){
  CInventory inv;inv.m_pfcnCallback=callback;CHud2 hud;
  for(int hand=0;hand<2;hand++)for(int idx=0;idx<4;idx++){
   inv.m_aoWeapons[hand][idx].m_pItemData=idx?&weapons[idx-1]:&empty[hand];inv.available[hand][idx]=true;
  }
  require(hud.PcWeaponSelectTap(side,&inv,FALSE)&&inv.reloads[side]==1,"tap invokes the correct hand's retail callback");
  require(hud.m_eWeaponSelectState==WEAPONSELECTSTATE_IDLE&&hud.opens==0,"tap never starts menu or changes control mode");
  require(hud.PcWeaponSelectTap(side,&inv,TRUE)&&inv.m_auCurWeapon[side]==2&&inv.switches[side]==1,"double tap cycles only the requested hand");
  inv.m_auCurWeapon[side]=3;require(hud.PcWeaponSelectTap(side,&inv,TRUE)&&inv.m_auCurWeapon[side]==1,"cycle wraps past empty slot");
  inv.available[side][2]=false;require(hud.PcWeaponSelectTap(side,&inv,TRUE)&&inv.m_auCurWeapon[side]==3,"cycle skips unavailable runtime weapon");
  inv.available[side][1]=false;require(!hud.PcWeaponSelectTap(side,&inv,TRUE)&&inv.m_auCurWeapon[side]==3,"no alternative leaves weapon unchanged");
  require(inv.reloads[side]==1,"failed or wrapped quick cycle never reloads");
  hud.m_bWSEnabled=false;require(!hud.PcWeaponSelectTap(side,&inv,TRUE),"disabled selection cannot cycle");
  hud.m_bWSEnabled=true;hud.drawEnabled=false;require(!hud.PcWeaponSelectTap(side,&inv,FALSE),"hidden selection rejects taps");
  hud.drawEnabled=true;hud.m_eCurHudMode=HUDMODE_MIL;
  require(hud.PcWeaponSelectTap(side,&inv,FALSE)&&inv.reloads[0]>=1,"borrowed bot tap retains primary reload");
  require(!hud.PcWeaponSelectTap(side,&inv,TRUE),"borrowed bot cannot quick-cycle Glitch's inventory");
  require(!hud.PcWeaponSelectTap(2,&inv,FALSE)&&!hud.PcWeaponSelectTap(side,nullptr,TRUE),"invalid input safely rejected");
 }
 for(int side=0;side<2;side++){
  CHud2 hud;CInventory inv;hud.m_nWhichWeaponSelectIsActive=side;
  hud.m_eWeaponSelectState=WEAPONSELECTSTATE_SCROLLINGOFF;
  hud.m_uButtons=auActivateButton[0]|auActivateButton[1];hud.m_uButtonsLatched=JINPUT_MOVEUP;
  for(int frame=0;frame<20;frame++)hud.WorkSelector(&inv);
  require(hud.m_eWeaponSelectState==WEAPONSELECTSTATE_IDLE&&hud.opens==0,"closing selector never reopens either hand from raw held input");
  hud.m_nWhichWeaponSelectIsActive=side;hud.m_eWeaponSelectState=WEAPONSELECTSTATE_SCROLLINGON;
  hud.m_nWSPendingScroll=4;hud.m_uButtons=0;hud.m_uButtonsLatched=JINPUT_MOVEDOWN;
  hud.WorkSelector(&inv);require(hud.m_eWeaponSelectState==WEAPONSELECTSTATE_SCROLLINGOFF&&hud.m_nWSPendingScroll==0,"release closes even while movement queued scrolls");
  hud.m_eWeaponSelectState=WEAPONSELECTSTATE_SCROLLINGTO;hud.m_nWSPendingScroll=5;hud.m_fWSTimeCountdown=0;
  for(int frame=0;frame<30;frame++)hud.WorkSelector(&inv);
  require(hud.m_eWeaponSelectState==WEAPONSELECTSTATE_IDLE,"released in-flight scroll finishes without a movement loop");
 }
 std::vector<FTexDef_t> atlases(40);
 for(int visit=0;visit<40;visit++){
  currentAtlas=&atlases[visit];*currentAtlas={visit,true};float bounds[4];int before=loads;
  _PcMenuArtLabel(_aPcMenuCoop,0,0,TRUE,1,1,&bounds[0],&bounds[1],&bounds[2],&bounds[3]);
  _PcMenuArtLabel(_aPcMenuCloseGame,0,0,TRUE,1,1,&bounds[0],&bounds[1],&bounds[2],&bounds[3]);
  require(loads==before+1,"each wrapper visit loads its own atlas exactly once");
  _PcMenuResetLabelTexture();currentAtlas->alive=false;
  require(!_PcMenuLabelTex.GetTexDef()&&!_bPcMenuLabelTexTried,"wrapper teardown drops cached atlas before resource release");
 }
 currentAtlas=nullptr;float bounds[4];int before=draws;
 _PcMenuArtLabel(_aPcMenuCoop,0,0,TRUE,1,1,&bounds[0],&bounds[1],&bounds[2],&bounds[3]);
 require(draws==before,"missing atlas does not bind or draw");_PcMenuResetLabelTexture();
 atlases[0].alive=true;currentAtlas=&atlases[0];before=loads;
 _PcMenuArtLabel(_aPcMenuCoop,0,0,TRUE,1,1,&bounds[0],&bounds[1],&bounds[2],&bounds[3]);
 require(loads==before+1,"new visit retries atlas after earlier load failure");
 std::printf("PASS: %d offline selection/menu checks; game not launched.\n",checks);
}
'''


def main():
    game = (ROOT / "ma/App/ma/game.cpp").read_text()
    hud = (ROOT / "ma/App/ma/Hud2.cpp").read_text()
    menus = (ROOT / "ma/App/ma/wpr_system.cpp").read_text()
    fpad = (ROOT / "ma/Lib/Fang2/fpad.h").read_text()
    end = fpad.index("} FPad_Latch_e;") + len("} FPad_Latch_e;")
    latches = fpad[fpad.rfind("typedef enum", 0, end):end]
    start = game.index("#define _WEAPONSELECT_HOLD_SECS")
    end = game.index("static _WeaponSelectButton_t", start)
    gesture = game[start:end]
    buttons = hud[hud.index("enum\n{"):hud.index("// =============================================================================================================", hud.index("enum\n{"))]
    activation = hud[hud.index("static const u32 auActivateButton"):]
    activation = activation[:activation.index(";") + 1]
    work = block(hud, "void CHud2::Work(CInventory *pInventory)")
    cancel = work[work.index("#if defined(MA_PC_INPUT)"):work.index("#endif", work.index("#if defined(MA_PC_INPUT)")) + len("#endif")]
    selector = block(work, "switch(m_eWeaponSelectState)")
    art_start = menus.index("#define _PCMENU_SLANT")
    art_end = menus.index("// Draw side (ortho pass)", art_start)
    # Integration guards: teardown and restore must reach the production helpers.
    assert "_PcMenuResetLabelTexture();" in block(menus, "static void _ResetSystem( void ) {")
    teardown = block(menus, "void wpr_system_End( void )")
    assert teardown.index("_ResetSystem();") < teardown.index("fres_ReleaseFrame( _ResFrame )")
    assert "game_PcResetWeaponSelect();" in block((ROOT / "ma/App/ma/gamesave.cpp").read_text(), "static BOOL _checkpoint_Restore(")
    code = "\n".join((PRELUDE, latches, gesture, buttons, activation,
                       block(hud, "BOOL CHud2::PcWeaponSelectTap(")))
    code += "\nvoid CHud2::WorkSelector(CInventory* pInventory){\n" + cancel + "\n" + selector + "}\n"
    code += "\n" + TEXTURES + menus[art_start:art_end] + CHECKS
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "selection_menu.cpp").write_text(code)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(selection_menu_check LANGUAGES CXX)\n"
        "add_executable(selection_menu_check selection_menu.cpp)\ntarget_compile_features(selection_menu_check PRIVATE cxx_std_17)\n")
    for command in (["cmake", "-S", str(OUT), "-B", str(OUT / "out")],
                    ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / "out/Release/selection_menu_check.exe")], check=True)


if __name__ == "__main__":
    main()

"""Production HUD ownership cleanup checks, without a game or user saves."""
from pathlib import Path
import subprocess
from test_collectable_skiplist import method
ROOT=Path(__file__).resolve().parent.parent
OUT=ROOT/'build/test-detpack-hud-lifetime'
BASE=r"""
#include <cstdio>
#include <cstdlib>
using f32=float; using s32=int; using u32=unsigned; using BOOL=int;
#define FANG_WINGC 1
bool Fang_bPortDiag=false;
#define DEVPRINTF(...) ((void)0)
constexpr BOOL TRUE=1,FALSE=0;
#define FASSERT_NOW ((void)0)
struct CHud2 {
 enum IconTimerType_t { ICON_TIMER_TYPE_DETPACK, ICON_TIMER_TYPE_RACE, NUM_ICON_TIMERS };
 enum { DRAW_ICON_TIMER=8, DRAW_HEALTH=16 };
 struct Data {f32* m_pfDrawFloat=nullptr;}m_aIconTimerData[NUM_ICON_TIMERS];
 u32 m_uDrawFlags=DRAW_HEALTH;int m_nCurrentIconTimer=0;
 BOOL SetIconTimerDraw(IconTimerType_t,BOOL,f32*);
 void ClearIconTimerDrawIfOwned(IconTimerType_t,const f32*);
 static CHud2* GetHudForPlayer(int);
};
CHud2 huds[4];CHud2* CHud2::GetHudForPlayer(int n){return &huds[n];}
struct CPlayer {static int m_nPlayerCount;};int CPlayer::m_nPlayerCount;
struct CEDetPackDrop {CEDetPackDrop *m_pMasterDetPack=this;f32 m_fDetonateTimeLeft=20;void _ClearHudTimer();};
int checks=0;void require(bool b,const char* text){++checks;if(!b){printf("FAIL: %s\n",text);exit(1);}}
"""
CHECKS=r"""
int main(){
 for(int players=1;players<=4;++players){CPlayer::m_nPlayerCount=players;
  for(int owner=0;owner<players;++owner){
   for(auto& h:huds)h=CHud2{};
   CEDetPackDrop pack,other,child;child.m_pMasterDetPack=&pack;
   huds[owner].SetIconTimerDraw(CHud2::ICON_TIMER_TYPE_DETPACK,TRUE,&pack.m_fDetonateTimeLeft);
   child._ClearHudTimer();require(huds[owner].m_aIconTimerData[0].m_pfDrawFloat==&pack.m_fDetonateTimeLeft,"child cannot clear master countdown");
   other._ClearHudTimer();require(huds[owner].m_uDrawFlags&8,"different pack cannot clear countdown");
   pack._ClearHudTimer();require(!(huds[owner].m_uDrawFlags&8),"master clears former owner's HUD after ownership changes");
   require(huds[owner].m_uDrawFlags&16,"health HUD survives cleanup");
   pack._ClearHudTimer();require(!(huds[owner].m_uDrawFlags&8),"repeated removal is safe");
   huds[owner].SetIconTimerDraw(CHud2::ICON_TIMER_TYPE_DETPACK,TRUE,&pack.m_fDetonateTimeLeft);
   other.m_fDetonateTimeLeft=10;
   huds[owner].SetIconTimerDraw(CHud2::ICON_TIMER_TYPE_DETPACK,TRUE,&other.m_fDetonateTimeLeft);
   pack._ClearHudTimer();require(huds[owner].m_aIconTimerData[0].m_pfDrawFloat==&other.m_fDetonateTimeLeft && (huds[owner].m_uDrawFlags&8),"removing replaced timer preserves the nearer active pack");
   f32 race=30;huds[owner].SetIconTimerDraw(CHud2::ICON_TIMER_TYPE_RACE,TRUE,&race);
   other._ClearHudTimer();require(huds[owner].m_aIconTimerData[1].m_pfDrawFloat==&race && (huds[owner].m_uDrawFlags&8),"race countdown survives DET cleanup");
  }
 }
 printf("PASS: %d HUD ownership and lifetime checks\n",checks);
}
"""
def main():
 code=BASE+method((ROOT/'ma/App/ma/Hud2.cpp').read_text(),'BOOL CHud2::SetIconTimerDraw(')
 code+=method((ROOT/'ma/App/ma/Hud2.cpp').read_text(),'void CHud2::ClearIconTimerDrawIfOwned(')
 code+=method((ROOT/'ma/App/ma/edetpackdrop.cpp').read_text(),'void CEDetPackDrop::_ClearHudTimer(')+CHECKS
 OUT.mkdir(parents=True,exist_ok=True);(OUT/'checks.cpp').write_text(code)
 (OUT/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(detpack_checks LANGUAGES CXX)\nadd_executable(checks checks.cpp)\ntarget_compile_features(checks PRIVATE cxx_std_17)\n')
 for cmd in [['cmake','-S',str(OUT),'-B',str(OUT/'out'),'-A','Win32'],['cmake','--build',str(OUT/'out'),'--config','Release']]:
  r=subprocess.run(cmd,capture_output=True,text=True)
  if r.returncode:raise SystemExit(r.stdout+r.stderr)
 subprocess.run([str(OUT/'out/Release/checks.exe')],check=True)
if __name__=='__main__':main()

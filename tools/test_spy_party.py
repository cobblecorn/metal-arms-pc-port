"""Compile the production factory HUD/fire/objective helpers for solo and 2-4 players."""
from pathlib import Path
import subprocess
from test_coop_checkpoint_rat import method

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'build/test-spy-party'


def main():
    source = r'''
#include <cstdio>
#include <cstdlib>
using s32=int;using u32=unsigned;using BOOL=int;using cchar=const char;
constexpr BOOL TRUE=1,FALSE=0;
#define FANG_WINGC 1
int checks=0;
void require(bool ok,const char*why){++checks;if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
struct CItemInst{int m_nClipAmmo=0;};
struct CInventory{CItemInst chip;CItemInst*IsItemInInventory(const char*){return &chip;}};
struct CBot{int m_nPossessionPlayerIndex=0;bool fire=false,reticle=true;CInventory*m_pInventory=nullptr;
 void SetBotFlag_DontAllowFire(){fire=true;}void ClearBotFlag_DontAllowFire(){fire=false;}
 void ReticleEnable(BOOL b){reticle=b;}};
struct Manager{bool coop=false;BOOL IsLocalCoop(){return coop;}}MultiplayerMgr;
struct CHud2{bool draw=true,select=true;static CHud2*GetHudForPlayer(int);
 void SetDrawEnabled(BOOL b){draw=b;}void SetWSEnable(BOOL b){select=b;}}huds[4];
CHud2*CHud2::GetHudForPlayer(int n){return &huds[n];}
struct CPlayer{static int m_nPlayerCount;CBot*m_pEntityOrig=nullptr;}Player_aPlayer[4];
int CPlayer::m_nPlayerCount=1;CBot bots[4];CInventory inventories[4];
struct Common{BOOL m_bHudState=FALSE;}common;
struct CSpyVsSpy{static Common*m_pCommonData;static CBot*GetGlitch(){return &bots[0];}
 static void TurnOffHUD(BOOL,BOOL=FALSE,BOOL=FALSE);static void AttackDisable(BOOL);};
Common*CSpyVsSpy::m_pCommonData=&common;
'''
    production = (ROOT / 'ma/App/ma/SpyVsSpy.cpp').read_text()
    for signature in ('void CSpyVsSpy::TurnOffHUD(', 'void CSpyVsSpy::AttackDisable(', 'static CItemInst *_SpyTeamChip('):
        source += method(production, signature)
    source += r'''
int main(){
 for(int n=0;n<4;++n){bots[n].m_nPossessionPlayerIndex=n;bots[n].m_pInventory=&inventories[n];Player_aPlayer[n].m_pEntityOrig=&bots[n];}
 for(bool coop:{false,true})for(int count=1;count<=4;++count){
  MultiplayerMgr.coop=coop;CPlayer::m_nPlayerCount=count;
  for(int n=0;n<4;++n){huds[n]=CHud2();bots[n].fire=false;bots[n].reticle=true;inventories[n].chip.m_nClipAmmo=0;}
  CSpyVsSpy::TurnOffHUD(TRUE);CSpyVsSpy::AttackDisable(TRUE);
  for(int n=0;n<4;++n){bool affected=n==0||(coop&&n<count);
   require(huds[n].draw==!affected&&huds[n].select==!affected,"HUD/weapon selection follows the team only in co-op");
   require(bots[n].fire==affected&&bots[n].reticle==!affected,"fire and reticle restrictions stay within the active mode/team");}
  CSpyVsSpy::TurnOffHUD(FALSE,FALSE,TRUE);CSpyVsSpy::AttackDisable(FALSE);
  for(int n=0;n<4;++n){bool affected=n==0||(coop&&n<count);
   require(huds[n].draw&&!bots[n].fire&&bots[n].reticle,"combat restores HUD and controls");
   require(huds[n].select==!affected,"scripted loadout disables selection for only the applicable players");}
  for(int owner=0;owner<count;++owner){
   inventories[owner].chip.m_nClipAmmo=1;auto*p=_SpyTeamChip();
   require(p->m_nClipAmmo==(coop||owner==0?1:0),"any active co-op partner can supply the objective chip; solo checks P1");
   inventories[owner].chip.m_nClipAmmo=0;
  }
 }
 MultiplayerMgr.coop=true;CPlayer::m_nPlayerCount=4;Player_aPlayer[1].m_pEntityOrig=nullptr;bots[2].m_pInventory=nullptr;
 inventories[3].chip.m_nClipAmmo=1;require(_SpyTeamChip()==&inventories[3].chip,"missing partners/inventories do not hide a later player's chip");
 std::printf("PASS: %d production factory party/mode/objective checks; game not launched.\n",checks);
}
'''
    source = source.replace('#include <cstdlib>', '#include <cstdlib>\n#include <initializer_list>')
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'party.cpp').write_text(source)
    (OUT / 'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(party LANGUAGES CXX)\nadd_executable(party party.cpp)\ntarget_compile_features(party PRIVATE cxx_std_17)\n')
    for command in (['cmake', '-S', str(OUT), '-B', str(OUT / 'out'), '-A', 'Win32'],
                    ['cmake', '--build', str(OUT / 'out'), '--config', 'Release']):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / 'out/Release/party.exe')], check=True)


if __name__ == '__main__':
    main()

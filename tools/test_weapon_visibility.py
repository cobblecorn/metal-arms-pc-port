"""Offline weapon-swap visibility regression. Does not launch the game or access saves."""
from pathlib import Path
import subprocess
from test_coop_checkpoint_rat import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'build/test-weapon-visibility'
FIXTURE = r'''
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
using u32=unsigned; using u8=unsigned char; using BOOL=int;
constexpr BOOL TRUE=1,FALSE=0;
#define MA_PC_INPUT 1
#define FASSERT(x) ((void)0)
#define DEVPRINTF(...) ((void)0)
#define FMATH_CLEARBITMASK(x,b) ((x)&=~(b))
#define FMATH_SETBITMASK(x,b) ((x)|=(b))
constexpr unsigned ENTITY_FLAG_DISABLE_DRAWING=1;
struct CEntity {
 unsigned m_nEntityFlags=0; bool meshHidden=false; int updates=0;
 BOOL IsCreated(){return TRUE;}
 BOOL IsDrawEnabled(){return !(m_nEntityFlags&ENTITY_FLAG_DISABLE_DRAWING);}
 void ClassHierarchyDrawEnable(BOOL show){meshHidden=!show;updates++;}
 void DrawEnable(BOOL show,BOOL force=FALSE);
};
struct CFMtx43A {static const CFMtx43A m_IdentityMtx;};
const CFMtx43A CFMtx43A::m_IdentityMtx;
struct CWeapon:CEntity {
 enum {STATE_STOWED,STATE_DEPLOYED};
 bool inWorld=false; int state=STATE_DEPLOYED,attach=0;
 void AddToWorld(){inWorld=true;}
 void RemoveFromWorld(){inWorld=false;}
 void ResetToState(int s){state=s;}
 void Attach_UnitMtxToParent_PS_NewScale_PS(void*,const char*,const CFMtx43A*,float,BOOL){attach++;}
};
enum {BONE_SECONDARY_FIRE,BONE_ATTACHPOINT_PRIMARY};
struct CBotGlitch:CEntity {
 struct Inv {u8 m_nWeaponInvIndex=0;u32 m_nWeaponInvCount=4;CWeapon*m_apWeapon[4]={};}m_WeaponInv[2];
 CWeapon*m_apWeapon[2]={};const char*m_apszBoneNameTable[2]={"secondary","primary"};int m_nPossessionPlayerIndex=0;
 void _ChangeWeaponIndex(u32,u32);
};
int checks=0;void require(bool ok,const char*why){checks++;if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
'''
CHECKS = r'''
int main(){
 // The old implementation propagates an outgoing weapon's temporary hide.
 CWeapon old,next; old.DrawEnable(FALSE); next.DrawEnable(old.IsDrawEnabled());
 require(next.meshHidden,"original hidden-weapon inheritance reproduced");
 for(int p=0;p<4;p++)for(u32 hand=0;hand<2;hand++)for(bool ownerVisible:{false,true}){
  CBotGlitch g;g.m_nPossessionPlayerIndex=p;g.DrawEnable(ownerVisible);
  CWeapon weapons[4];for(int n=0;n<4;n++)g.m_WeaponInv[hand].m_apWeapon[n]=&weapons[n];
  g.m_apWeapon[hand]=&weapons[0];weapons[0].inWorld=true;
  // Includes entity/mesh disagreement after an earlier temporary hide.
  weapons[0].DrawEnable(!ownerVisible);weapons[1].meshHidden=ownerVisible;
  for(u32 index:{1u,2u,3u,0u,1u,3u}){
   CWeapon*out=g.m_apWeapon[hand];g._ChangeWeaponIndex(hand,index);
   CWeapon*active=g.m_apWeapon[hand];
   require(active==&weapons[index]&&g.m_WeaponInv[hand].m_nWeaponInvIndex==index,"correct selection");
   require(!out->inWorld&&active->inWorld,"world membership maintained");
   require(bool(active->IsDrawEnabled())==ownerVisible&&active->meshHidden!=ownerVisible,"entity and mesh visibility follow owner");
   require(active->updates>0,"mesh flags refreshed even if entity flag already agrees");
   require(active->state==(hand?CWeapon::STATE_DEPLOYED:CWeapon::STATE_STOWED),"normal deployment state retained");
   require(hand||active->attach>0,"primary attachment retained");
  }
  CWeapon*before=g.m_apWeapon[hand];g._ChangeWeaponIndex(hand,9);
  require(g.m_apWeapon[hand]==before&&before->inWorld,"invalid index leaves active weapon intact");
  g.m_WeaponInv[hand].m_apWeapon[2]=nullptr;g._ChangeWeaponIndex(hand,2);
  require(hand?g.m_apWeapon[hand]==nullptr:g.m_apWeapon[hand]==before,"empty primary guard and empty secondary retained");
 }
 CBotGlitch hidden;hidden.DrawEnable(FALSE);CWeapon grenade;hidden.m_WeaponInv[1].m_apWeapon[1]=&grenade;
 hidden._ChangeWeaponIndex(1,1);require(grenade.meshHidden,"empty outgoing hand still inherits hidden owner");
 CBotGlitch p1,p2;CWeapon a,b;p1.DrawEnable(FALSE);p1.m_WeaponInv[0].m_apWeapon[1]=&a;
 p2.m_WeaponInv[0].m_apWeapon[1]=&b;p1._ChangeWeaponIndex(0,1);p2._ChangeWeaponIndex(0,1);
 require(a.meshHidden&&!b.meshHidden,"partner visibility remains independent");
 std::printf("PASS: %d offline weapon-visibility checks; game not launched.\n",checks);
}
'''

def main():
    entity=(ROOT/'ma/App/ma/entity.cpp').read_text()
    glitch=(ROOT/'ma/App/ma/botglitch.cpp').read_text()
    code=FIXTURE+method(entity,'void CEntity::DrawEnable(')+method(glitch,'void CBotGlitch::_ChangeWeaponIndex(')+CHECKS
    OUT.mkdir(parents=True,exist_ok=True)
    (OUT/'visibility.cpp').write_text(code)
    (OUT/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(weapon_visibility LANGUAGES CXX)\nadd_executable(weapon_visibility visibility.cpp)\ntarget_compile_features(weapon_visibility PRIVATE cxx_std_17)\n')
    for command in (['cmake','-S',str(OUT),'-B',str(OUT/'out'),'-A','Win32'],['cmake','--build',str(OUT/'out'),'--config','Release']):
        run=subprocess.run(command,capture_output=True,text=True)
        if run.returncode: raise SystemExit(run.stdout+run.stderr)
    subprocess.run([str(OUT/'out/Release/weapon_visibility.exe')],check=True)

if __name__=='__main__': main()

"""Checks the production packing input filter without launching the game."""
from pathlib import Path
import subprocess
from test_coop_checkpoint_rat import method
ROOT=Path(__file__).resolve().parent.parent
OUT=ROOT/'build/test-spy-packing'
FIXTURE=r'''
#include <cstdio>
#include <cstdlib>
using BOOL=int; constexpr BOOL TRUE=1,FALSE=0;
struct CEntity{CEntity* parent=nullptr;};
struct CHumanControl{float m_fForward=1,m_fStrafeRight=-1,m_fCrossDown=1,m_fAimDown=.5f,m_fRotateCW=.25f,m_fFire1=1,m_fFire2=1;unsigned m_nPadFlagsJump=7,m_nPadFlagsSelect1=3,m_nPadFlagsSelect2=5,m_nPadFlagsMelee=9;};
struct CProgramStage{CEntity* crate=nullptr;bool packed=false;BOOL Contains(CEntity *body)const{return packed&&crate&&body&&body->parent==crate;}};
constexpr int STAGE_PROGRAM=5;
struct Game{int m_eCurrentStage=0;CProgramStage* m_pProgramStage=nullptr;};
struct CSpyVsSpy{static inline bool active=false;static inline Game* m_pGame=nullptr;static inline CEntity* lead=nullptr;static BOOL IsFactoryActive(){return active&&m_pGame;}static CEntity* GetGlitch(){return lead;}static void ConstrainPackingControls(CEntity*,CHumanControl*);};
int checks=0;void require(bool good,const char* label){++checks;if(!good){std::printf("FAIL: %s\n",label);std::exit(1);}}
'''
CHECKS=r'''
int main(){CEntity lead,partner,crate,other;CProgramStage stage;stage.crate=&crate;Game game;game.m_pProgramStage=&stage;CSpyVsSpy::m_pGame=&game;CSpyVsSpy::lead=&lead;
 for(int active=0;active<2;++active)for(int program=0;program<2;++program)for(int packed=0;packed<2;++packed)for(int attached=0;attached<2;++attached)for(int actor=0;actor<2;++actor){
  CSpyVsSpy::active=active;game.m_eCurrentStage=program?STAGE_PROGRAM:4;stage.packed=packed;lead.parent=attached?&crate:&other;partner.parent=&crate;CHumanControl input;
  CSpyVsSpy::ConstrainPackingControls(actor?&partner:&lead,&input);bool held=active&&program&&packed&&attached&&!actor;
  require(input.m_fForward==(held?0:1)&&input.m_fStrafeRight==(held?0:-1)&&input.m_fCrossDown==(held?0:1)&&input.m_nPadFlagsJump==(held?0:7),"only the actually packed story actor loses movement and jumping");
  require(input.m_fAimDown==.5f&&input.m_fRotateCW==.25f&&input.m_fFire1==1&&input.m_fFire2==1&&input.m_nPadFlagsSelect1==3&&input.m_nPadFlagsSelect2==5&&input.m_nPadFlagsMelee==9,"aiming firing weapon selection and melee survive containment");
 }
 CSpyVsSpy::active=true;game.m_eCurrentStage=STAGE_PROGRAM;stage.packed=true;lead.parent=&crate;
 CSpyVsSpy::ConstrainPackingControls(&lead,nullptr);CHumanControl input;CSpyVsSpy::ConstrainPackingControls(nullptr,&input);require(input.m_fForward==1,"missing controls or body safe");
 lead.parent=nullptr;CSpyVsSpy::ConstrainPackingControls(&lead,&input);require(input.m_fForward==1&&input.m_nPadFlagsJump==7,"box detachment restores ordinary movement immediately");
 std::printf("PASS: %d production packing input checks; game not launched.\n",checks);
}
'''
def main():
 source=(ROOT/'ma/App/ma/SpyVsSpy.cpp').read_text()
 OUT.mkdir(parents=True,exist_ok=True)
 (OUT/'packing.cpp').write_text(FIXTURE+method(source,'void CSpyVsSpy::ConstrainPackingControls(')+CHECKS)
 (OUT/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(packing LANGUAGES CXX)\nadd_executable(packing packing.cpp)\ntarget_compile_features(packing PRIVATE cxx_std_17)\n')
 for command in (['cmake','-S',str(OUT),'-B',str(OUT/'out'),'-A','Win32'],['cmake','--build',str(OUT/'out'),'--config','Release']):
  run=subprocess.run(command,capture_output=True,text=True)
  if run.returncode:raise SystemExit(run.stdout+run.stderr)
 subprocess.run([str(OUT/'out/Release/packing.exe')],check=True)
if __name__=='__main__':main()

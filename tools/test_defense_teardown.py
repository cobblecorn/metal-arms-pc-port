"""Offline production turret/world teardown checks; never launches the game."""
from pathlib import Path
import subprocess
from test_collectable_skiplist import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'build/test-defense-teardown'

BASE = r'''
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <vector>
using BOOL=int;using u32=unsigned;using cchar=const char;
constexpr BOOL TRUE=1,FALSE=0;
#define FASSERT(x) do{if(!(x))std::abort();}while(0)
#define FINLINE inline
struct FangAllocTag{};
bool fangDelete=false;
#define fnew new(FangAllocTag{})
#define fdelete(p) do{fangDelete=true;delete(p);fangDelete=false;}while(0)
struct CFVec3A{float x=0,y=0,z=0;};
struct CFMtx43A{CFVec3A m_vPos;};
int checks=0,activeBrains=0,brains=0,resourceSets=0,allocs=0,frees=0,destroys=0;
void require(bool ok,const char* why){checks++;if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
struct CEntity{
 enum{ENTITY_FLAG_AUTODELETE=0x80};
 unsigned m_nEntityFlags=0;
 bool created=false,inWorld=false,active=false;
 CFMtx43A matrix;
 CEntity* parent=nullptr;std::vector<CEntity*> children;
 static inline std::vector<CEntity*> in,out;
 static void* operator new(std::size_t n,FangAllocTag){allocs++;return ::operator new(n);}
 static void operator delete(void*p){require(fangDelete,"world cleanup uses Fang deletion");frees++;::operator delete(p);}
 static void operator delete(void*p,FangAllocTag){frees++;::operator delete(p);}
 virtual ~CEntity(){if(created)Destroy();}
 BOOL IsSystemInitialized(){return TRUE;}BOOL IsCreated(){return created;}
 FINLINE void SetAutoDelete(BOOL);
 CFMtx43A* MtxToWorld(){return &matrix;}
 static void erase(std::vector<CEntity*>& list,CEntity*p){list.erase(std::remove(list.begin(),list.end(),p),list.end());}
 static CEntity* prev(std::vector<CEntity*>& list,CEntity*p){auto i=std::find(list.begin(),list.end(),p);return i!=list.end()&&i!=list.begin()?*(i-1):nullptr;}
 static CEntity* InWorldList_GetTail(){return in.empty()?nullptr:in.back();}
 static CEntity* OutOfWorldList_GetTail(){return out.empty()?nullptr:out.back();}
 CEntity* InWorldList_GetPrev(){return prev(in,this);}
 CEntity* OutOfWorldList_GetPrev(){return prev(out,this);}
 BOOL Create(int,BOOL,cchar*,const CFMtx43A*m,cchar*){created=inWorld=active=true;matrix=*m;in.push_back(this);brains++;activeBrains++;resourceSets++;return TRUE;}
 void RemoveFromWorld(BOOL){if(inWorld){erase(in,this);out.push_back(this);inWorld=false;if(active){active=false;activeBrains--;}}}
 void DetachFromParent(){if(parent){erase(parent->children,this);parent=nullptr;}}
 void DetachAllChildren(){for(auto*c:children)c->parent=nullptr;children.clear();}
 virtual void ClassHierarchyDestroy(){require(created&&!inWorld&&!parent&&children.empty(),"entity is removed/detached before brain/resource destruction");erase(out,this);created=false;brains--;resourceSets--;destroys++;}
 void Destroy();static void _DestroyAll();
};
struct CBotAAGun:CEntity{
 BOOL m_bLimitHeading=FALSE,m_bMorterAttachment=FALSE;
 float m_fMaxHeading=0,m_fMinHeading=0,m_fMinPitch=0,m_fMaxPitch=0,m_fHeadingPerSec=0,m_fPitchPerSec=0;
 float HealthContainerCount()const{return 3;}float NormHealth()const{return 1;}
 void* GetArmorProfile()const{return nullptr;}BOOL IsInvincible()const{return FALSE;}unsigned GetTeam()const{return 1;}
 void SetHealthContainerCount(float){}void SetNormHealth(float){}void SetArmorProfile(void*){}
 void SetInvincible(BOOL){}void InitialTeam(unsigned){}void EnableDriverExit(BOOL){}void EnableEnterMsgDisplay(BOOL){}
 ~CBotAAGun()override;
 BOOL PortCreateDefenseGun(const CBotAAGun*,cchar*,const CFMtx43A*);
};
'''

CHECKS = r'''
int main(){
 CBotAAGun config;CFMtx43A matrix;
 // Reproduce the former missing ownership independently of the allocator bug.
 CBotAAGun* stranded[2];
 for(auto&g:stranded){g=fnew CBotAAGun;require(g->PortCreateDefenseGun(&config,"clone",&matrix),"create reproduction turret");g->SetAutoDelete(FALSE);}
 CEntity::_DestroyAll();
 require(activeBrains==2&&brains==2&&CEntity::in.size()==2&&resourceSets==2,"unowned clones survive normal teardown with live brains and stale entity entries");
 for(auto*g:stranded)g->SetAutoDelete(TRUE);
 CEntity::_DestroyAll();
 require(brains==0&&activeBrains==0&&resourceSets==0&&CEntity::in.empty()&&CEntity::out.empty(),"world ownership repairs complete cleanup");
 for(int cycle=0;cycle<100;cycle++)for(int count=1;count<=4;count++){
  CBotAAGun* guns[4]{};CEntity player;
  int before=destroys;
  guns[0]=fnew CBotAAGun;
  require(guns[0]->Create(-1,FALSE,"biggun",&matrix,"Default"),"authored gun setup");guns[0]->SetAutoDelete(TRUE);
  for(int i=1;i<count;i++){
   guns[i]=fnew CBotAAGun;
   require(guns[i]->PortCreateDefenseGun(guns[0],"partner",&matrix),"create partner turret");
   require(guns[i]->m_nEntityFlags&CEntity::ENTITY_FLAG_AUTODELETE,"production helper transfers clone ownership to world");
   if(cycle%2)guns[i]->RemoveFromWorld(TRUE); // e.g. checkpoint/world removal
  }
  // A seated occupant is detached safely without becoming world-owned.
  player.parent=guns[count-1];guns[count-1]->children.push_back(&player);
  CEntity::_DestroyAll();
  require(!player.parent,"seated player is detached during turret destruction");
  require(destroys==before+count,"every gun destroyed exactly once");
  require(brains==0&&activeBrains==0&&resourceSets==0,"all gun brains and dependent resources released before AI shutdown");
  require(CEntity::in.empty()&&CEntity::out.empty(),"level-complete fixups cannot traverse freed turret entries");
  require(allocs==frees,"world cleanup deletes all allocated turrets");
  CEntity::_DestroyAll();require(destroys==before+count,"repeated cleanup is safe");
 }
 std::printf("PASS: %d turret/world teardown checks; game not launched.\n",checks);
}
'''


def main():
    entity = (ROOT / 'ma/App/ma/entity.cpp').read_text()
    gun = (ROOT / 'ma/App/ma/botAAgun.cpp').read_text()
    header = (ROOT / 'ma/App/ma/entity.h').read_text()
    setter = method(header, 'FINLINE void SetAutoDelete(')
    setter = setter.replace('SetAutoDelete(', 'CEntity::SetAutoDelete(', 1)
    code = BASE + setter + method(entity, 'void CEntity::Destroy(')
    code += method(entity, 'void CEntity::_DestroyAll(')
    code += method(gun, 'CBotAAGun::~CBotAAGun(')
    code += method(gun, 'BOOL CBotAAGun::PortCreateDefenseGun(') + CHECKS
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'teardown.cpp').write_text(code)
    (OUT / 'CMakeLists.txt').write_text(
        'cmake_minimum_required(VERSION 3.20)\nproject(defense_teardown LANGUAGES CXX)\n'
        'add_executable(teardown teardown.cpp)\ntarget_compile_features(teardown PRIVATE cxx_std_17)\n')
    for command in (['cmake', '-S', str(OUT), '-B', str(OUT / 'out'), '-A', 'Win32'],
                    ['cmake', '--build', str(OUT / 'out'), '--config', 'Release']):
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode:
            raise SystemExit(result.stdout + result.stderr)
    subprocess.run([str(OUT / 'out/Release/teardown.exe')], check=True)


if __name__ == '__main__':
    main()

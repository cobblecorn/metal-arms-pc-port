"""Compile production chip/EUK/recovery methods with small offline engine fixtures."""
from pathlib import Path
import re
import subprocess

from gamedata_dump import decode
from test_collectable_skiplist import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-checkpoint-euk"

BASE = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <limits>
using u32=unsigned;using s32=int;using s16=short;using f32=float;using cchar=const char;using BOOL=bool;
#define TRUE true
#define FALSE false
#define strnicmp _strnicmp
#define fclib_stricmp _stricmp
#define FASSERT(x) require(!!(x),"production assertion")
#define DEVPRINTF(...) ((void)0)
constexpr int MAX_PLAYERS=4,ENTITY_BIT_BOTGLITCH=1;
constexpr float _TC_MOVE=.25f,FMATH_MAX_FLOAT=3.402823466e38f;
constexpr int _EUK_MESH_ID=50;
int checks=0;
void require(bool ok,const char* why){++checks;if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
struct CItem {u32 m_uCurLevel=1;CItem* m_pUpgraded=nullptr;CItem* m_pDowngraded=nullptr;};
struct CWeapon;
struct CItemInst {CItem* m_pItemData=nullptr;CWeapon* m_pWeapon=nullptr;s16 m_nUpgradeLevel=1;void NotifyWeaponUpgraded(u32);u32 HighestUpgradeAvailable();};
struct CWeapon {u32 level=0,count=3;bool created=true;CItemInst* item=nullptr;int ammo=19;
 bool IsCreated(){return created;}u32 GetUpgradeLevel(){return level;}u32 GetUpgradeLevelCount(){return count;}
 void SetUpgradeLevel(u32 n){require(n<count,"valid weapon level");level=n;item->NotifyWeaponUpgraded(n);}
};
struct CInventory {CItemInst* item=nullptr;const char* tag=nullptr;
 CItemInst* IsWeaponInInventory(const char* name){return tag&&!_stricmp(tag,name)?item:nullptr;}
};
struct CBot {CInventory* m_pInventory=nullptr;int bits=ENTITY_BIT_BOTGLITCH,m_nPossessionPlayerIndex=0;int TypeBits(){return bits;}};
struct CFVec2 {float x=0,y=0;void Zero(){x=y=0;}void Set(float a,float b){x=a;y=b;}};
using FMeshTexLayerHandle_t=int;
struct Mesh {int nTexLayerIDCount=1;};
struct CFWorldMesh {Mesh mesh;Mesh* m_pMesh=&mesh;CFVec2 rate,st;int calls=0;
 int GetTexLayerHandle(int id){require(id==50,"EUK texture layer");return id;}
 void AnimTC_SetScrollRate(int,const CFVec2& v){rate=v;}
 void AnimTC_SetScrollST(int,const CFVec2& v){st=v;++calls;}
};
struct CCollectableType {CollectableType_e m_eType;const char* m_pszName;
 static void RemapTCs(CFWorldMesh*,CollectableType_e);
};
struct NameType_t {const char* pszName;CollectableType_e eType;};
CCollectableType weaponTypes[24];
struct CCollectable {
 static BOOL IsWeaponUpgrade(CollectableType_e t){return t>=COLLECTABLE_WEAPON_UPGRADE&&t<COLLECTABLE_WEAPON_UPGRADE_END;}
 static void* m_pCollectHud;static CCollectableType* shown;
 static CCollectableType* _FindCollectableType(CollectableType_e t){for(auto& v:weaponTypes)if(v.m_eType==t)return &v;return nullptr;}
 static CWeapon* _FindWeaponUpgrade(CBot*,CCollectableType*,u32&);
 static BOOL _IsFirstScopePickup(CBot*,CCollectableType*);
 static BOOL GiveWeaponToPlayer(CBot*,const char* name,int ammo){require(!_stricmp(name,"Scope L1")&&ammo==0,"first scope acquires the base attachment without ammo");return true;}
 static BOOL _GiveWeaponUpgradeToPlayer(CBot*,CCollectableType*);
 static void _PickupSpecialItem(CCollectableType* t){shown=t;}
};
void* CCollectable::m_pCollectHud=(void*)1;CCollectableType* CCollectable::shown=nullptr;
struct CFCheckPoint {static std::vector<unsigned> data;static size_t cursor;
 static void SaveData(unsigned v){data.push_back(v);}static void LoadData(unsigned& v){v=data.at(cursor++);}
};
std::vector<unsigned> CFCheckPoint::data;size_t CFCheckPoint::cursor=0;
struct CPlayer {
 struct{bool m_bIsSinglePlayer=true;struct{unsigned m_nSecretChipsCollected=0;}m_SPStats;}m_GameStats;
 void CreditSecretChip();static void CheckpointSaveSecretChips();static void CheckpointRestoreSecretChips();
};
CPlayer Player_aPlayer[MAX_PLAYERS];
struct CFVec3A {float x=0,y=0,z=0;float MagSq()const{return x*x+y*y+z*z;}void Zero(){x=y=z=0;}};
struct PhysicsState_t {
 struct{CFVec3A m_vX{1,0,0},m_vY{0,1,0},m_vZ{0,0,1};}mOrientation;
 CFVec3A vPosition,vVelocity,vMomentum,vAcceleration,vAngularVelocity,vAngularMomentum,vAngularAcceleration;
 CFVec3A vTotalForce,vTotalTorque,vTotalFriction,vTotalFrictionTorque;
};
struct CFPhysicsObject {PhysicsState_t m_State,m_PortGoodState;bool m_bPortHaveGoodState=false;
 BOOL _PortStateIsSane(const PhysicsState_t&)const;void _PortRecoverState(cchar*);
};
'''

CHECKS = r'''
int main(){
 const char* families[]={"laser","ripper","rivet gun","rocket","blaster","tether","spew","scope"};
 CollectableType_e bases[]={COLLECTABLE_WEAPON_LASER_L1,COLLECTABLE_WEAPON_RIPPER_L1,COLLECTABLE_WEAPON_RIVET_GUN_L1,COLLECTABLE_WEAPON_ROCKET_L1,COLLECTABLE_WEAPON_BLASTER_L1,COLLECTABLE_WEAPON_TETHER_L1,COLLECTABLE_WEAPON_SPEW_L1,COLLECTABLE_WEAPON_SCOPE_L1};
 const float offsets[8][2]={{.25f,0},{.75f,.25f},{0,0},{.25f,.25f},{.5f,0},{.75f,.5f},{.5f,.25f},{0,0}};
 char names[24][64]{};int wi=0;
 for(int f=0;f<8;++f)for(int l=0;l<(f==7?2:3);++l){std::sprintf(names[wi],"%s L%d",families[f],l+1);weaponTypes[wi]={(CollectableType_e)(bases[f]+l),names[wi]};++wi;}
 for(int f=0;f<8;++f)for(int variant=1;variant<=(f==7?2:3);++variant){
  char name[64];std::sprintf(name,"euk %s",families[f]);if(variant>1)std::sprintf(name+std::strlen(name)," %d",variant);
  CCollectableType pickup{_RetailUpgradeType(name),name};
  require(_RetailUpgradeBase(name)==bases[f],"retail EUK classification");
  CCollectableType* base=CCollectable::_FindCollectableType(bases[f]);
  CItem levels[3];for(int i=0;i<3;++i){levels[i].m_uCurLevel=i+1;levels[i].m_pUpgraded=i<(f==7?1:2)?&levels[i+1]:nullptr;levels[i].m_pDowngraded=i?&levels[i-1]:nullptr;}
  CItemInst items[4];CWeapon guns[4];CInventory inventories[4];CBot bots[4];
  for(int p=0;p<4;++p){items[p].m_pItemData=&levels[0];items[p].m_pWeapon=&guns[p];guns[p].count=f==7?3:4;guns[p].item=&items[p];inventories[p]={&items[p],base->m_pszName};bots[p].m_pInventory=&inventories[p];bots[p].m_nPossessionPlayerIndex=p;}
  for(int p=0;p<4;++p){
   require(CCollectable::_GiveWeaponUpgradeToPlayer(&bots[p],&pickup),"each co-op recipient can upgrade");
   require(guns[p].level==1&&items[p].m_nUpgradeLevel==2&&items[p].m_pItemData==&levels[1],"weapon and saved item level both advance");
   require(guns[p].ammo==19,"upgrade does not invent ammo");
   require(CCollectable::shown->m_eType==bases[f]+1,"HUD gets upgraded weapon");
   if(f!=7){require(CCollectable::_GiveWeaponUpgradeToPlayer(&bots[p],&pickup),"second EUK advances L2 to L3");require(items[p].m_nUpgradeLevel==3&&items[p].m_pItemData==&levels[2],"L3 persists in inventory");}
   require(!CCollectable::_GiveWeaponUpgradeToPlayer(&bots[p],&pickup),"maxed player gun refuses NPC-only tier without traversing past item chain");
  }
  inventories[0].item=nullptr;require(CCollectable::_GiveWeaponUpgradeToPlayer(&bots[0],&pickup)==(f==7&&variant==1),"only the unnumbered first scope acquires a missing base attachment");
  bots[1].bits=0;require(!CCollectable::_GiveWeaponUpgradeToPlayer(&bots[1],&pickup),"possessed NPC cannot consume EUK");
  CFWorldMesh mesh;CCollectableType::RemapTCs(&mesh,_RetailUpgradeBase(name));
  require(mesh.calls==(f==7?0:1),"scope stays untouched and EUK atlas remaps");
  require(mesh.st.x==offsets[f][0]&&mesh.st.y==offsets[f][1],"correct weapon icon atlas offset");
 }
 for(const char* bad:{"euk","euk laser 4","euk scope 3","euk rocketship","euk rivet gun 22",""})require(_RetailUpgradeBase(bad)==COLLECTABLE_UNKNOWN,"unknown EUK names rejected");
 require(_RetailUpgradeBase(nullptr)==COLLECTABLE_UNKNOWN,"null name rejected");
 require(_RetailUpgradeBase("EUK RIVET GUN 2")==COLLECTABLE_WEAPON_RIVET_GUN_L1,"retail case insensitive");
 for(int p=0;p<4;++p)Player_aPlayer[p].m_GameStats.m_SPStats.m_nSecretChipsCollected=p;
 CFCheckPoint::data.clear();CPlayer::CheckpointSaveSecretChips();require(CFCheckPoint::data.size()==4,"checkpoint saves four fixed slots");
 for(int death=0;death<100;++death){
  for(int p=0;p<4;++p)Player_aPlayer[p].CreditSecretChip();
  CFCheckPoint::cursor=0;CPlayer::CheckpointRestoreSecretChips();
  for(int p=0;p<4;++p)require(Player_aPlayer[p].m_GameStats.m_SPStats.m_nSecretChipsCollected==(unsigned)p,"recollect and die never stacks secret count");
 }
 Player_aPlayer[0].CreditSecretChip();CFCheckPoint::data.clear();CPlayer::CheckpointSaveSecretChips();Player_aPlayer[0].CreditSecretChip();
 CFCheckPoint::cursor=0;CPlayer::CheckpointRestoreSecretChips();require(Player_aPlayer[0].m_GameStats.m_SPStats.m_nSecretChipsCollected==1,"later checkpoint retains earlier chip");
 Player_aPlayer[0].m_GameStats.m_bIsSinglePlayer=false;Player_aPlayer[0].CreditSecretChip();require(Player_aPlayer[0].m_GameStats.m_SPStats.m_nSecretChipsCollected==1,"PvP chip credit unchanged");
 CFPhysicsObject tank;tank.m_State.vPosition={4000,-200,1000};tank.m_State.vVelocity={100,0,0};tank.m_State.vAngularVelocity={0,10,0};
 require(tank._PortStateIsSane(tank.m_State),"ordinary tank motion accepted");
 tank.m_PortGoodState=tank.m_State;tank.m_bPortHaveGoodState=true;
 tank.m_State.vPosition.x=std::numeric_limits<float>::quiet_NaN();require(!tank._PortStateIsSane(tank.m_State),"invalid state rejected");
 tank._PortRecoverState("after step");require(tank._PortStateIsSane(tank.m_State)&&tank.m_State.vPosition.x==4000&&tank.m_State.vVelocity.MagSq()==0,"safety recovery still works");
 tank.m_bPortHaveGoodState=false;tank.m_State.vPosition.x=12;tank._PortRecoverState("before step");require(tank.m_State.vPosition.x==12,"reset cache cannot restore old vehicle position");
 std::printf("PASS: %d checkpoint, retail EUK, atlas and physics checks; game not launched.\n",checks);
}
'''


def main():
    collect = (ROOT / "ma/App/ma/collectable.cpp").read_text()
    header = (ROOT / "ma/App/ma/collectable.h").read_text()
    player = (ROOT / "ma/App/ma/player.cpp").read_text()
    physics = (ROOT / "ma/Lib/Fang2/fphysics.cpp").read_text()
    enum = re.search(r"typedef enum\s*\{.*?\}\s*CollectableType_e;", header, re.S).group()
    table = re.search(r"static const NameType_t _retailWeaponUpgrades\[\] = \{.*?\};", collect, re.S).group()
    code = "#define FANG_WINGC 1\n" + enum + BASE + table + "\n"
    for source, signature in (
        (collect, "static CollectableType_e _RetailUpgradeBase("),
        (collect, "static CollectableType_e _RetailUpgradeType("),
        ((ROOT / "ma/App/ma/ItemInst.cpp").read_text(), "void CItemInst::NotifyWeaponUpgraded("),
        ((ROOT / "ma/App/ma/ItemInst.cpp").read_text(), "u32 CItemInst::HighestUpgradeAvailable("),
        (collect, "BOOL CCollectable::_IsFirstScopePickup("),
        (collect, "CWeapon *CCollectable::_FindWeaponUpgrade("),
        (collect, "BOOL CCollectable::_GiveWeaponUpgradeToPlayer("),
        (collect, "void CCollectableType::RemapTCs("),
        (player, "void CPlayer::CreditSecretChip("),
        (player, "void CPlayer::CheckpointSaveSecretChips("),
        (player, "void CPlayer::CheckpointRestoreSecretChips("),
        (physics, "static BOOL _PortVecIsSane("),
        (physics, "BOOL CFPhysicsObject::_PortStateIsSane("),
        (physics, "void CFPhysicsObject::_PortRecoverState("),
    ):
        code += method(source, signature)
    retail = decode((ROOT / "gamedata/mst/goodies.csv").read_bytes())["tables"]
    names = [t["name"] for t in retail if t["name"].lower().startswith("euk ")]
    assert len(names) == 23, names
    known = dict(re.findall(r'\{\s*"([^"]+)"\s*,\s*(COLLECTABLE_\w+)\s*\}',
                            re.search(r"static const NameType_t _collectableTypes\[\] = \{.*?\};", collect, re.S).group()))
    retail_names = {t["name"].lower() for t in retail}
    item_names = {t["name"].lower() for t in decode((ROOT / "gamedata/mst/itemlist$.csv").read_bytes())["tables"]}
    for base in re.findall(r'\{\s*"euk [^"]+"\s*,\s*(COLLECTABLE_\w+)\s*\}', table):
        tag = next(name for name, kind in known.items() if kind == base)
        assert tag.lower() in retail_names and tag.lower() in item_names, tag
    # Real retail names, not just fixture spellings.
    checks = "".join(f'require(_RetailUpgradeBase("{n}")!=COLLECTABLE_UNKNOWN,"authored EUK supported");\n' for n in names)
    code += CHECKS.replace("int main(){", "int main(){\n" + checks)
    save = (ROOT / "ma/App/ma/gamesave.cpp").read_text()
    assert "CheckpointSaveSecretChips" in method(save, "static void _SaveGlobalData(")
    assert "CheckpointRestoreSecretChips" in method(save, "static void _RestoreGlobalData(")
    assert "m_bPortHaveGoodState = FALSE" in method(physics, "void CFPhysicsObject::_SetDefaults(")
    assert "IsWeaponUpgrade" in method(collect, "BOOL CCollectable::_IsPersonalPickup(")
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "regression.cpp").write_text(code)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(checkpoint_euk LANGUAGES CXX)\n"
        "add_executable(regression regression.cpp)\ntarget_compile_features(regression PRIVATE cxx_std_17)\n")
    for command in (["cmake", "-S", str(OUT), "-B", str(OUT / "out"), "-A", "Win32"],
                    ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode:
            raise SystemExit(result.stdout + result.stderr)
    subprocess.run([str(OUT / "out/Release/regression.exe")], check=True)


if __name__ == "__main__":
    main()

"""Check production charged-projectile impacts against local retail tables and DOL."""
from pathlib import Path
import struct
import subprocess
from gamedata_dump import decode
from dol_xref import Dol
from test_coop_checkpoint_rat import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'build/test-laser-retail-impact'
BASE = r'''
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
using u16=unsigned short;using u8=unsigned char;using u32=unsigned;using f32=float;using BOOL=bool;
constexpr BOOL FALSE=false,TRUE=true;
int checks=0;void check(bool v,const char* s){++checks;if(!v){std::fprintf(stderr,"FAIL: %s\n",s);std::exit(1);}}
struct CFVec3 {float x,y,z;};
struct CFVec3A {union{struct{float x,y,z;};CFVec3 v3;};
 CFVec3A(float X=0,float Y=0,float Z=0):x(X),y(Y),z(Z){}
 CFVec3A& Mul(const CFVec3A& a,float n){x=a.x*n;y=a.y*n;z=a.z*n;return *this;}
 CFVec3A& Add(const CFVec3A& a){x+=a.x;y+=a.y;z+=a.z;return *this;}
 float Dot(const CFVec3A& a)const{return x*a.x+y*a.y+z*a.z;}
 void ReceiveReflection(const CFVec3A& d,const CFVec3A& n){float k=-2*d.Dot(n);Mul(n,k).Add(d);}
 void Unitize(){float k=1/std::sqrt(Dot(*this));x*=k;y*=k;z*=k;}
};
struct CEntity {enum{PROJECTILE_REACTION_DEFAULT,PROJECTILE_REACTION_RICOCHET,PROJECTILE_REACTION_DEATH_TO_ALL,PROJECTILE_REACTION_STICKY,PROJECTILE_REACTION_HURT_ME,PROJECTILE_REACTION_SHIELD};};
struct CFWorldMesh{};struct CDamageProfile{float amount;};
struct FCollImpact_t{CFVec3A ImpactPoint{2,3,4},UnitFaceNormal{0,0,-1};int reaction=0;bool wire=false;CEntity* entity=nullptr;void* pTag=nullptr;unsigned nUserType=0;};
struct CDamageForm{enum{DAMAGE_LOCALE_IMPACT,DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY};
 struct Damager_t{int nDamagerPlayerIndex=-1;void* pWeapon=nullptr;};
 int m_nDamageLocale=0,m_nDamageDelivery=0;CDamageProfile* m_pDamageProfile=nullptr;
 Damager_t m_Damager;CEntity* m_pDamageeEntity=nullptr;
 void InitTriDataFromCollImpact(CFWorldMesh*,const FCollImpact_t*,const CFVec3A*){}
};
struct CDamage{static inline CDamageForm form;static inline std::vector<CDamageForm> submitted;
 static CDamageForm* GetEmptyDamageFormFromPool(){form={};return &form;}
 static void SubmitDamageForm(CDamageForm* p){submitted.push_back(*p);}
};
struct Material{void DrawAllDrawableParticles(const CFVec3A*,const CFVec3A*,BOOL,float,float,float)const{}};
using CGCollMaterial=Material;
struct CGColl{static bool ExtractWire(const FCollImpact_t* p){return p->wire;}
 static u8 ComputeProjectileReaction(const FCollImpact_t* p){return p->reaction;}
 static const Material* GetMaterial(const FCollImpact_t*){static Material m;return &m;}
 static CEntity* ExtractEntity(const FCollImpact_t* p){return p->entity;}
};
using FExplosion_GroupHandle_t=const char*;using FExplosion_SpawnerHandle_t=int;
constexpr unsigned FEXPLOSION_SPAWN_NO_SMOKE=0x40;
struct FExplosionSpawnParams_t {unsigned uFlags=0,uSurfaceType=0;CFVec3A Pos_WS,UnitDir;void InitToDefaults(){*this={};}};
struct CExplosion2{static inline std::vector<const char*> groups;
 static FExplosion_GroupHandle_t GetExplosionGroup(const char* n){return n;}
 static FExplosion_SpawnerHandle_t GetExplosionSpawner(){return 1;}
 static void SpawnExplosion(int,const char* group,FExplosionSpawnParams_t* p){check(p->uFlags==0x40,"retail explosion flag is NO_SMOKE, not NO_DAMAGE");groups.push_back(group);}
};
using FDecalDefHandle_t=const char*;
struct CFDecal{static inline const char* last=nullptr;
 static const char* GetDefByName(const char* s){return s;}
 static void Create(const char* s,const FCollImpact_t*,const CFVec3A*){last=s;}
};
struct CFSoundGroup{static inline int plays=0;static void PlaySound(CFSoundGroup*,BOOL,const CFVec3A*){++plays;}};
enum TracerKillReason_e{TRACER_KILLREASON_REACHED_MAX_DIST,TRACER_KILLREASON_HIT_GEO,TRACER_KILLREASON_COUNT};
constexpr unsigned TRACERFLAG_THICK_PROJECTILE=8,TRACERFLAG_DEFER_FIRST_WORK=16;
constexpr u16 _CHARGED_TRACER=0x8000;
struct TracerDef_t{void* pUser=nullptr;u16 nUser=0;unsigned uFlags=8;void(*pFcnBuildSkipList)(void*)=nullptr;
 CFVec3A UnitDir_WS{0,0,1},TailPos_WS;CDamageForm::Damager_t Damager;};
std::vector<TracerDef_t> continuations;
bool tracer_NewTracer(int,const TracerDef_t* t,float head,BOOL,BOOL,const CDamageForm::Damager_t* d){
 check(head==0,"charged continuation starts at the impact rather than extending through geometry");
 auto copy=*t;copy.Damager=*d;continuations.push_back(copy);return true;
}
int endParticles=0;
void fparticle_SpawnEmitter(int,const CFVec3&,const CFVec3*,float){++endParticles;}
struct CWeaponLaser{static constexpr unsigned EUK_COUNT_LASER=5;
 struct Charge{CDamageProfile* apDamage[3]{};int hEndParticle=1;float fRange=100;
 const char* pszRicochetImpact="ric",*pszImpact="hit",*pszDecal="burst";CFSoundGroup* pRicochetSound=nullptr;};
 using _RetailCharge_t=Charge;static inline Charge m_aRetailCharge[5];int m_hTracerGroup=1;
 static CDamageProfile* _ChargedDamageProfile(u16);
 static void _ChargedTracerCallback(TracerDef_t*,TracerKillReason_e,const FCollImpact_t*);
};
'''
CHECKS = r'''
int main(){
 CWeaponLaser laser;CDamageProfile profiles[5][3]{};
 const float retail[3][3]={{20,35,50},{35,55,75},{50,75,100}};
 for(int j=0;j<3;++j){int level=j==2?4:j+1;for(int t=0;t<3;++t){profiles[level][t].amount=retail[j][t];CWeaponLaser::m_aRetailCharge[level].apDamage[t]=&profiles[level][t];}}
 for(int level:{1,2,4})for(int tier=0;tier<3;++tier)for(int player=0;player<4;++player)for(int reaction=0;reaction<6;++reaction){
  continuations.clear();CDamage::submitted.clear();CExplosion2::groups.clear();CFDecal::last=nullptr;
  TracerDef_t shot;shot.pUser=&laser;shot.nUser=_CHARGED_TRACER|(tier<<3)|level;shot.Damager.nDamagerPlayerIndex=player;
  CEntity target;FCollImpact_t hit;hit.reaction=reaction;hit.entity=&target;hit.pTag=&target;
  CWeaponLaser::_ChargedTracerCallback(&shot,TRACER_KILLREASON_HIT_GEO,&hit);
  check(CDamage::submitted.size()==1,"exactly one direct charged damage form per impact");
  auto form=CDamage::submitted[0];check(form.m_pDamageProfile==&profiles[level][tier],"retail damage band survives reflection and weapon switching");
  check(form.m_Damager.nDamagerPlayerIndex==player&&form.m_pDamageeEntity==&target,"shot ownership and target preserved for every player");
  check(continuations.size()==1&&continuations[0].UnitDir_WS.z==-1,"retail charged shot reflects at both terrain and bot surfaces");
  check(continuations[0].nUser==shot.nUser&&continuations[0].Damager.nDamagerPlayerIndex==player,"reflection retains original tier and shooter");
  check(continuations[0].uFlags&TRACERFLAG_DEFER_FIRST_WORK,"reflections defer movement to avoid same-frame collision cascades");
  bool reflect=reaction==1||reaction==5;
  check(CExplosion2::groups.size()==1&&CExplosion2::groups[0]==(reflect?CWeaponLaser::m_aRetailCharge[level].pszRicochetImpact:CWeaponLaser::m_aRetailCharge[level].pszImpact),"shield and ricochet surfaces select retail ricochet impact group");
  check((CFDecal::last!=nullptr)==!reflect,"only regular impacts use charged burst decals");
  continuations.clear();CDamage::submitted.clear();hit.wire=true;
  CWeaponLaser::_ChargedTracerCallback(&shot,TRACER_KILLREASON_HIT_GEO,&hit);
  check(continuations.size()==1&&continuations[0].UnitDir_WS.z==1&&CDamage::submitted.empty(),"wire contact passes through without damage");
  continuations.clear();auto before=endParticles;
  CWeaponLaser::_ChargedTracerCallback(&shot,TRACER_KILLREASON_REACHED_MAX_DIST,nullptr);
  check(endParticles==before+1&&continuations.empty()&&CDamage::submitted.empty(),"range expiry emits retail end effect without a damage hit");
 }
 TracerDef_t invalid;invalid.pUser=&laser;invalid.nUser=0xffff;auto before=continuations.size();
 CWeaponLaser::_ChargedTracerCallback(&invalid,TRACER_KILLREASON_HIT_GEO,nullptr);check(continuations.size()==before,"invalid charged snapshots safely ignored");
 std::printf("PASS: %d production charged-impact checks; no game launched.\n",checks);
}
'''


def main():
    tables = {t['name']: [f['value'] for f in t['fields']]
              for t in decode((ROOT/'gamedata/mst/w_laser.csv').read_bytes())['tables']}
    damage = {t['name']: [f['value'] for f in t['fields']]
              for t in decode((ROOT/'gamedata/mst/damage.csv').read_bytes())['tables']}
    for name, expected in [('laserl2', [20, 35, 50]), ('laserl3', [35, 55, 75]),
                           ('lasermil_possessed', [50, 75, 100])]:
        assert [damage[n.lower()][6] for n in tables[name][60:63]] == expected
    dol = Dol((ROOT/'gamedata/sys/main.dol').read_bytes())
    assert dol.word(0x801fa318) == 0xc002d594  # top band compares the constant below
    assert struct.unpack('>f', struct.pack('>I', dol.word(0x804bf9b4)))[0] == struct.unpack('>f', struct.pack('>f', .95))[0]
    assert dol.word(0x801fa330) == 0xc002d52c  # middle band compares .5
    assert struct.unpack('>f', struct.pack('>I', dol.word(0x804bf94c)))[0] == .5
    source = (ROOT/'ma/App/ma/weapon_laser.cpp').read_text()
    code = BASE + method(source, 'CDamageProfile *CWeaponLaser::_ChargedDamageProfile(')
    code += method(source, 'void CWeaponLaser::_ChargedTracerCallback(') + CHECKS
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT/'impact.cpp').write_text(code)
    (OUT/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(laser_retail_impact LANGUAGES CXX)\nadd_executable(impact impact.cpp)\ntarget_compile_features(impact PRIVATE cxx_std_17)\n')
    for cmd in [['cmake', '-S', str(OUT), '-B', str(OUT/'out'), '-A', 'Win32'],
                ['cmake', '--build', str(OUT/'out'), '--config', 'Release']]:
        run = subprocess.run(cmd, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT/'out/Release/impact.exe')], check=True)


if __name__ == '__main__':
    main()

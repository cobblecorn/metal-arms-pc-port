"""Offline production-method checks for the campaign playthrough fixes.

Compiles isolated world/audio/checkpoint fixtures; never launches ma_port or uses saves.
"""
from pathlib import Path
import subprocess
from test_coop_checkpoint_rat import method, FIXTURE as PLACEMENT_FIXTURE

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'build/test-coop-playthrough'

BASE = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <vector>
using BOOL=int;using s32=int;using u32=unsigned;using s16=short;using f32=float;using cchar=const char;
constexpr BOOL TRUE=1,FALSE=0;
#define FANG_WINGC 1
#define FASSERT(x) ((void)0)
#define DEVPRINTF(...) ((void)0)
#define FINLINE inline
#define FMATH_MIN(a,b) std::min(a,b)
int checks=0;
void require(bool b,const char* m){++checks;if(!b){std::printf("FAIL: %s\n",m);std::exit(1);}}
int fclib_stricmp(const char* a,const char* b){return _stricmp(a,b);}
'''

RADIO = r'''
using TransmissionAuthor_e=int;using FSndFx_FxHandle_t=int;using GamePhrase_e=int;
constexpr int TRANSMISSION_AUTHOR_COUNT=5,FSNDFX_INVALID_FX_HANDLE=-1,FAUDIO_EMITTER_STATE_PLAYING=1;
constexpr int GAMEPHRASE_COLONEL_ALLOY=0,GAMEPHRASE_AGENT_SHHH=1,GAMEPHRASE_KRUNK=2,GAMEPHRASE_AGENT_GOFF=3,GAMEPHRASE_DR_EXAVOLT=4;
constexpr int ENTITY_BIT_BOTGLITCH=1,_TRANSMISSION_ANTENNA_ATTACH_BONE=1;
constexpr float _TRANSMISSION_FLASH_SECS=.1f,_TRANSMISSION_FADEOUT_SECS=.2f,_TRANSMISSION_ANTENNA_OFFSET_Y=1,_TRANSMISSION_ANTENNA_SCALE=1;
float FLoop_fPreviousLoopSecs=.1f;int FAudio_EmitterDefaultPriorityLevel=1;
const wchar_t* Game_apwszPhrases[]={L"Alloy",L"Shhh",L"Krunk",L"Goff",L"Exavolt"};
int audioStarts=0,streamStarts=0,streamStops=0;bool streamPlaying=false,bankPlaying=true;
struct Emitter{void Destroy(){delete this;}int GetState(){return bankPlaying?1:0;}};
Emitter* alloc(int,float,float,int,float,BOOL){++audioStarts;return new Emitter;}
#define FSNDFX_ALLOCNPLAY2D alloc
void level_PlayStreamingSpeech(const char*,float){++streamStarts;streamPlaying=true;}
void level_StopStreamingSpeech(){++streamStops;streamPlaying=false;}
BOOL level_IsStreamingSpeechPlaying(){return streamPlaying;}
struct CEntity{int TypeBits(){return ENTITY_BIT_BOTGLITCH;}};
struct CFMtx43A{struct{float y=0;}m_vPos;void Identity(){}};
struct Antenna{bool visible=false;CEntity* parent=nullptr;void AddToWorld(){visible=true;}void RemoveFromWorld(){visible=false;parent=nullptr;}
 void Attach_UnitMtxToParent_PS_NewScale_WS(CEntity* p,int,CFMtx43A*,float,BOOL){parent=p;}};
struct CHud2{
 enum{TRANSMISSION_STATE_IDLE,TRANSMISSION_STATE_START_ON1,TRANSMISSION_STATE_START_OFF1,
 TRANSMISSION_STATE_START_ON2,TRANSMISSION_STATE_START_OFF2,TRANSMISSION_STATE_ON,TRANSMISSION_STATE_FADING_OUT};
 int m_nPlayerIdx=0,m_nTransmissionState=TRANSMISSION_STATE_IDLE;float m_fTransmissionsTimer=0;
 bool m_bTransmissionMirror=false,m_bTransmissionAbortWithCutScene=false;
 Emitter* m_pTransmissionAudioEmitter=nullptr;const wchar_t* m_pwszTransmissionAuthor=nullptr;
 Antenna antenna;Antenna* m_pTransmissionAntennaMeshEntity=&antenna;
 static void TransmissionShared_Stop(BOOL);
 static BOOL TransmissionShared_Start(TransmissionAuthor_e,FSndFx_FxHandle_t,float,BOOL);
 static BOOL TransmissionShared_Start(TransmissionAuthor_e,const char*,float,BOOL);
 BOOL TransmissionMsg_Start(TransmissionAuthor_e,FSndFx_FxHandle_t,float,BOOL);
 BOOL TransmissionMsg_Start(TransmissionAuthor_e,const char*,float,BOOL);
 void _TransmissionMsg_Start(TransmissionAuthor_e,BOOL);void TransmissionMsg_Stop(BOOL=TRUE);void _TransmissionWork();
};
struct CPlayer{static inline int m_nPlayerCount=2;CHud2 m_Hud;CEntity* m_pEntityOrig=nullptr;};
CPlayer Player_aPlayer[4];int current=0;
CHud2* GetCurrentHud(){return &Player_aPlayer[current].m_Hud;}
struct{bool coop=true;BOOL IsLocalCoop(){return coop;}}MultiplayerMgr;
'''
RADIO_CHECKS = r'''
void run(){
 CEntity bodies[4];for(int i=0;i<4;i++){Player_aPlayer[i].m_Hud.m_nPlayerIdx=i;Player_aPlayer[i].m_pEntityOrig=&bodies[i];}
 for(int count=2;count<=4;count++)for(current=0;current<count;current++)for(int bank=0;bank<2;bank++){
  CPlayer::m_nPlayerCount=count;int before=audioStarts+streamStarts;bankPlaying=true;
  bool started=bank?CHud2::TransmissionShared_Start(2,10,.5f,TRUE):CHud2::TransmissionShared_Start(2,"radio",.5f,TRUE);
  require(started&&audioStarts+streamStarts==before+1,"radio audio starts exactly once regardless of current HUD");
  for(int i=0;i<count;i++){
   auto& h=Player_aPlayer[i].m_Hud;
   require(h.m_pwszTransmissionAuthor==Game_apwszPhrases[2]&&h.antenna.visible&&h.antenna.parent==&bodies[i],"each HUD and original body receives radio visuals");
   require(h.m_bTransmissionMirror==(i!=0),"only P1 owns radio audio");
  }
  for(int frame=0;frame<5;frame++)for(int i=0;i<count;i++)Player_aPlayer[i].m_Hud._TransmissionWork();
  bankPlaying=false;streamPlaying=false;
  for(int frame=0;frame<5;frame++)for(int i=0;i<count;i++)Player_aPlayer[i].m_Hud._TransmissionWork();
  for(int i=0;i<count;i++)require(Player_aPlayer[i].m_Hud.m_nTransmissionState==CHud2::TRANSMISSION_STATE_IDLE&&!Player_aPlayer[i].m_Hud.antenna.visible,"natural audio end clears every HUD and antenna");
  CHud2::TransmissionShared_Start(1,"radio",1,TRUE);int stops=streamStops;
  CHud2::TransmissionShared_Stop(FALSE);
  require(streamStops==stops+1,"mirrors never stop the shared stream repeatedly");
  require(!CHud2::TransmissionShared_Start(-1,10,1,TRUE),"invalid author does not create visuals");
  require(!CHud2::TransmissionShared_Start(1,-1,1,TRUE),"invalid audio does not create mirrors");
 }
 MultiplayerMgr.coop=false;current=2;int before=streamStarts;CHud2::TransmissionShared_Start(1,"radio",1,TRUE);
 require(streamStarts==before+1&&Player_aPlayer[2].m_Hud.antenna.visible&&!Player_aPlayer[0].m_Hud.antenna.visible,"solo/PvP retains current HUD behavior");
 CHud2::TransmissionShared_Stop(FALSE);
}
'''

GATE = r'''
constexpr int ENTITY_BIT_DETPACKDROP=1,ENTITY_BIT_BOTZOMBIEBOSS=2,ENTITY_BIT_BOTZOM=4,DAMAGE_HITPOINT_TYPE_COUNT=2;
struct CEntity{const char* name="frontie";int bits=0;const char* Name()const{return name;}int TypeBits(){return bits;}};
struct CBot:CEntity{bool recruited=false;BOOL Recruit_IsRecruited(){return recruited;}};
struct CDamageProfile{float m_afUnitHitpoints[2]{1,1};struct{float m_fInnerValue=1,m_fOuterValue=1;}m_HitpointRange;BOOL m_abZeroInnerAndOuterValues[2]{};};
struct CDamageData{struct{int nDamagerPlayerIndex=-1;CEntity* pEntity=nullptr;}m_Damager;CDamageProfile* m_pDamageProfile=nullptr;float m_afDeltaHitpoints[2]{1,1};};
void fang_MemCopy(void* a,void* b,size_t n){std::memcpy(a,b,n);}
struct CMeshEntity:CEntity{int hits=0;float hitpoints=0;void InflictDamage(CDamageData* d){++hits;hitpoints+=d->m_afDeltaHitpoints[0];}};
struct CEBoomer:CMeshEntity{enum{BOOMER_FLAG_ZOMBIEBALL_ONLY=1};int m_nBoomerFlags=0;bool detOnly=false;CDamageProfile* m_pDamageOnlyProfile=nullptr;
 BOOL IsDetPackOnly(){return detOnly;}void InflictDamage(CDamageData*);};
struct CPlayer{static inline int m_nPlayerCount=4;};
struct{bool coop=true;BOOL IsLocalCoop(){return coop;}}MultiplayerMgr;
int Level_nLoadedIndex=0;struct{const char* pszWorldResName;}Level_aInfo[1]={{"WEWCcomm_02"}};
'''
GATE_CHECKS = r'''
void run(){
 CDamageProfile titan,normal;CEBoomer gate;gate.m_pDamageOnlyProfile=&titan;CDamageData damage;damage.m_pDamageProfile=&normal;
 for(int i=0;i<4;i++){damage.m_Damager.nDamagerPlayerIndex=i;int before=gate.hits;gate.InflictDamage(&damage);require(gate.hits==before+1,"each co-op player can damage the retail entrance with ordinary weapons");}
 for(int mode=0;mode<5;mode++){
  gate.name="frontie";MultiplayerMgr.coop=true;Level_aInfo[0].pszWorldResName="WEWCcomm_02";Level_nLoadedIndex=0;damage.m_Damager.nDamagerPlayerIndex=0;
  if(mode==0)MultiplayerMgr.coop=false;if(mode==1)gate.name="other";if(mode==2)Level_aInfo[0].pszWorldResName="other";if(mode==3)Level_nLoadedIndex=-1;if(mode==4)damage.m_Damager.nDamagerPlayerIndex=-1;
  int before=gate.hits;gate.InflictDamage(&damage);require(gate.hits==before,"fallback excludes solo/PvP, other doors/worlds, unloaded levels and NPC shots");
  damage.m_pDamageProfile=&titan;gate.InflictDamage(&damage);require(gate.hits==before+1,"Titan cannon remains accepted by original profile filter");damage.m_pDamageProfile=&normal;
 }
 MultiplayerMgr.coop=true;Level_nLoadedIndex=0;Level_aInfo[0].pszWorldResName="WEWCcomm_02";gate.name="frontie";damage.m_Damager.nDamagerPlayerIndex=0;
 gate.detOnly=true;float before=gate.hitpoints;gate.InflictDamage(&damage);require(gate.hitpoints==before,"detpack-only protection remains enforced");
 gate.detOnly=false;gate.m_nBoomerFlags=CEBoomer::BOOMER_FLAG_ZOMBIEBALL_ONLY;damage.m_pDamageProfile=&normal;damage.m_afDeltaHitpoints[0]=1;before=gate.hitpoints;
 gate.InflictDamage(&damage);require(gate.hitpoints==before,"zombieball-only protection remains enforced");
 Level_aInfo[0].pszWorldResName="WEWJjourn01";CBot zombie;zombie.bits=ENTITY_BIT_BOTZOM;
 for(const char* name:{"pipezom1","pipezom2"}){zombie.name=name;require(_IsCagedWastelandZombie(&zombie),"authored cage occupants remain neutral before release/recruitment");zombie.recruited=true;require(!_IsCagedWastelandZombie(&zombie),"recruited zombie uses normal team/friendship behavior");zombie.recruited=false;}
 zombie.name="other";require(!_IsCagedWastelandZombie(&zombie)&&!_IsCagedWastelandZombie(nullptr),"other zombies and null entities unaffected");
}
'''

LIQUID = r'''
struct Vec{float x=0,y=0,z=0;};struct Matrix{Vec m_vPos;};
struct CFCheckPoint{
 static inline std::vector<unsigned char> data;static inline size_t cursor=0;
 template<class T>static void SaveData(const T& t){auto* p=(const unsigned char*)&t;data.insert(data.end(),p,p+sizeof(T));}
 template<class T>static void LoadData(T& t){std::memcpy(&t,data.data()+cursor,sizeof(T));cursor+=sizeof(T);}
};
struct CEntity{Matrix m_MtxToWorld;float m_fScaleToWorld=1;int selected=-1;BOOL IsCreated(){return TRUE;}
 void ClassHierarchyRelocated(void*){}void CheckpointSaveSelect(int){}void CheckpointSaveList_AddTailAndMark(int i){selected=i;}
 BOOL CheckpointSave(){CFCheckPoint::SaveData(m_MtxToWorld);return TRUE;}void CheckpointRestore(){CFCheckPoint::LoadData(m_MtxToWorld);}};
struct Surface{Vec ext;Matrix matrix;int calls=0;void SetupVolume(Vec e,Matrix m){ext=e;matrix=m;++calls;}};
int _nNumPendingDamageForms=4;float FLoop_fPreviousLoopSecs=1;
struct CELiquidVolume:CEntity{Vec m_vExt{10,100,10};Matrix m_vMtx;float m_afRawValues[3]{},m_afShapeValues[3]{};
 float m_fHeightOffset=0,m_fDeltaHeightPerSec=0,m_fTime=0;Surface surface;Surface* m_pLiquid=&surface;
 void ClassHierarchyRelocated(void*);void ChangeLiquidHeight(float);void ChangeLiquidHeight_Time(float,float);
 void CheckpointSaveSelect(int);BOOL CheckpointSave();void CheckpointRestore();void tick();};
'''
LIQUID_CHECKS = r'''
void run(){
 CELiquidVolume liquid;liquid.m_MtxToWorld.m_vPos.y=10;liquid.ClassHierarchyRelocated(nullptr);
 require(liquid.surface.matrix.m_vPos.y==10,"initial liquid surface follows entity");
 liquid.ChangeLiquidHeight_Time(-20,4);FLoop_fPreviousLoopSecs=1;liquid.tick();
 liquid.CheckpointSaveSelect(3);liquid.CheckpointSave();float savedY=liquid.surface.matrix.m_vPos.y,savedExt=liquid.m_vExt.y;
 require(liquid.selected==3,"liquid selected into checkpoint even without script variable save");
 liquid.m_MtxToWorld.m_vPos.y=-120;liquid.ClassHierarchyRelocated(nullptr);
 require(liquid.surface.matrix.m_vPos.y==-122.5f,"attached liquid follows drained sludge parent and keeps height offset");
 FLoop_fPreviousLoopSecs=8;liquid.tick();require(liquid.m_fTime==0&&liquid.m_fHeightOffset==-10,"long frame clamps to remaining height change");
 CFCheckPoint::cursor=0;liquid.CheckpointRestore();liquid.ClassHierarchyRelocated(nullptr);
 require(liquid.surface.matrix.m_vPos.y==savedY&&liquid.m_vExt.y==savedExt&&liquid.m_fTime==3,"checkpoint restores centre, size, remaining animation and post-relocation height");
 require(_nNumPendingDamageForms==0,"restore clears stale pending liquid damage");
}
'''

VENDOR = r'''
constexpr int MAX_PLAYERS=4;
bool _abCoopEmptyOffer[MAX_PLAYERS]{};
enum BarterState_e{BARTERSTATE_NOT_IN_WORLD,BARTERSTATE_HIDDEN,BARTERSTATE_IDLE};
struct Profile{int visits=0;void VisitedBarterDroids(){++visits;}};
struct CPlayer{static inline int m_nPlayerCount=4;static CPlayer* m_pCurrent;Profile* m_pPlayerProfile=nullptr;};
CPlayer Player_aPlayer[4];CPlayer* CPlayer::m_pCurrent=&Player_aPlayer[1];
struct{bool coop=true;BOOL IsLocalCoop(){return coop;}}MultiplayerMgr;
u32 savedState=BARTERSTATE_NOT_IN_WORLD;s16 savedPoint=-1,_nBarterPtSelected=-1;bool present=false,hidden=false,_bLevelOK=true;int placed=-1;
struct CFCheckPoint{static int GetCheckPoint(){return 0;}static void SetObjectDataHandle(int){}static void LoadData(u32& n){n=savedState;}static void LoadData(s16& n){n=savedPoint;}};
int _hSaveData[1]{};
struct CBarterTable{void CheckPointRestore(){}};
struct CBarterLevel{static u32 GetNumTables(){return 0;}static CBarterTable* GetTable(int){return nullptr;}};
struct Point{const char* Name(){return "vendor";}}point;Point* _apBarterPts[4]={&point,&point,&point,&point};
BOOL bartersystem_AreInWorld(){return present;}void bartersystem_RemoveFromWorld(BOOL){present=false;}
void bartersystem_Hide(BOOL b){hidden=b;}void bartersystem_PlaceInWorldAndStartAttracting(const char*){placed=1;present=true;}
'''
VENDOR_CHECKS = r'''
void run(){
 Profile profiles[4];for(int i=0;i<4;i++)Player_aPlayer[i].m_pPlayerProfile=&profiles[i];_RememberBarterIntroduction();
 for(auto& p:profiles)require(p.visits==1,"introduction history shared by participating profiles");
 Player_aPlayer[3].m_pPlayerProfile=nullptr;_RememberBarterIntroduction();require(profiles[0].visits==2,"missing partner profile safe");
 MultiplayerMgr.coop=false;_RememberBarterIntroduction();require(profiles[1].visits==3&&profiles[0].visits==2,"solo records only current profile");
 for(int coop=0;coop<2;coop++)for(int state=0;state<3;state++){
  MultiplayerMgr.coop=coop;present=true;placed=-1;_nBarterPtSelected=1;savedPoint=state==BARTERSTATE_NOT_IN_WORLD?-1:0;savedState=state;
  bartersystem_CheckpointRestore();bool expected=state==BARTERSTATE_IDLE||(coop&&state==BARTERSTATE_NOT_IN_WORLD);
  require(present==expected,"co-op recovers encountered post-checkpoint vendors while solo and hidden state retain authored behavior");
  require(hidden==(state==BARTERSTATE_HIDDEN),"hidden checkpoint cannot accidentally reveal vendors");
 }
 MultiplayerMgr.coop=true;present=false;placed=-1;savedState=BARTERSTATE_NOT_IN_WORLD;_nBarterPtSelected=-1;bartersystem_CheckpointRestore();
 require(placed==-1,"unencountered vendors never spawn during restore");
}
'''

RENDER = r'''
enum{LM_NONE,LM4,ZMASK_EMASK_LM2,ZMASK_LM3,EMASK_LM3};
enum{SHADERTYPE_DIFFUSE,SHADERTYPE_SURFACE,SHADERTYPE_TRANSLUCENCY,SHADERTYPE_SPECULAR};
enum{FSHADERS_LIGHT_REG_ZMASK,FSHADERS_LIGHT_REG_EMASK,FSHADERS_LIGHT_REG_BUMPMAP};
enum{FSh_RENDER_MOTIF_LIGHTMAPS=1,FSh_RENDER_MULTIPASS_LIGHTING=2,FSh_RENDER_BUMP=4};
enum{FSHADERS_oBASE_ADD_rbENV=10,FSHADERS_LIQUID_LAYER_ENV=11};
int m_nLM=0,m_nLMMode=0,FSh_shaderType=0,FSh_shaderFlags=3,_nShaderID=0,_nSurfaceShaderID=0;
int _nPerPixelPoint=0,_nPerPixelSpot=0;int* FSh_pnLightMapInputRegisters=nullptr;
int FSh_pnLightInputRegisters[3]{};bool FSh_bUseFastPass=false,FSh_bShadowRender=false,_bFastShader=false,fsh_bUseExtColorStream=false;
void* _pAttenMap=(void*)1;BOOL _UseTrans(u32){return FALSE;}
u32 fsh_GetNumDiffusePasses(u32,u32,BOOL*);u32 fsh_GetNumSpecularPasses(u32,u32){return 0;}
'''
RENDER_CHECKS = r'''
void run(){
 int maps[16]{5,1};FSh_pnLightMapInputRegisters=maps;
 for(int type:{SHADERTYPE_DIFFUSE,SHADERTYPE_TRANSLUCENCY})for(int masks=0;masks<4;masks++){
  FSh_shaderType=type;FSh_pnLightInputRegisters[0]=masks&1;FSh_pnLightInputRegisters[1]=masks&2;
  fsh_bUseExtColorStream=false;fsh_GetCurrentNumPasses(nullptr);require(m_nLMMode!=LM_NONE&&m_nLM==5,"textured lightmaps remain selected when mesh supplies their UV stream");
  fsh_bUseExtColorStream=true;fsh_GetCurrentNumPasses(nullptr);require(m_nLMMode==LM_NONE&&m_nLM==0,"baked vertex colors never select stale lightmap sampling or extra lightmap passes");
 }
 FSh_bUseFastPass=true;_nSurfaceShaderID=FSHADERS_oBASE_ADD_rbENV;FSh_shaderType=SHADERTYPE_DIFFUSE;BOOL fast=FALSE;
 fsh_GetCurrentNumPasses(&fast);require(fast&&m_nLMMode==LM_NONE,"fast reflection shader respects external baked colors");
 fsh_bUseExtColorStream=false;fsh_GetCurrentNumPasses(&fast);require(m_nLMMode==LM4,"fast reflection shader retains real lightmaps");
}
'''

MOVIES = r'''
using FGameDataTableHandle_t=int;using FGameDataFileHandle_t=int;using FGameData_VarType_e=int;using cutscene_Handle_t=int;
constexpr int FGAMEDATA_INVALID_FILE_HANDLE=-1,FGAMEDATA_INVALID_TABLE_HANDLE=-1,FGAMEDATA_VAR_TYPE_STRING=1,CUTSCENE_INVALID_HANDLE=-1;
int Level_hLevelDataFile=1,_hEndingMovie=-1,intro=-1;bool haveStart=true,haveEnd=true,stringEnd=true;
int fgamedata_GetFirstTableHandle(int,const char* name){return std::strcmp(name,"EndMovie")==0?(haveEnd?2:-1):(haveStart?1:-1);}
int fgamedata_GetNumFields(int){return 1;}
const char* fgamedata_GetPtrToFieldData(int table,int,int& type){type=table==2&&!stringEnd?0:1;return table==2?"ending.bik":"intro.bik";}
int cutscene_AcquireHandle(const char* file,BOOL){return std::strcmp(file,"ending.bik")==0?2:1;}
void level_SetIntroMovieHandle(int h){intro=h;}
int level_GetEndingMovieHandle(){return _hEndingMovie;}
int starts=0;bool movieWorks=true,_bEndingMoviePlayed=false;
BOOL cutscene_Start(int){++starts;return movieWorks;}
struct{bool campaign=true;BOOL IsSinglePlayer(){return campaign;}}MultiplayerMgr;
struct CPlayer{static inline float m_fSfxVolumeCache=-1;};
float faudio_GetSfxMasterVol(){return .8f;}
BOOL finish();float volume(float);
'''
MOVIE_CHECKS = r'''
void run(){
 _LoadMovieData();require(intro==1&&_hEndingMovie==2,"authored opening and ending movies both acquired");
 haveStart=false;intro=_hEndingMovie=-1;_LoadMovieData();require(intro==-1&&_hEndingMovie==2,"ending loads independently of missing intro");
 haveEnd=false;_hEndingMovie=-1;_LoadMovieData();require(_hEndingMovie==-1,"mission without authored ending retains normal transition");
 haveEnd=true;stringEnd=false;_LoadMovieData();require(_hEndingMovie==-1,"invalid ending field ignored");
 _hEndingMovie=2;require(finish()&&starts==1,"completion starts ending before next-level schedule");
 require(!finish()&&starts==1,"return from ending cannot replay it indefinitely");
 _bEndingMoviePlayed=false;_hEndingMovie=-1;require(!finish()&&starts==1,"missing ending proceeds immediately");
 _bEndingMoviePlayed=false;_hEndingMovie=2;movieWorks=false;require(!finish()&&starts==2,"failed playback proceeds rather than softlocking completion");
 _bEndingMoviePlayed=false;MultiplayerMgr.campaign=false;require(!finish()&&starts==2,"PvP completion never plays campaign ending");
 require(std::abs(volume(1)-.4f)<.001f,"movie respects current volume before fade cache exists");
 CPlayer::m_fSfxVolumeCache=.6f;require(std::abs(volume(1)-.3f)<.001f,"ending uses pre-fade volume with six-decibel headroom");
 CPlayer::m_fSfxVolumeCache=0;require(volume(1)==0,"muted effects also mute movie audio");
}
'''

HIDDEN_LIQUID = r'''
struct CFVec3{float x=0,y=0,z=0;void Set(const CFVec3& v){*this=v;}void Sub(const CFVec3& a,const CFVec3& b){x=a.x-b.x;y=a.y-b.y;z=a.z-b.z;}
 void Unitize(){float n=std::sqrt(x*x+y*y+z*z);x/=n;y/=n;z/=n;}float Dot(const CFVec3& a){return x*a.x+y*a.y+z*a.z;}};
using CFVec3A=CFVec3;
struct CFSphere{CFVec3 m_Pos;float m_fRadius=1;};struct FVisVolume_t{};
struct CFWorldTracker{CFSphere sphere;CFSphere GetBoundingSphere(){return sphere;}};
struct CFWorldMesh:CFWorldTracker{};
#define FMATH_FABS(x) std::abs(x)
#define FMATH_CLAMP(x,a,b) (x=std::clamp(x,a,b))
constexpr float RADIUS_MUL=.75f;CFVec3 _vUp{0,1,0};
using FParticle_EmitterHandle_t=int;int particles=0,callbacks=0,planesOff=0;
int fparticle_SpawnEmitter(int,CFVec3,CFVec3*,float){return ++particles;}
void collision(CFWorldMesh*,void*){++callbacks;}
auto FLiquid_pCollisionCallback=collision;
void fsh_ActivateRenderPlane(int,BOOL on){if(!on)++planesOff;}
unsigned FVid_nFrameCounter=1;
struct CFLiquidVolume{
 static inline CFLiquidVolume* m_pSelf=nullptr;static inline float m_fMinPScale=0,m_fPScale=1;
 bool m_bRenderEnabled=false,m_bRender=true,m_bProcedural=false,m_bInteract=true,m_bUseParticles=true,lastInteract=true;
 void* m_pData=nullptr;void* m_pUserData=nullptr;int m_hCollisionParticleDef=1,m_nFrame=1,m_nRenderPlaneID=1,collisions=0;
 unsigned m_nLastFrameWork=0;CFSphere m_VolumeSphere;CFVec3 m_vExt{10,10,10};struct{CFVec3 m_vUp{0,1,0};}m_Mtx;
 BOOL Displace(CFVec3A&,float,float,BOOL,BOOL interact){lastInteract=interact;return TRUE;}
 BOOL InCurCollisionList(float,float,int){return FALSE;}static BOOL CollisionCallback(CFWorldTracker*,FVisVolume_t*);
 void HandleCollisions(){++collisions;CFWorldMesh mesh;mesh.sphere.m_Pos={0,2,0};m_pSelf=this;CollisionCallback(&mesh,nullptr);}
};
void work(CFLiquidVolume& liquid);
void run(){
 CFLiquidVolume liquid;work(liquid);require(!liquid.m_bRender&&planesOff==1&&liquid.collisions==1&&callbacks==1,"invisible liquid stops rendering but keeps collision callback");
 require(!liquid.lastInteract&&particles==0,"invisible liquid never accesses absent ripple data or emits splash visuals");
 work(liquid);require(liquid.collisions==1,"four split-screen views cannot repeat invisible hazard collision in one frame");
 ++FVid_nFrameCounter;work(liquid);require(liquid.collisions==2,"invisible hazard collision continues next frame");
 liquid.m_bRenderEnabled=true;liquid.m_pData=&liquid;liquid.HandleCollisions();require(liquid.lastInteract&&particles==1,"visible liquid retains ripple and splash behavior");
}
'''

def main():
    def source(path): return (ROOT / path).read_text()
    hud = source('ma/App/ma/Hud2.cpp')
    code = BASE + '\nnamespace radio {\n' + RADIO
    for signature in ('void CHud2::TransmissionShared_Stop(',
                      'BOOL CHud2::TransmissionShared_Start( TransmissionAuthor_e nAuthorIndex, FSndFx',
                      'BOOL CHud2::TransmissionShared_Start( TransmissionAuthor_e nAuthorIndex, cchar',
                      'BOOL CHud2::TransmissionMsg_Start( TransmissionAuthor_e nAuthorIndex, FSndFx',
                      'BOOL CHud2::TransmissionMsg_Start( TransmissionAuthor_e nAuthorIndex, cchar',
                      'void CHud2::_TransmissionMsg_Start(', 'void CHud2::TransmissionMsg_Stop(',
                      'void CHud2::_TransmissionWork('):
        code += method(hud, signature)
    code += RADIO_CHECKS + '}\nnamespace gate {\n' + GATE
    code += method(source('ma/App/ma/eboomer.cpp'), 'static BOOL _CoopCanBreachEntrance(')
    code += method(source('ma/App/ma/eboomer.cpp'), 'void CEBoomer::InflictDamage(')
    code += method(source('ma/App/ma/Ai/AIGameUtils.cpp'), 'static BOOL _IsWastelandCageOccupant(')
    code += method(source('ma/App/ma/Ai/AIGameUtils.cpp'), 'static BOOL _IsCagedWastelandZombie(')
    code += GATE_CHECKS + '}\nnamespace liquid {\n' + LIQUID
    liquid = source('ma/App/ma/ELiquidVolume.cpp')
    for signature in ('void CELiquidVolume::ClassHierarchyRelocated(', 'void CELiquidVolume::ChangeLiquidHeight(',
                      'void CELiquidVolume::ChangeLiquidHeight_Time(', 'void CELiquidVolume::CheckpointSaveSelect(',
                      'BOOL CELiquidVolume::CheckpointSave(', 'void CELiquidVolume::CheckpointRestore('):
        code += method(liquid, signature)
    code += 'void CELiquidVolume::tick(){' + method(liquid, 'if (m_fTime > 0.0f)') + '}\n'
    code += LIQUID_CHECKS + '}\nnamespace vendor {\n' + VENDOR
    barter = source('ma/App/ma/BarterSystem.cpp')
    code += method(barter, 'static void _RememberBarterIntroduction(')
    code += method(barter, 'void bartersystem_CheckpointRestore(')
    code += VENDOR_CHECKS + '}\nnamespace render {\n' + RENDER
    render = source('ma/Lib/Fang2/dx/fdx8sh.cpp')
    code += method(render, 'u32 fsh_GetNumDiffusePasses(')
    code += method(render, 'FINLINE u32 fsh_GetCurrentNumPasses(')
    code += RENDER_CHECKS + '}\nnamespace movies {\n' + MOVIES
    code += method(source('ma/App/ma/level.cpp'), 'static void _LoadMovieData( void ) {')
    code += 'BOOL finish(){' + method(source('ma/App/ma/game.cpp'), 'if( MultiplayerMgr.IsSinglePlayer() && !_bEndingMoviePlayed )') + 'return FALSE;}\n'
    cutscene = source('ma/App/ma/cutscene.cpp')
    volume_start = cutscene.index('\tconst f32 fUserVolume =')
    volume_end = cutscene.index('#endif', volume_start)
    code += 'float volume(float fNormalizedVolume){' + cutscene[volume_start:volume_end] + 'return fNormalizedVolume;}\n'
    code += MOVIE_CHECKS + '}\n'
    code += 'namespace hidden {\n' + HIDDEN_LIQUID
    liquid_engine = source('ma/Lib/Fang2/fliquid.cpp')
    code += method(liquid_engine, 'BOOL CFLiquidVolume::CollisionCallback(')
    code += 'void work(CFLiquidVolume& liquid){CFLiquidVolume* m_LiquidVolumes[]={&liquid};for(int i=0;i<1;i++){' + method(liquid_engine, 'if( !m_LiquidVolumes[i]->m_bRenderEnabled )') + '}}\n'
    code += '}\nint main(){radio::run();gate::run();liquid::run();vendor::run();render::run();movies::run();hidden::run();std::printf("PASS: %d offline playthrough checks; game not launched.\\n",checks);}\n'
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'playthrough.cpp').write_text(code)
    (OUT / 'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(playthrough LANGUAGES CXX)\nadd_executable(playthrough playthrough.cpp)\ntarget_compile_features(playthrough PRIVATE cxx_std_17)\n')
    for command in (['cmake', '-S', str(OUT), '-B', str(OUT / 'out'), '-A', 'Win32'],
                    ['cmake', '--build', str(OUT / 'out'), '--config', 'Release']):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode: raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / 'out/Release/playthrough.exe')], check=True)

if __name__ == '__main__': main()

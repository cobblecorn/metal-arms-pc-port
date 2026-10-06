"""Offline production-method checks for scene grounding, lift restore and liquid state.

Never launches ma_port or touches profiles. Windows/MSVC; outputs stay under build/.
"""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-coop-scene-restore"


def method(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth, end = 1, brace + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


FIXTURE = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>
using BOOL=int; using u32=unsigned; using s32=int; using u16=unsigned short;
using u64=unsigned long long; using f32=float;
constexpr BOOL TRUE=1,FALSE=0;
u64 FLoop_nTotalLoopTicks=100;
#define FANG_WINGC 1
#define FANG_PLATFORM_DX 1
#define FASSERT(x) ((void)0)
#define DEVPRINTF(...) ((void)0)
struct CFVec3 {float x=0,y=0,z=0; void Zero(){x=y=z=0;} void Set(float a,float b,float c){x=a;y=b;z=c;}};
struct CFMtx43A {
 CFVec3 m_vX{1,0,0},m_vY{0,1,0},m_vZ{0,0,1},m_vPos;
 static CFMtx43A m_IdentityMtx;
 void Identity(){*this=m_IdentityMtx;}
};
CFMtx43A CFMtx43A::m_IdentityMtx;
struct CFCheckPoint {
 static inline std::vector<unsigned char> data; static inline size_t cursor=0;
 template<class T> static void SaveData(const T& x){auto p=(const unsigned char*)&x;data.insert(data.end(),p,p+sizeof x);}
 template<class T> static void LoadData(T& x){if(cursor+sizeof x>data.size())std::abort();std::memcpy(&x,data.data()+cursor,sizeof x);cursor+=sizeof x;}
};
struct Anim {float phase=0;void UpdateUnitTime(float x){phase=x;}};
struct CEntity {BOOL CheckpointSave(){return TRUE;}void CheckpointRestore(){}BOOL IsInWorld(){return TRUE;}};
struct CMeshEntity:CEntity {
 u64 m_nLastAnimTicks=0,m_nLastFlipTicks=0;float m_fUserAnim_SpeedMult=0;
 u32 m_nUserAnim_Flip=0,m_nMeshEntityFlags=0,m_nMeshFlipIndex=0;
 s32 m_nUserAnim_CurrentInstIndex=0; Anim* m_pUserAnim_CurrentInst=nullptr;
 int selectedMesh=0,computes=0;
 BOOL CheckpointSave();void CheckpointRestore();
 void SelectMesh(u32 i,BOOL,BOOL){selectedMesh=i;m_nMeshFlipIndex=i;m_nLastFlipTicks=FLoop_nTotalLoopTicks;}
 Anim* UserAnim_GetCurrentInst(){return m_pUserAnim_CurrentInst;}
 void UserAnim_SetSpeedMult(float s){m_fUserAnim_SpeedMult=s;}
 void UserAnim_Pause(BOOL p){m_nMeshEntityFlags=p?1:0;}
 void ComputeMtxPalette(BOOL){computes++;}
};
using FSndFx_FxHandle_t=int; struct CFAudioEmitter{}; CFAudioEmitter emitter;
constexpr int FAudio_EmitterDefaultPriorityLevel=0;
#define FSNDFX_ALLOCNPLAY3D(...) (&emitter)
struct Portal{bool open=false;void SetOpenState(BOOL b){open=b;}};
struct CDoorEntity:CMeshEntity {
 enum ActionState_e {DOORSTATE_ZERO,DOORSTATE_ONE,DOORSTATE_ZEROTOONE,DOORSTATE_ONETOZERO};
 enum GotoReason_e {GOTOREASON_UNKNOWN,GOTOREASON_PICKUP,GOTOREASON_DESTINATION};
 enum {DOORMOVETYPE_NONE,DOORMOVETYPE_LINE,MOVETYPE_NONE,MOVETYPE_BONETRANS,MOVETYPE_ANIMATION,
       USERTYPE_DOOR,USERTYPE_LIFT,NO_COLLISION_TEST};
 float m_fPickupCntDn=0,m_fTimeOpen=0,m_fUnitPos=0,m_fUnitPosMapped=0;
 GotoReason_e m_eCurGotoReason=GOTOREASON_UNKNOWN;BOOL m_bUnoccupiedLastFrame=0,m_bLocked=0,m_bStartOpen=0;
 ActionState_e m_eState=DOORSTATE_ZERO;
 int m_eDoorMoveType=DOORMOVETYPE_LINE,m_eBoneMoveType=MOVETYPE_NONE,m_eUserType=USERTYPE_LIFT;
 BOOL m_bIsPortal=0;Portal* m_pPortal=nullptr;float m_afAnimSpeedMult[2]{2,3};
 int m_hSoundCloseLoop=2,m_hSoundOpenLoop=1;
 CFAudioEmitter *m_pSoundCloseLoopEmitter=nullptr,*m_pSoundOpenLoopEmitter=nullptr;
 CFMtx43A matrix;int lineUpdates=0,boneUpdates=0,stops=0;
 struct Sphere{float m_fRadius=1;}sphere;
 Sphere GetBoundingSphere_WS(){return sphere;}
 CFMtx43A* MtxToWorld(){return &matrix;}const char* Name(){return "jail_lift";}
 BOOL CheckpointSave();void CheckpointRestore();
 void StopDoorLoopSounds(){m_pSoundCloseLoopEmitter=m_pSoundOpenLoopEmitter=nullptr;stops++;}
 void _UpdateLineDoorPosition(int){lineUpdates++;matrix.m_vPos.y=10*m_fUnitPosMapped;}
 void DoBoneMovement(){boneUpdates++;}
 void SnapToPos(u32){std::abort();} // Restoration must retain saved timers, not take the arrival path.
};
constexpr int COLLSTATE_FLOOR=1,BOTJUMPSTATE_AIR=1;
struct CBotGlitch {
 CFMtx43A matrix;CFVec3 velocity, m_ImpulseVelocity_WS;void* m_pCableHook=nullptr;
 int m_nJumpState=0,floor=0;bool air=false,jumping=false,sticky=false,parent=false,walkOff=false;
 int animations=0,stops=0,relocations=0;CFVec3 nextDelta;
 CFMtx43A* MtxToWorld(){return &matrix;}
 BOOL IsInAir(){return air;} BOOL IsJumping(){return jumping;}
 int GetCollisionState(){return floor;}BOOL GetStickyEntity(){return sticky;}BOOL GetParent(){return parent;}
 void ReleaseCable(){m_pCableHook=nullptr;air=true;floor=0;}
 void ZeroVelocity(){velocity.Zero();stops++;}
 void Relocate_RotXlatFromUnitMtx_WS(CFMtx43A* m,BOOL){matrix=*m;relocations++;}
 void Work(){
  animations++;matrix.m_vPos.x+=nextDelta.x;matrix.m_vPos.y+=nextDelta.y;matrix.m_vPos.z+=nextDelta.z;
  if(walkOff){air=true;floor=0;}
  if(air||jumping||!floor){velocity.y-=1;matrix.m_vPos.y+=velocity.y;
   if(matrix.m_vPos.y<=0){matrix.m_vPos.y=0;floor=COLLSTATE_FLOOR;air=jumping=false;velocity.y=0;}}
 }
 void PortStopForCoopScene();void PortWorkForCoopScene();
};
constexpr int D3DVBF_DISABLE=0;
#define D3DTS_WORLDMATRIX(x) (x)
CFMtx43A shaderWorld,fixedWorld;int activeView=0,renderView=0,vertexBlend=1;
void fdx8xfm_SetCustomDXMatrix(int,const CFMtx43A* m,BOOL shader){(shader?shaderWorld:fixedWorld)=*m;}
void fdx8xfm_SetViewDXMatrix(BOOL){renderView=activeView;}
void fdx8_SetRenderState_VERTEXBLEND(int v){vertexBlend=v;}
void fxfm_SetViewAndWorldSpaceModelMatrices(){shaderWorld.Identity();vertexBlend=0;}
struct CFTexInst{};struct CFColorRGB{};struct fsh_Render_Plane_t{};
fsh_Render_Plane_t _aRenderPlanes[4];BOOL bRenderPlane=TRUE;
'''

CHECKS = r'''
int checks=0;
void require(bool b,const char* message){checks++;if(!b){std::printf("FAIL: %s\n",message);std::exit(1);}}
int main(){
 for(int player=0;player<4;player++){
  CBotGlitch bot;bot.air=true;bot.matrix.m_vPos={0,12,0};
  bot.PortWorkForCoopScene();
  require(bot.matrix.m_vPos.y<12&&bot.stops==0&&bot.relocations==0,"airborne spectator keeps gravity");
  for(int frame=0;frame<12;frame++)bot.PortWorkForCoopScene();
  require(bot.floor&&bot.matrix.m_vPos.y==0&&bot.animations==13,"spectator lands and continues animation work");
  bot.nextDelta={2,.2f,3};bot.PortWorkForCoopScene();
  require(bot.matrix.m_vPos.x==0&&bot.matrix.m_vPos.z==0&&bot.matrix.m_vPos.y==.2f,"idle hold preserves floor correction");
  bot.sticky=true;bot.PortWorkForCoopScene();
  require(bot.matrix.m_vPos.x==2&&bot.matrix.m_vPos.z==3,"moving platform carries spectator");
  bot.sticky=false;bot.parent=true;bot.PortWorkForCoopScene();
  require(bot.matrix.m_vPos.x==4&&bot.matrix.m_vPos.z==6,"attached platform owns position");
  CBotGlitch cable;cable.matrix.m_vPos.y=12;cable.m_pCableHook=&cable;
  cable.PortWorkForCoopScene();require(!cable.m_pCableHook&&cable.air&&cable.matrix.m_vPos.y<12,"released cable falls normally");
  CBotGlitch ledge;ledge.floor=COLLSTATE_FLOOR;ledge.matrix.m_vPos.y=12;ledge.walkOff=true;
  ledge.PortWorkForCoopScene();require(ledge.air&&ledge.relocations==0,"lost support cannot be pinned to old floor");
 }
 for(int state=0;state<4;state++)for(int bone:{CDoorEntity::MOVETYPE_NONE,CDoorEntity::MOVETYPE_BONETRANS,CDoorEntity::MOVETYPE_ANIMATION}){
  for(int previous=0;previous<4;previous++){
   Anim animation;Portal portal;CDoorEntity lift;
   lift.m_eState=(CDoorEntity::ActionState_e)state;lift.m_eBoneMoveType=bone;
   lift.m_fUnitPos=state==0?0:state==1?1:.4f;lift.m_fUnitPosMapped=state<2?lift.m_fUnitPos:.35f;
   lift.m_fTimeOpen=7;lift.m_fPickupCntDn=.75f;lift.m_bLocked=1;
   lift.m_pUserAnim_CurrentInst=&animation;lift.m_nLastAnimTicks=100;lift.m_nLastFlipTicks=90;
   lift.m_nMeshFlipIndex=state==1?1:0;lift.m_bIsPortal=true;lift.m_pPortal=&portal;
   CFCheckPoint::data.clear();CFCheckPoint::cursor=0;lift.CheckpointSave();
   lift.m_eState=(CDoorEntity::ActionState_e)previous;lift.m_fTimeOpen=0;lift.m_fPickupCntDn=0;
   lift.m_fUnitPos=lift.m_fUnitPosMapped=1;lift.m_bLocked=0;
   lift.m_nLastAnimTicks=999999;lift.m_nLastFlipTicks=999999;lift.m_nMeshFlipIndex=3;
   lift.m_fUserAnim_SpeedMult=-99;lift.m_nMeshEntityFlags=0;animation.phase=1;
   lift.CheckpointRestore();
   const bool moving=state>=2;
   require(lift.m_eState==state&&lift.m_fTimeOpen==7&&lift.m_fPickupCntDn==.75f&&lift.m_bLocked,"saved lift state, lock and timers survive restore");
   require(lift.m_nLastAnimTicks==100&&lift.m_nLastFlipTicks==100,"animation clocks rewind with checkpoint");
   require(lift.selectedMesh==(state==1?1:0),"selected mesh restored");
   require(lift.matrix.m_vPos.y==10*lift.m_fUnitPosMapped&&lift.lineUpdates==1,"physical lift pose matches saved position for all prior states");
   require(portal.open==(state!=0),"portal follows restored door state");
   require((lift.m_pSoundOpenLoopEmitter!=nullptr)==moving&&!lift.m_pSoundCloseLoopEmitter,"lift loop resumes only while moving");
   if(bone==CDoorEntity::MOVETYPE_ANIMATION){
    require(animation.phase==lift.m_fUnitPos&&lift.computes==1,"animation pose rebuilt immediately");
    require(bool(lift.m_nMeshEntityFlags)==!moving,"only moving checkpoint animations resume");
    if(moving)require(lift.m_fUserAnim_SpeedMult==(state==2?2:-3),"restored animation direction matches logical motion");
   }else if(bone==CDoorEntity::MOVETYPE_BONETRANS)require(lift.boneUpdates==1,"translated bones rebuilt");
   require(CFCheckPoint::cursor==CFCheckPoint::data.size(),"checkpoint save/load layouts match");
  }
 }
 for(int state=2;state<4;state++){
  CDoorEntity door;door.m_eUserType=CDoorEntity::USERTYPE_DOOR;
  door.m_eState=(CDoorEntity::ActionState_e)state;door.m_fUnitPos=door.m_fUnitPosMapped=.4f;
  CFCheckPoint::data.clear();CFCheckPoint::cursor=0;door.CheckpointSave();door.CheckpointRestore();
  require(bool(door.m_pSoundCloseLoopEmitter)==(state==3)&&bool(door.m_pSoundOpenLoopEmitter)==(state==2),"restored door loop matches opening or closing");
 }
 CFTexInst texture;CFVec3 forward{0,1,0};
 for(int view=0;view<4;view++){
  activeView=view;shaderWorld.m_vPos={50,40,30};fixedWorld=shaderWorld;vertexBlend=1;
  planeSetup(0,1,1,0,0,&texture,1,1);
  require(fixedWorld.m_vPos.x==0&&shaderWorld.m_vPos.x==0&&vertexBlend==0&&renderView==view,"surface resets both transform paths and current viewport");
  _MoveOut(.1f,forward);require(fixedWorld.m_vY.y==1.1f&&shaderWorld.m_vY.y==1.1f,"waterfall layers set both transform paths");
  fixedWorld.m_vPos={-10,-20,-30};shaderWorld=fixedWorld;vertexBlend=1;
  meshSetup(0,0,0,nullptr,nullptr,nullptr,1,forward,nullptr,nullptr,&texture);
  require(fixedWorld.m_vPos.x==0&&fixedWorld.m_vY.y==1&&shaderWorld.m_vPos.x==0&&vertexBlend==0&&renderView==view,"next waterfall discards bot and previous layer transforms");
 }
 std::printf("PASS: %d offline scene/lift/liquid checks; game not launched.\n",checks);
}
'''


def main():
    bot = (ROOT / "ma/App/ma/botglitch.cpp").read_text()
    door = (ROOT / "ma/App/ma/Door.cpp").read_text()
    mesh = (ROOT / "ma/App/ma/meshentity.cpp").read_text()
    liquid = (ROOT / "ma/Lib/Fang2/dx/fdx8sh.cpp").read_text()
    code = FIXTURE
    for source, signatures in ((mesh, ("BOOL CMeshEntity::CheckpointSave(", "void CMeshEntity::CheckpointRestore(")),
                               (door, ("BOOL CDoorEntity::CheckpointSave(", "void CDoorEntity::CheckpointRestore(")),
                               (bot, ("void CBotGlitch::PortStopForCoopScene(", "void CBotGlitch::PortWorkForCoopScene("))):
        code += "\n".join(method(source, signature) for signature in signatures)
    # Execute the production entry setup before shader/material-specific draw calls.
    plane = method(liquid, "void fsh_RenderPlane(")
    plane = plane[:plane.index("FDX8_SetVertexShader(")].replace("fsh_RenderPlane(", "planeSetup(")
    waterfall = method(liquid, "void fsh_DrawLiquidMesh(")
    waterfall = waterfall[:waterfall.index("FDX8_pDev->SetVertexShaderConstant(")].replace("fsh_DrawLiquidMesh(", "meshSetup(")
    code += plane + "\n#endif\n}\n" + waterfall + "\n#endif\n}\n"
    code += method(liquid, "void _MoveOut(") + CHECKS
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "scene.cpp").write_text(code)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(coop_scene_check LANGUAGES CXX)\n"
        "add_executable(coop_scene_check scene.cpp)\ntarget_compile_features(coop_scene_check PRIVATE cxx_std_17)\n")
    for command in (["cmake", "-S", str(OUT), "-B", str(OUT / "out"), "-A", "Win32"],
                    ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / "out/Release/coop_scene_check.exe")], check=True)


if __name__ == "__main__":
    main()

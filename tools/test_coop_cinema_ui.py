"""Offline production camera-sharing and co-op panel layout checks. Never runs the game."""
from pathlib import Path
import subprocess
from test_coop_checkpoint_rat import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'build/test-coop-cinema-ui'
FIXTURE = r'''
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>
using BOOL=int;using u32=unsigned;using s32=int;using f32=float;
constexpr BOOL TRUE=1,FALSE=0;
#define FANG_WINGC 1
#define MA_PC_INPUT 1
#define FASSERT(x) ((void)0)
#define PLAYER_CAM(n) (n)
#define FMATH_MAX(a,b) std::max(a,b)
#define FMATH_MIN(a,b) std::min(a,b)
constexpr int MAX_PLAYERS=4;
int checks=0;void require(bool b,const char* m){++checks;if(!b){std::printf("FAIL: %s\n",m);std::exit(1);}}
struct CFVec3{float x=0,y=0,z=0;CFVec3& v3;CFVec3(float a=0,float b=0,float c=0):x(a),y(b),z(c),v3(*this){}
 CFVec3(const CFVec3& a):x(a.x),y(a.y),z(a.z),v3(*this){}CFVec3& operator=(const CFVec3& a){x=a.x;y=a.y;z=a.z;return *this;}
 void Set(float a,float b,float c){x=a;y=b;z=c;}};
struct CFXfm{struct{CFVec3 m_vPos;}m_MtxR;int frame=0;static inline int drawnFrame=0;void InitStackWithView()const{drawnFrame=frame;}};
struct FViewport_t{float fTanHalfFOVY=.5f,fHalfFOVY=std::atan(.5f),fHalfFOVX=1,fNearZ=.2f,fFarZ=1000;
 unsigned nWidth=640,nHeight=480,nScreenLeftX=0,nScreenTopY=0;void* pTexDef=nullptr;};
float fmath_Atan(float y,float x){return std::atan2(y,x);}
void fviewport_InitPersp(FViewport_t* p,float x,float y,float nearZ,float farZ,u32 left,u32 top,u32 w,u32 h,void* tex){
 p->fHalfFOVX=x;p->fHalfFOVY=y;p->fTanHalfFOVY=std::tan(y);p->fNearZ=nearZ;p->fFarZ=farZ;p->nScreenLeftX=left;p->nScreenTopY=top;p->nWidth=w;p->nHeight=h;p->pTexDef=tex;}
FViewport_t* activeViewport=nullptr;void fviewport_SetActive(FViewport_t* p){activeViewport=p;}
int _nLastCamSetup=0,_nLastCamIdx=0;
struct CFCamera{
 CFCamera* m_pViewSource=nullptr;FViewport_t borrowed,own;FViewport_t* m_pViewViewport=&borrowed;
 struct{bool m_bInited=true;FViewport_t* m_pViewport=nullptr;CFXfm m_Xfm;}m_CameraData;CFXfm m_FinalXfm;
 CFCamera(){m_CameraData.m_pViewport=&own;}
 BOOL SetViewSource(CFCamera*);const FViewport_t* GetViewport();const CFXfm* GetFinalXfm();const CFXfm* GetXfmWithoutShake()const;
 const CFVec3* GetPos();void SetupCameraAndViewport();
};
CFCamera cameras[4];bool _bSystemInited=true;unsigned _nNumCameras=4;
u32 fcamera_GetCameraCount();CFCamera* fcamera_GetCameraByIndex(int n){return &cameras[n];}
using GameCamType_e=int;
constexpr int GAME_CAM_PLAYER_1=0,GAME_CAM_TYPE_NOT_IN_USE=0,GAME_CAM_TYPE_ROBOT_3RD=1,GAME_CAM_TYPE_MANUAL=2,GAME_CAM_TYPE_CUTSCENE=3,GAME_CAM_TYPE_VEHICLE=4;
int cameraTypes[4]{1,4,4,1};void gamecam_GetCameraManByIndex(int n,int* type){*type=cameraTypes[n];}
constexpr int ENTITY_BIT_BOT=1,CONTROLMODE_NORMAL=0,CONTROLMODE_LETTERBOX=1,CONTROLMODE_LETTERBOXFF=2,CONTROLMODE_BARTERSYSTEM=3;
struct CEntity{bool inWorld=true;int TypeBits(){return ENTITY_BIT_BOT;}BOOL IsInWorld(){return inWorld;}};
struct CBot:CEntity{bool dead=false;BOOL IsDeadOrDying(){return dead;}};
struct CPlayer{static inline int m_nPlayerCount=2;CEntity* m_pEntityCurrent=nullptr;bool control=true;FViewport_t* m_pViewportPersp3D=nullptr;BOOL HasEntityControl(){return control;}};
CPlayer Player_aPlayer[4];int _aeControlMode[4]{},_nCoopStoryPlayer=0;
BOOL MAScript_bPortScriptCameraActive=FALSE;int game_GetStoryPlayerIndex(){return _nCoopStoryPlayer;}
struct{bool coop=true;BOOL IsLocalCoop(){return coop;}}MultiplayerMgr;
struct CPauseScreen{static inline bool paused=false;static BOOL IsActive(){return paused;}};
struct CSpyVsSpy{static inline bool individual=false,packing=false;static BOOL CoopIndividualViews(){return individual;}static BOOL CoopPackingWaiting(){return packing;}};
using cell=int;struct AMX{};
#define AMX_NATIVE_CALL
#define SCRIPT_CHECK_NUM_PARAMS(...) ((void)0)
#define SCRIPT_ERROR(...) ((void)0)
CBot* cameraTargets[4]{};
void gamecam_SwitchPlayerTo3rdPersonCamera(int n,CBot* bot){cameraTypes[n]=GAME_CAM_TYPE_ROBOT_3RD;cameraTargets[n]=bot;}
CPlayer* _ScriptPlayer(){return &Player_aPlayer[_nCoopStoryPlayer];}
struct CMAST_CamWrapper{static inline BOOL m_bCamInitted=FALSE;static cell Cam_Deactivate(AMX*,cell*);};

struct CFColorRGBA{float r,g,b,a;CFColorRGBA(float x,float y,float z,float w):r(x),g(y),b(z),a(w){}};
struct Quad{CFVec3 a,b,c,d;CFColorRGBA color;};std::vector<Quad> quads;
void fdraw_SolidQuad(CFVec3* a,CFVec3* b,CFVec3* c,CFVec3* d,CFColorRGBA* color){quads.push_back({*a,*b,*c,*d,*color});}
using FDrawCullDir_e=int;int cull=7,pushes=0;
constexpr int FRENDERER_DRAW=1,FDRAW_DEPTHTEST_ALWAYS=1,FDRAW_COLORFUNC_DECAL_AI=1,FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE=1,FDRAW_CULLDIR_NONE=0;
void frenderer_Push(int,void*){++pushes;}void frenderer_Pop(){--pushes;}
void fdraw_Depth_EnableWriting(BOOL){}void fdraw_Depth_SetTest(int){}void fdraw_SetTexture(void*){}void fdraw_Color_SetFunc(int){}void fdraw_Alpha_SetBlendOp(int){}
int fdraw_GetCullDir(){return cull;}void fdraw_SetCullDir(int n){cull=n;}
'''
CHECKS = r'''
int main(){
 CBot bots[4];for(int n=0;n<4;n++){Player_aPlayer[n].m_pEntityCurrent=&bots[n];cameras[n].m_FinalXfm.frame=10+n;cameras[n].m_CameraData.m_Xfm.frame=20+n;}
 for(int count=2;count<=4;++count)for(int actor=1;actor<count;++actor)for(int revived=0;revived<2;++revived) {
  CPlayer::m_nPlayerCount=count;_nCoopStoryPlayer=actor;bots[0].dead=!revived;
  CMAST_CamWrapper::m_bCamInitted=TRUE;MAScript_bPortScriptCameraActive=TRUE;
  cameraTypes[0]=GAME_CAM_TYPE_MANUAL;
  CMAST_CamWrapper::Cam_Deactivate(nullptr,nullptr);
  require(cameraTargets[0]==&bots[0],"manual scene returns P1 camera to its owner even when another player acts the scene");
  require(!MAScript_bPortScriptCameraActive&&!CMAST_CamWrapper::m_bCamInitted,"manual scene releases its sharing flag and initialized state");
  bots[0].dead=false;require(cameraTargets[0]==&bots[0],"P1 revival retains its own follow target");
 }
 MultiplayerMgr.coop=false;_nCoopStoryPlayer=1;CMAST_CamWrapper::m_bCamInitted=TRUE;
 CMAST_CamWrapper::Cam_Deactivate(nullptr,nullptr);require(cameraTargets[0]==&bots[1],"retail solo/PvP camera target selection retained");MultiplayerMgr.coop=true;
 for(int count=2;count<=4;count++)for(int actor=0;actor<count;actor++)for(int scripted=0;scripted<3;scripted++){
  CPlayer::m_nPlayerCount=count;_nCoopStoryPlayer=actor;
  cameraTypes[0]=scripted==2?GAME_CAM_TYPE_CUTSCENE:scripted==1?GAME_CAM_TYPE_MANUAL:GAME_CAM_TYPE_ROBOT_3RD;
  MAScript_bPortScriptCameraActive=scripted==1;
  for(int n=0;n<4;n++){_aeControlMode[n]=CONTROLMODE_NORMAL;Player_aPlayer[n].control=true;bots[n].dead=false;}
  if(!scripted)_aeControlMode[actor]=CONTROLMODE_LETTERBOX;
  bots[(actor+1)%count].dead=true;_CoopWatchCamerasWork();int source=scripted?0:actor;
  for(int n=0;n<count;n++){
   require(cameras[n].GetFinalXfm()==&cameras[source].m_FinalXfm,"living, dead and vehicle viewers use exact live cinematic transform");
   require(cameras[n].GetXfmWithoutShake()==&cameras[source].m_CameraData.m_Xfm,"unshaken camera queries share source too");
   require(cameras[n].GetPos()==&cameras[source].m_CameraData.m_Xfm.m_MtxR.m_vPos.v3,"camera position queries match scene view");
   require(cameras[n].m_CameraData.m_Xfm.frame==20+n,"borrowing a scene view never changes owned camera state");
   cameras[n].SetupCameraAndViewport();require(CFXfm::drawnFrame==cameras[source].m_FinalXfm.frame&&activeViewport==Player_aPlayer[n].m_pViewportPersp3D,"scene render and player perspective pointer use same borrowed viewport");
  }
  cameras[source].m_FinalXfm.frame+=100;for(int n=0;n<count;n++)require(cameras[n].GetFinalXfm()->frame==cameras[source].m_FinalXfm.frame,"source work after link setup appears in same frame, including shake/roll");
  CPauseScreen::paused=true;_CoopWatchCamerasWork();for(int n=0;n<count;n++)require(!cameras[n].m_pViewSource,"pause clears borrowed view without replacing camera controller");CPauseScreen::paused=false;
  _CoopWatchCamerasWork();_CoopWatchCamerasReset();for(int n=0;n<count;n++)require(cameras[n].GetFinalXfm()==&cameras[n].m_FinalXfm&&Player_aPlayer[n].m_pViewportPersp3D==&cameras[n].own,"skip/restart/unload reset restores each player's own view and projection");
 }
 // A packed actor has firing controls back while partners still watch the story.
 for(int count=2;count<=4;++count){
  CPlayer::m_nPlayerCount=count;_nCoopStoryPlayer=0;MAScript_bPortScriptCameraActive=false;
  for(int n=0;n<4;++n){cameraTypes[n]=GAME_CAM_TYPE_ROBOT_3RD;_aeControlMode[n]=CONTROLMODE_NORMAL;Player_aPlayer[n].control=true;bots[n].dead=false;}
  CSpyVsSpy::packing=true;_CoopWatchCamerasWork();
  for(int n=1;n<count;++n)require(cameras[n].m_pViewSource==&cameras[0],"packed story body retains shared camera with P1 controls restored");
  CSpyVsSpy::packing=false;_CoopWatchCamerasWork();
  for(int n=0;n<count;++n)require(!cameras[n].m_pViewSource,"box exit releases every borrowed camera");
  CSpyVsSpy::individual=true;_aeControlMode[0]=CONTROLMODE_LETTERBOX;_CoopWatchCamerasWork();
  for(int n=0;n<count;++n)require(!cameras[n].m_pViewSource,"instructor individual views remain independent");
  CSpyVsSpy::individual=false;
 }
 // Start linked, enter a private shop, then leave and resume real cinematics.
 for(int count=2;count<=4;count++)for(int shopper=0;shopper<count;shopper++)for(int staleScript=0;staleScript<2;staleScript++){
  CPlayer::m_nPlayerCount=count;_nCoopStoryPlayer=shopper;
  for(int n=0;n<4;n++){_aeControlMode[n]=CONTROLMODE_NORMAL;Player_aPlayer[n].control=true;bots[n].dead=false;cameraTypes[n]=GAME_CAM_TYPE_ROBOT_3RD;}
  MAScript_bPortScriptCameraActive=false;
  _aeControlMode[shopper]=CONTROLMODE_LETTERBOX;_CoopWatchCamerasWork();
  require(cameras[(shopper+1)%count].m_pViewSource==&cameras[shopper],"cinematic link present before shopping transition");
  _aeControlMode[shopper]=CONTROLMODE_BARTERSYSTEM;Player_aPlayer[shopper].control=false;cameraTypes[shopper]=GAME_CAM_TYPE_MANUAL;
  MAScript_bPortScriptCameraActive=staleScript;
  // With a non-P1 shopper, no script controls the P1 camera.
  _CoopWatchCamerasWork();
  for(int n=0;n<count;n++){
   require(!cameras[n].m_pViewSource&&cameras[n].GetFinalXfm()==&cameras[n].m_FinalXfm,"vendor view never borrowed, including stale script flag");
   require(Player_aPlayer[n].m_pViewportPersp3D==&cameras[n].own,"each player regains own projection on entering shop");
   require(Player_aPlayer[n].control==(n!=shopper),"only shopper stays held; partners retain movement controls");
  }
  _aeControlMode[shopper]=CONTROLMODE_NORMAL;Player_aPlayer[shopper].control=true;cameraTypes[shopper]=GAME_CAM_TYPE_ROBOT_3RD;MAScript_bPortScriptCameraActive=false;
  _CoopWatchCamerasWork();for(int n=0;n<count;n++)require(!cameras[n].m_pViewSource,"shop exit restores independent gameplay");
  cameraTypes[0]=GAME_CAM_TYPE_MANUAL;MAScript_bPortScriptCameraActive=true;
  _CoopWatchCamerasWork();for(int n=1;n<count;n++)require(cameras[n].m_pViewSource==&cameras[0],"next real scripted camera still shared after shop exit");
 }
 cameraTypes[0]=GAME_CAM_TYPE_ROBOT_3RD;MAScript_bPortScriptCameraActive=false;_nCoopStoryPlayer=0;for(int n=0;n<4;n++){_aeControlMode[n]=0;Player_aPlayer[n].control=true;}
 _CoopWatchCamerasWork();require(!cameras[1].m_pViewSource,"ordinary co-op gameplay remains independent");
 _aeControlMode[0]=CONTROLMODE_LETTERBOX;MultiplayerMgr.coop=false;_CoopWatchCamerasWork();require(!cameras[1].m_pViewSource,"solo/PvP does not borrow cinematic views");MultiplayerMgr.coop=true;
 _bSystemInited=false;require(fcamera_GetCameraCount()==0,"camera cleanup safe before initialization");_CoopWatchCamerasReset();_bSystemInited=true;
 require(!cameras[0].SetViewSource(&cameras[0]),"self-link rejected");require(cameras[1].SetViewSource(&cameras[0])&&!cameras[0].SetViewSource(&cameras[1]),"cyclic source links rejected");
 for(auto dims:{std::pair<int,int>{640,480},{1280,720},{1920,1080},{2560,1440},{3440,1440},{3840,2160}}){
  for(int split=0;split<3;split++){
   auto& src=cameras[0];auto& dst=cameras[1];src.own.nWidth=dims.first;src.own.nHeight=dims.second/(split==0?2:1);
   src.own.fHalfFOVY=.45f+split*.1f;src.own.fTanHalfFOVY=std::tan(src.own.fHalfFOVY);src.own.fNearZ=.1f+split;src.own.fFarZ=300+split;
   dst.own.nWidth=dims.first/(split==1?2:1);dst.own.nHeight=dims.second/2;dst.own.nScreenTopY=dims.second/2;dst.own.nScreenLeftX=split==1?dims.first/2:0;
   float ownedLens=dst.own.fHalfFOVX;const auto* v=dst.GetViewport();
   require(v->nWidth==dst.own.nWidth&&v->nHeight==dst.own.nHeight&&v->nScreenTopY==dst.own.nScreenTopY&&v->nScreenLeftX==dst.own.nScreenLeftX,"mirroring never moves or expands split-screen area");
   require(v->fHalfFOVY==src.own.fHalfFOVY&&v->fNearZ==src.own.fNearZ&&v->fFarZ==src.own.fFarZ,"live zoom/clip planes follow cinematic source");
   require(std::abs(std::tan(v->fHalfFOVX)/v->fTanHalfFOVY-float(v->nWidth)/v->nHeight)<.0001f,"different split aspects retain undistorted vertical cinematic framing");
   require(dst.own.fHalfFOVX==ownedLens,"cinematic lens does not overwrite normal owned projection");
  }
  float hx=dims.first*.5f,hy=dims.second*.5f;quads.clear();_PcCoopJoinPanels(hx,hy);
  require(quads.size()==25&&pushes==0&&cull==7,"five panels draw faces/edges and restore renderer/cull state");
  for(int n=0;n<4;n++){
   const auto& face=quads[(n+1)*5];float left=(face.a.x/hx+1)*.5f,top=(1-face.a.y/hy)*.5f,width=(face.b.x-face.a.x)/(hx*2),height=(face.a.y-face.d.y)/(hy*2);
   require(std::abs(width-.35f)<.0001f&&std::abs(height-.24f)<.0001f,"panel proportions do not grow with widescreen or resolution");
   require(std::abs(left-(n&1?.51f:.14f))<.0001f&&std::abs(top-(n<2?.285f:.545f))<.0001f,"player panels retain separate rows/columns");
   float stroke=quads[(n+1)*5+1].a.y-quads[(n+1)*5+1].d.y;require(stroke>=1&&stroke<=3,"border thickness stays between one and three pixels");
  }
 }
 std::printf("PASS: %d offline cinematic-camera and menu-layout checks; game not launched.\n",checks);
}
'''

def main():
    camera=(ROOT/'ma/Lib/Fang2/fcamera.cpp').read_text()
    game=(ROOT/'ma/App/ma/game.cpp').read_text()
    menu=(ROOT/'ma/App/ma/wpr_system.cpp').read_text()
    scripts=(ROOT/'ma/App/ma/MAScriptTypes.cpp').read_text()
    code=FIXTURE
    for signature in ('u32 fcamera_GetCameraCount(', 'BOOL CFCamera::SetViewSource(',
                      'const FViewport_t *CFCamera::GetViewport(', 'const CFXfm *CFCamera::GetFinalXfm(',
                      'const CFXfm *CFCamera::GetXfmWithoutShake(', 'const CFVec3 *CFCamera::GetPos(',
                      'void CFCamera::SetupCameraAndViewport('): code+=method(camera,signature)
    for signature in ('static void _CoopWatchCamerasReset( void ) {','static void _CoopWatchCamerasWork( void ) {'): code+=method(game,signature)
    for signature in ('static void _PcCoopJoinPanel(', 'static void _PcCoopJoinPanels('): code+=method(menu,signature)
    code+=method(scripts,'cell AMX_NATIVE_CALL CMAST_CamWrapper::Cam_Deactivate(')
    code+=CHECKS
    OUT.mkdir(parents=True,exist_ok=True)
    (OUT/'cinema_ui.cpp').write_text(code)
    (OUT/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(cinema_ui LANGUAGES CXX)\nadd_executable(cinema_ui cinema_ui.cpp)\ntarget_compile_features(cinema_ui PRIVATE cxx_std_17)\n')
    for command in (['cmake','-S',str(OUT),'-B',str(OUT/'out'),'-A','Win32'],['cmake','--build',str(OUT/'out'),'--config','Release']):
        run=subprocess.run(command,capture_output=True,text=True)
        if run.returncode: raise SystemExit(run.stdout+run.stderr)
    subprocess.run([str(OUT/'out/Release/cinema_ui.exe')],check=True)

if __name__=='__main__': main()

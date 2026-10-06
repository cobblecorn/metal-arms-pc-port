"""Production vendor audio and active-location listener checks; no game launch."""
from pathlib import Path
import subprocess
from test_collectable_skiplist import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'build/test-vendor-audio'
BASE = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
using s32=int;using u32=unsigned;using f32=float;using BOOL=bool;using cchar=const char;
#define TRUE true
#define FALSE false
#define FANG_WINGC 1
#define FMATH_MAX_FLOAT 1e30f
#define FMATH_CLAMP_UNIT_FLOAT(x) x=std::max(0.f,std::min(1.f,x))
#define FMATH_CLAMP(x,a,b) x=std::max(a,std::min(b,x))
#define BARTER_SYSTEM_LEVELMUSIC_PAUSE_HYSTERISIS 10.f
#define BARTER_SYSTEM_ATTRACT_RADIUS_HYSTERISIS 1.f
float FLoop_fPreviousLoopSecs=.25f;int checks=0,stopAll=0;
void require(bool b,const char* m){++checks;if(!b){std::fprintf(stderr,"FAIL: %s\n",m);std::exit(1);}}
int fclib_stricmp(const char* a,const char* b){return _stricmp(a,b);}
float fmath_Inv(float f){return 1/f;}
struct CFVec3A{float x=0,y=0,z=0;float MagSq(){return x*x+y*y+z*z;}
 CFVec3A& Sub(const CFVec3A& a,const CFVec3A& b){x=a.x-b.x;y=a.y-b.y;z=a.z-b.z;return *this;}
 float Dist(const CFVec3A& b)const{return std::sqrt((x-b.x)*(x-b.x)+(y-b.y)*(y-b.y)+(z-b.z)*(z-b.z));}
 CFVec3A& Unitize(){float n=std::sqrt(MagSq());x/=n;y/=n;z/=n;return *this;}
 float Dot(const CFVec3A& b){return x*b.x+y*b.y+z*b.z;}};
struct CFMtx43A{CFVec3A m_vPos,m_vRight{1,0,0};};
struct CBot{CFMtx43A mtx;bool inWorld=true,dead=false;bool IsInWorld(){return inWorld;}bool IsDeadOrDying(){return dead;}CFMtx43A* MtxToWorld(){return &mtx;}};
struct CPlayer{static int m_nPlayerCount;CBot* m_pEntityCurrent;};int CPlayer::m_nPlayerCount=2;CPlayer Player_aPlayer[4];
int _nBarterPtSelected=1;unsigned _nBarterPtCnt=2;CBot* _apBarterPts[2];
struct{bool coop=true;bool IsLocalCoop(){return coop;}}MultiplayerMgr;
struct CFAudioStream{float volume=1,pan=0;void SetVolume(float v){volume=v;}void SetPan(float p){pan=p;}}streams[2];
struct Info{CFAudioStream* pAudioStream=nullptr;char szFilename[64]{};} _aStreamingAudioInfo[2];
int _STREAM_COUNT=2;bool musicPaused=false;
void level_StopStream(const char* name){for(auto& s:_aStreamingAudioInfo)if(s.pAudioStream&&!_stricmp(name,s.szFilename))s.pAudioStream=nullptr;}
void level_StopStreamingSpeech(){_aStreamingAudioInfo[1].pAudioStream=nullptr;}
void level_StopAllStreams(){++stopAll;for(auto& s:_aStreamingAudioInfo)s.pAudioStream=nullptr;}
void level_PauseMusic(bool b){musicPaused=b;}
int level_StartStream(const char* name,unsigned,float,int slot,bool){if(slot<0){slot=0;while(slot<2&&_aStreamingAudioInfo[slot].pAudioStream)++slot;}if(slot>=2||_aStreamingAudioInfo[slot].pAudioStream)return -1;auto& s=_aStreamingAudioInfo[slot];std::strcpy(s.szFilename,name);s.pAudioStream=&streams[slot];return slot;}
void level_PlayStreamingSpeech(const char* name,float,unsigned){level_StopStreamingSpeech();level_StartStream(name,0,1,1,false);}
bool level_IsStreamingSpeechPlaying(){return _aStreamingAudioInfo[1].pAudioStream;}
CFAudioStream* level_GetStreamingSpeechAudioStream(){return _aStreamingAudioInfo[1].pAudioStream;}
class CBarterSound{public:enum Mode_e{MODE_NOT_PLAYING,MODE_ATTRACT,MODE_BARTER,MODE_FADING_OUT};
 Mode_e m_eCurMode;float m_fUnitVolume;bool m_bMusicPaused,m_bBarterTunePlaying;
 CBarterSound();void StartBarteringTune();void Stop(bool=false);void UpdateAttractMusic(const CFVec3A*,float,const CFMtx43A*);void Work();};
'''
CHECKS = r'''
int main(){
 CBot points[2],bots[4];points[1].mtx.m_vPos.x=100;_apBarterPts[0]=&points[0];_apBarterPts[1]=&points[1];
 for(int i=0;i<4;i++)Player_aPlayer[i].m_pEntityCurrent=&bots[i];
 bots[0].mtx.m_vPos.x=0;bots[1].mtx.m_vPos.x=98;
 require(_CoopBarterListener()==1,"listener follows occupied vendor location, not an inactive point");
 bots[1].dead=true;require(_CoopBarterListener()==0,"dead partner excluded");bots[0].dead=true;
 require(_CoopBarterListener()==-1,"no dead listener selected");
 for(int n=2;n<=4;n++){CPlayer::m_nPlayerCount=n;for(int i=0;i<n;i++){bots[i].dead=false;bots[i].mtx.m_vPos.x=10*i;}require(_CoopBarterListener()==n-1,"nearest living partner selected in two through four players");}
 _nBarterPtSelected=-1;require(_CoopBarterListener()==-1,"absent vendors have no listener");
 CBarterSound audio;CFVec3A vendor;CFMtx43A near,far;far.m_vPos.x=150;
 level_StartStream("mission",0,1,0,true);audio.UpdateAttractMusic(&vendor,80,&near);
 require(level_GetStreamByName("Barter_Attr"),"attract track starts near vendors");audio.UpdateAttractMusic(&vendor,80,&near);
 require(musicPaused,"nearby vendor track pauses mission music");audio.Stop();
 require(!musicPaused&&!level_GetStreamByName("Barter_Attr")&&level_GetStreamByName("mission"),"stop removes owned loop and releases mission pause without deleting mission");
 audio.UpdateAttractMusic(&vendor,80,&near);audio.UpdateAttractMusic(&vendor,80,&near);audio.UpdateAttractMusic(&vendor,80,&far);
 require(!musicPaused&&!level_GetStreamByName("Barter_Attr"),"leaving radius ends attraction and restores music");
 audio.UpdateAttractMusic(&vendor,80,&near);audio.UpdateAttractMusic(&vendor,80,&near);
 level_PlayStreamingSpeech("radio",1,1);audio.UpdateAttractMusic(&vendor,80,&near);
 require(level_GetStreamByName("radio")&&!musicPaused&&audio.m_eCurMode==CBarterSound::MODE_NOT_PLAYING,"transmission takeover releases pause and is not overwritten");
 streams[1].volume=.73f;audio.Stop();require(level_GetStreamByName("radio")&&streams[1].volume==.73f,"vendor stop preserves unrelated speech");
 audio.UpdateAttractMusic(&vendor,80,&near);require(level_GetStreamByName("radio"),"idle attraction waits for transmission to finish");
 level_StopStreamingSpeech();audio.UpdateAttractMusic(&vendor,80,&near);audio.Stop(true);level_PlayStreamingSpeech("radio",1,1);audio.Work();
 require(streams[1].volume==.73f&&level_GetStreamByName("radio"),"vendor fade cannot alter a replacement radio stream");audio.Stop();
 audio.StartBarteringTune();require(level_GetStreamByName("mission")&&level_GetStreamByName("Barter_Shop")&&stopAll==0&&musicPaused,"co-op shop preserves mission track while playing its own tune");
 audio.Stop(true);for(int i=0;i<5;i++)audio.Work();require(!level_GetStreamByName("Barter_Shop")&&!musicPaused,"shop fade releases owned tune and mission pause");
 require(!level_GetStreamByName(nullptr)&&!level_GetStreamByName("missing"),"stream lookup rejects absent names");
 MultiplayerMgr.coop=false;audio.StartBarteringTune();require(stopAll==1&&!level_GetStreamByName("mission")&&level_GetStreamByName("Barter_Shop"),"solo/PvP retain original shop stream behavior");
 std::printf("PASS: %d production vendor audio/listener checks; game not launched.\n",checks);
}
'''


def main():
    audio = (ROOT / 'ma/App/ma/BarterSound.cpp').read_text()
    defines = '\n'.join(line for line in audio.splitlines() if line.startswith('#define _'))
    code = BASE + defines + '\n'
    code += method((ROOT / 'ma/App/ma/level.cpp').read_text(), 'CFAudioStream *level_GetStreamByName(')
    code += method((ROOT / 'ma/App/ma/BarterSystem.cpp').read_text(), 'static s32 _CoopBarterListener(')
    for signature in ('CBarterSound::CBarterSound(', 'void CBarterSound::StartBarteringTune(',
                      'void CBarterSound::Stop(', 'void CBarterSound::UpdateAttractMusic(', 'void CBarterSound::Work('):
        code += method(audio, signature)
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'audio.cpp').write_text(code + CHECKS)
    (OUT / 'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(vendor_audio LANGUAGES CXX)\nadd_executable(audio audio.cpp)\ntarget_compile_features(audio PRIVATE cxx_std_17)\n')
    for command in (['cmake', '-S', str(OUT), '-B', str(OUT / 'out'), '-A', 'Win32'],
                    ['cmake', '--build', str(OUT / 'out'), '--config', 'Release']):
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode:
            raise SystemExit(result.stdout + result.stderr)
    subprocess.run([str(OUT / 'out/Release/audio.exe')], check=True)


if __name__ == '__main__':
    main()

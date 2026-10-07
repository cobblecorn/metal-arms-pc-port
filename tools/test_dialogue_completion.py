"""Check the production natural-completion path against short retail timings."""
from pathlib import Path
import subprocess

root=Path(__file__).resolve().parent.parent
out=root/'build/test-dialogue-completion'
source=(root/'ma/App/ma/BotTalkInst.cpp').read_text()
start=source.index('\tm_fCurTimePos += FLoop_fPreviousLoopSecs;', source.index('void CBotTalkInst::Work()'))
end=source.index('// update the audio with', start)
code=r'''
#include <cstdio>
#include <cstdlib>
using s32=int;
#define FANG_WINGC 1
#define DEVPRINTF(...) ((void)0)
#define BOTTALKINSTFLAG_FORCE_2D_AUDIO 1
#define BOTTALKINSTFLAG_3DSOUND_PLAYING 2
#define BOTTALKINSTFLAG_AUDIO_DAMAGED 4
#define FAUDIO_EMITTER_STATE_PLAYING 1
bool Fang_bPortDiag=false;float FLoop_fPreviousLoopSecs=.01f;
struct CBot{static bool m_bCutscenePlaying;const char* Name(){return "Player0";}};
bool CBot::m_bCutscenePlaying=true;
struct Sound{int state=1;float seconds=4.556327f;int GetState(){return state;}float GetSecondsToPlay(){return seconds;}};
struct Talk{float m_fTotalTime=3.815057f;bool stick=false;bool StickAtEnd(){return stick;}};
struct CBotTalkInst;
struct CTalkSystem2{static void TerminateActiveTalk(CBotTalkInst*);};
struct CBotTalkInst{float m_fCurTimePos=0;Talk* m_pTalkData;Sound* m_pCurSound;CBot* m_pBotTarget;unsigned m_uFlags=0;bool ended=false;void End(bool){ended=true;}void Complete();};
void CTalkSystem2::TerminateActiveTalk(CBotTalkInst* p){p->ended=true;}
void CBotTalkInst::Complete(){
'''+source[start:end]+r'''
}
int checks=0;
void require(bool ok,const char* message){++checks;if(!ok){printf("FAIL: %s\n",message);exit(1);}}
int main(){
 Talk talk;Sound clip;CBot bot;CBotTalkInst inst;inst.m_pTalkData=&talk;inst.m_pCurSound=&clip;inst.m_pBotTarget=&bot;
 for(int frame=0;frame<460;++frame){if(inst.m_fCurTimePos>=clip.seconds)clip.state=0;inst.Complete();
  if(inst.m_fCurTimePos<clip.seconds)require(!inst.ended,"normal player cinematic dialogue survives its shorter authored timing");}
 require(inst.ended,"finite dialogue finishes naturally once the sample ends");
 inst.ended=false;inst.m_fCurTimePos=4;clip.state=1;inst.m_uFlags=BOTTALKINSTFLAG_FORCE_2D_AUDIO;
 inst.Complete();require(!inst.ended,"forced cinematic dialogue keeps existing protection");
 inst.m_uFlags=BOTTALKINSTFLAG_3DSOUND_PLAYING;inst.Complete();require(inst.ended,"3D ambient talk keeps authored timing");
 inst.ended=false;inst.m_uFlags=BOTTALKINSTFLAG_AUDIO_DAMAGED;inst.Complete();require(inst.ended,"damaged voice can terminate at authored timing");
 inst.ended=false;inst.m_uFlags=0;clip.seconds=-1;inst.Complete();require(inst.ended,"infinite emitter cannot hold a cinematic forever");
 inst.ended=false;clip.seconds=4.56f;CBot::m_bCutscenePlaying=false;inst.Complete();require(inst.ended,"gameplay dialogue keeps authored timing");
 inst.ended=false;CBot::m_bCutscenePlaying=true;inst.m_pCurSound=nullptr;inst.Complete();require(inst.ended,"missing audio cannot hold a cinematic");
 printf("PASS: %d production dialogue completion checks\n",checks);
}
'''
out.mkdir(parents=True,exist_ok=True)
(out/'checks.cpp').write_text(code)
(out/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(dialogue_checks LANGUAGES CXX)\nadd_executable(checks checks.cpp)\n')
for cmd in (['cmake','-S',str(out),'-B',str(out/'out'),'-A','Win32'],['cmake','--build',str(out/'out'),'--config','Release']):
 p=subprocess.run(cmd,capture_output=True,text=True)
 if p.returncode:raise SystemExit(p.stdout+p.stderr)
subprocess.run([str(out/'out/Release/checks.exe')],check=True)

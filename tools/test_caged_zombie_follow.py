"""Offline checks of production cage-ally selection and follow enforcement."""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'build/test-caged-zombie-follow'


def method(source, signature):
    start = source.index(signature)
    brace = source.index('{', start)
    end, depth = brace + 1, 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end] + '\n'


PRELUDE = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
using BOOL=int;using s32=int;
constexpr int FALSE=0,TRUE=1,MAX_PLAYERS=4,ENTITY_BIT_BOT=1,ENTITY_BIT_BOTZOM=2;
#define DEVPRINTF(...) (void)0
int checks=0;
void require(bool b,const char* text){++checks;if(!b){std::fprintf(stderr,"FAIL: %s\n",text);std::exit(1);}}
int fclib_stricmp(const char* a,const char* b){while(*a&&*b){int d=std::tolower(*a++)-std::tolower(*b++);if(d)return d;}return *a-*b;}
struct CAIBrain {
 bool active=true,thought=false,automatic=false,succeed=true;CAIBrain* leader=nullptr;int assigns=0,stops=0;
 BOOL GetFlag_Active(){return active;}CAIBrain* GetLeader(){return leader;}
 void* GetCurFollowerThoughtPtr(){return thought?this:nullptr;}
 void StopFollowing(){++stops;leader=nullptr;thought=false;}
 void SetFlag_Buddy_Ctrl_Auto(){automatic=true;}
 BOOL AssignLeader(CAIBrain* p){++assigns;if(succeed){leader=p;thought=true;}return succeed;}
};
struct CEntity {
 const char* name="other";int bits=ENTITY_BIT_BOT|ENTITY_BIT_BOTZOM;CAIBrain* brain=nullptr;
 const char* Name(){return name;}int TypeBits(){return bits;}CAIBrain* AIBrain(){return brain;}
};
struct CBot:CEntity {
 enum{BOTFLAG2_INST_CANNOT_BE_RECRUITED=1};
 bool created=true,world=true,dead=false,recruited=false,shock=false,reserved=false;
 int m_nPossessionPlayerIndex=-1,m_nOwnerPlayerIndex=-1,m_nBotFlags2=0;
 BOOL IsCreated(){return created;}BOOL IsInWorld(){return world;}BOOL IsDeadOrDying(){return dead;}
 BOOL Recruit_IsRecruited(){return recruited;}BOOL DataPort_IsBeingShocked(){return shock;}BOOL DataPort_IsReserved(){return reserved;}
 BOOL Recruit_IsInstanceForbidden() const;
};
struct CFVec3A{};
struct CPlayer{static inline int m_nPlayerCount=1;CEntity* m_pEntityCurrent=nullptr;};
CPlayer Player_aPlayer[MAX_PLAYERS];
struct{bool campaign=true;BOOL IsSinglePlayer(){return campaign;}}MultiplayerMgr;
int Level_nLoadedIndex=0;struct{const char* pszWorldResName;}Level_aInfo[1]={{"WEWJjourn01"}};
'''

CHECKS = r'''
int main(){
 CAIBrain playerBrains[4];CBot players[4];
 for(int i=0;i<4;i++){players[i].bits=ENTITY_BIT_BOT;players[i].brain=&playerBrains[i];players[i].m_nPossessionPlayerIndex=i;Player_aPlayer[i].m_pEntityCurrent=&players[i];}
 for(int count=1;count<=4;count++)for(const char* name:{"pipezom1","pipezom2"}){
  CPlayer::m_nPlayerCount=count;CAIBrain brain;CBot bot;bot.name=name;bot.brain=&brain;
  require(_IsCagedWastelandZombie(&bot),"unreleased cage occupants stay neutral");
  require(aiutils_CanScriptRecruitWastelandZombie(&bot,&players[0],nullptr),"retail cage script can recruit these otherwise non-recruitable zombies");
  CFVec3A epicenter;
  require(!aiutils_CanScriptRecruitWastelandZombie(&bot,&players[0],&epicenter),"recruiter grenades retain ordinary zombie restrictions");
  aiutils_EnsureFreedWastelandZombieFollowsPlayer(&bot);
  require(!brain.leader&&brain.assigns==0,"friendship does not release or move a caged bot");
  bot.recruited=true;bot.shock=true;aiutils_EnsureFreedWastelandZombieFollowsPlayer(&bot);
  require(brain.assigns==0,"wait for authored recruitment shock to finish");
  bot.shock=false;aiutils_EnsureFreedWastelandZombieFollowsPlayer(&bot);
  require(brain.leader==&playerBrains[0]&&brain.thought&&brain.automatic,"freed bot follows P1 in solo and co-op without seeing the player first");
  require(!_IsCagedWastelandZombie(&bot),"released bots use ordinary recruited friendship");
  for(int frame=0;frame<60;frame++)aiutils_EnsureFreedWastelandZombieFollowsPlayer(&bot);
  require(brain.assigns==1&&brain.stops==0,"healthy formation is not restarted each frame");
  brain.StopFollowing();aiutils_EnsureFreedWastelandZombieFollowsPlayer(&bot);
  require(brain.leader==&playerBrains[0]&&brain.assigns==2,"follow is restored after checkpoint brain reconstruction");
  brain.thought=false;aiutils_EnsureFreedWastelandZombieFollowsPlayer(&bot);
  require(brain.thought&&brain.assigns==3,"leader with missing follow thought can recover");
  if(count>1){
   players[0].dead=true;aiutils_EnsureFreedWastelandZombieFollowsPlayer(&bot);
   require(brain.leader==&playerBrains[1],"surviving partner leads while P1 is dead");
   players[0].dead=false;aiutils_EnsureFreedWastelandZombieFollowsPlayer(&bot);
   require(brain.leader==&playerBrains[0],"P1 resumes leading after revival");
  }
  CAIBrain possessedBrain;CBot possessed;possessed.bits=ENTITY_BIT_BOT;possessed.brain=&possessedBrain;
  Player_aPlayer[0].m_pEntityCurrent=&possessed;aiutils_EnsureFreedWastelandZombieFollowsPlayer(&bot);
  require(brain.leader==&possessedBrain,"allies follow controlled body during possession");
  Player_aPlayer[0].m_pEntityCurrent=&players[0];
 }
 CPlayer::m_nPlayerCount=1;
 for(int gate=0;gate<14;gate++){
  CAIBrain brain;CBot bot;bot.name="pipezom1";bot.brain=&brain;bot.recruited=true;
  switch(gate){
   case 0:MultiplayerMgr.campaign=false;break;case 1:Level_nLoadedIndex=-1;break;
   case 2:Level_aInfo[0].pszWorldResName="other";break;case 3:bot.name="other";break;
   case 4:bot.name=nullptr;break;case 5:bot.bits=ENTITY_BIT_BOT;break;
   case 6:bot.created=false;break;case 7:bot.world=false;break;case 8:bot.dead=true;break;
   case 9:bot.m_nPossessionPlayerIndex=0;break;case 10:bot.brain=nullptr;break;
   case 11:brain.active=false;break;case 12:Player_aPlayer[0].m_pEntityCurrent=nullptr;break;
   case 13:players[0].world=false;break;
  }
  aiutils_EnsureFreedWastelandZombieFollowsPlayer(&bot);
  require(brain.assigns==0,"exclude PvP, other levels/bots, invalid state, possession and unavailable leaders");
  MultiplayerMgr.campaign=true;Level_nLoadedIndex=0;Level_aInfo[0].pszWorldResName="WEWJjourn01";
  Player_aPlayer[0].m_pEntityCurrent=&players[0];players[0].world=true;
 }
 CAIBrain brain;CBot bot;bot.name="pipezom2";bot.brain=&brain;bot.recruited=true;brain.succeed=false;
 aiutils_EnsureFreedWastelandZombieFollowsPlayer(&bot);require(!brain.leader,"allocation failure does not fabricate a follower");
 brain.succeed=true;aiutils_EnsureFreedWastelandZombieFollowsPlayer(&bot);require(brain.leader==&playerBrains[0],"failed allocation retries next work");
 aiutils_EnsureFreedWastelandZombieFollowsPlayer(nullptr);
 require(!_IsWastelandCageOccupant(nullptr),"null entity is safe");
 for(int gate=0;gate<13;gate++){
  CBot freed;freed.name="pipezom1";
  switch(gate){
   case 0:freed.reserved=true;break;case 1:freed.shock=true;break;case 2:freed.dead=true;break;
   case 3:freed.world=false;break;case 4:freed.created=false;break;case 5:freed.recruited=true;break;
   case 6:freed.m_nPossessionPlayerIndex=0;break;case 7:freed.m_nOwnerPlayerIndex=0;break;
   case 8:freed.m_nBotFlags2=CBot::BOTFLAG2_INST_CANNOT_BE_RECRUITED;break;
   case 9:freed.name="other";break;case 10:players[0].dead=true;break;
   case 11:players[0].m_nPossessionPlayerIndex=-1;break;case 12:MultiplayerMgr.campaign=false;break;
  }
  require(!aiutils_CanScriptRecruitWastelandZombie(&freed,&players[0],nullptr),"script exception respects safety, ownership, instance restrictions and campaign scope");
  players[0].dead=false;players[0].m_nPossessionPlayerIndex=0;MultiplayerMgr.campaign=true;
 }
 require(!aiutils_CanScriptRecruitWastelandZombie(nullptr,&players[0],nullptr),"null recruit is safe");
 CBot freed;freed.name="pipezom2";
 require(!aiutils_CanScriptRecruitWastelandZombie(&freed,nullptr,nullptr),"null recruiter is safe");
 std::printf("PASS: %d production cage-ally checks; game not launched.\n",checks);
}
'''


def main():
    source = (ROOT / 'ma/App/ma/Ai/AIGameUtils.cpp').read_text()
    code = PRELUDE.replace('#include <cctype>', '#include <cctype>\n#include <initializer_list>')
    getter = method((ROOT / 'ma/App/ma/bot.h').read_text(), 'BOOL Recruit_IsInstanceForbidden(')
    code += getter.replace('BOOL Recruit_IsInstanceForbidden(', 'BOOL CBot::Recruit_IsInstanceForbidden(').replace('FASSERT( IsCreated() );', '')
    for signature in ('static BOOL _IsWastelandCageOccupant(', 'static BOOL _IsCagedWastelandZombie(',
                      'BOOL aiutils_CanScriptRecruitWastelandZombie(',
                      'void aiutils_EnsureFreedWastelandZombieFollowsPlayer('):
        code += method(source, signature)
    code += CHECKS
    bot = (ROOT / 'ma/App/ma/bot.cpp').read_text()
    assert 'aiutils_EnsureFreedWastelandZombieFollowsPlayer( this );' in method(bot, 'void CBot::ClassHierarchyWork(')
    assert '&& !aiutils_CanScriptRecruitWastelandZombie( this, pRecruiterBot, pEpicenter_WS )' in method(bot, 'BOOL CBot::Recruit(')
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'follow.cpp').write_text(code)
    (OUT / 'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(cage_follow LANGUAGES CXX)\nadd_executable(cage_follow follow.cpp)\ntarget_compile_features(cage_follow PRIVATE cxx_std_17)\n')
    for command in (['cmake', '-S', str(OUT), '-B', str(OUT / 'out'), '-A', 'Win32'],
                    ['cmake', '--build', str(OUT / 'out'), '--config', 'Release']):
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode:
            raise SystemExit(result.stdout + result.stderr)
    subprocess.run([str(OUT / 'out/Release/cage_follow.exe')], check=True)


if __name__ == '__main__':
    main()

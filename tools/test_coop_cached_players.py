"""Production player bindings + retail elevator script in AMX; no game launch."""
from pathlib import Path
import subprocess
from test_collectable_skiplist import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'build/test-coop-cached-players'

BASE = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "pc_script_goals.h"
using u32=unsigned;using u8=unsigned char;using s32=int;
const int ENTITY_BIT_BOT=1,MAX_PLAYERS=4;
int checks=0;
void require(bool b,const char* m){++checks;if(!b){std::fprintf(stderr,"FAIL: %s\n",m);std::exit(1);}}
struct CEntity{const char* name="player";bool inWorld=true;bool IsInWorld(){return inWorld;}int TypeBits(){return ENTITY_BIT_BOT;}const char* Name(){return name;}};
struct CBot:CEntity{bool dead=false;static bool m_bCutscenePlaying;bool IsDeadOrDying(){return dead;}};
bool CBot::m_bCutscenePlaying=false;
struct CPlayer{static int m_nPlayerCount;CEntity* m_pEntityOrig;CEntity* m_pEntityCurrent;};
int CPlayer::m_nPlayerCount=2;CPlayer Player_aPlayer[4];int _nCoopStoryPlayer=-1;
struct{bool coop=true;bool IsLocalCoop(){return coop;}}MultiplayerMgr;
struct CFScript{char m_szScriptFileName[32];unsigned m_uDataAreaSize;};
struct CFScriptInst{bool m_bIsInitialized=true;AMX m_oAMX{};CFScript* m_pScript;void* m_pDataArea;};
struct CFScriptSystem{static CFScriptInst* m_pCurScriptInst;};CFScriptInst* CFScriptSystem::m_pCurScriptInst=nullptr;
struct CMAScriptTypes{static void RefreshCoopScriptPlayers();};
int checkpoint=0,added=0,liftAdded=0,fakeRemoved=0,assigned=0;
cell timer=700,trigger=800,lift=900,fakeLift=901;CBot bots[4];
int AMXAPI callback(AMX* amx,cell id,cell* result,cell* params){
 auto* h=(AMX_HEADER*)amx->base;auto* f=(AMX_FUNCSTUB*)(amx->base+h->natives)+id;const char* n=f->name;*result=0;
 if(!std::strcmp(n,"Bot_GetPlayer")){*result=(cell)Player_aPlayer[game_GetStoryPlayerIndex()].m_pEntityCurrent;_CoopBindScriptPlayer(amx,(CEntity*)*result);}
 else if(!std::strcmp(n,"E_Find")){cell* addr;char text[100];amx_GetAddr(amx,params[1],&addr);amx_GetString(text,addr);*result=600;
  if(!std::strcmp(text,"add_titan1"))*result=trigger;
  if(!std::strcmp(text,"g_lift"))*result=lift;
  if(!std::strcmp(text,"g_fakelift"))*result=fakeLift;
 }else if(!std::strcmp(n,"event_TranslateName")){cell* addr;char text[100];amx_GetAddr(amx,params[1],&addr);amx_GetString(text,addr);*result=!std::strcmp(text,"tripwire")?77:88;}
 else if(!std::strcmp(n,"Timer_AcquireETimer"))*result=timer;
 else if(!std::strcmp(n,"Checkpoint_Save"))++checkpoint;
 else if(!std::strcmp(n,"E_AddToWorld")){++added;if(params[1]==lift)++liftAdded;}
 else if(!std::strcmp(n,"E_RemoveFromWorld")){if(params[1]==fakeLift)++fakeRemoved;}
 else if(!std::strcmp(n,"Bot_GotoE")){++assigned;require(params[5]==(cell)&bots[1],"NPC look target follows surviving story player");}
 return AMX_ERR_NONE;
}
void execute(CFScriptInst& inst,const char* name,int event=0,cell d1=0,cell d2=0,cell d3=0){
 auto& amx=inst.m_oAMX;auto* h=(AMX_HEADER*)amx.base;
 std::memcpy(amx.base+h->dat,inst.m_pDataArea,inst.m_pScript->m_uDataAreaSize);
 CFScriptSystem::m_pCurScriptInst=&inst;int index;cell result;
 require(amx_FindPublic(&amx,const_cast<char*>(name),&index)==0,"public found");
 int err=event?amx_Exec(&amx,&result,index,5,0,event,d1,d2,d3):amx_Exec(&amx,&result,index,1,0);
 require(err==0,"actual script executes");
 std::memcpy(inst.m_pDataArea,amx.base+h->dat,inst.m_pScript->m_uDataAreaSize);
}
'''

CHECKS = r'''
int main(int argc,char** argv){
 require(argc==2,"retail script supplied");FILE* f=std::fopen(argv[1],"rb");require(f,"opened");AMX_HEADER h;
 require(std::fread(&h,sizeof(h),1,f)==1,"header read");auto* mem=(unsigned char*)std::calloc(1,h.stp);std::rewind(f);
 require(std::fread(mem,1,h.size,f)==size_t(h.size),"read");std::fclose(f);
 CFScript script{};std::strcpy(script.m_szScriptFileName,"xewr4adtitn.sma");script.m_uDataAreaSize=h.hea-h.dat;
 CFScriptInst inst;inst.m_pScript=&script;require(amx_Init(&inst.m_oAMX,mem)==0,"real AMX expansion/relocation");inst.m_oAMX.callback=callback;
 inst.m_pDataArea=std::malloc(script.m_uDataAreaSize);std::memcpy(inst.m_pDataArea,mem+h.dat,script.m_uDataAreaSize);
 for(int i=0;i<4;i++){bots[i].name=i?"Player1":"Player0";Player_aPlayer[i].m_pEntityOrig=Player_aPlayer[i].m_pEntityCurrent=&bots[i];}
 execute(inst,"OnInit");require(_nCoopScriptPlayerBindings==1&&_aCoopScriptPlayerBindings[0].nOffset==52,"native registers actual direct player cache");
 bots[0].dead=true;
 execute(inst,"OnEvent",77,0,trigger,(cell)&bots[1]);
 require(checkpoint==0&&added==0,"legacy cached P1 rejects live P2 and never creates lift");
 CMAScriptTypes::RefreshCoopScriptPlayers();
 require(((cell*)inst.m_pDataArea)[13]==(cell)&bots[1],"dead P1 cache rebound to P2");
 execute(inst,"OnEvent",77,0,trigger,(cell)&bots[1]);
 require(checkpoint==1&&added==8,"surviving P2 activates normal checkpoint and all eight NPCs");
 execute(inst,"OnEvent",88,timer,1,0);require(assigned==4,"normal elevator setup timer assigns all four walkers");
 execute(inst,"OnEvent",88,timer,2,0);require(liftAdded==1&&fakeRemoved==1,"normal second timer installs usable lift and removes fake");
 bots[0].dead=false;CMAScriptTypes::RefreshCoopScriptPlayers();
 require(((cell*)inst.m_pDataArea)[13]==(cell)&bots[0],"revived P1 canonical identity restored");
 bots[0].dead=true;CBot::m_bCutscenePlaying=true;_nCoopStoryPlayer=1;
 CMAScriptTypes::RefreshCoopScriptPlayers();require(((cell*)inst.m_pDataArea)[13]==(cell)&bots[1],"chosen scene actor bound");
 bots[0].dead=false;CMAScriptTypes::RefreshCoopScriptPlayers();require(((cell*)inst.m_pDataArea)[13]==(cell)&bots[1],"active scene actor stays stable after revival");
 CBot::m_bCutscenePlaying=false;MultiplayerMgr.coop=false;
 CMAScriptTypes::RefreshCoopScriptPlayers();require(((cell*)inst.m_pDataArea)[13]==(cell)&bots[1],"solo/PvP untouched");MultiplayerMgr.coop=true;
 ((cell*)inst.m_pDataArea)[13]=12345;CMAScriptTypes::RefreshCoopScriptPlayers();require(((cell*)inst.m_pDataArea)[13]==12345,"script-repurposed entity slot preserved");
 ((cell*)inst.m_pDataArea)[13]=(cell)&bots[0];CMAScriptTypes::RefreshCoopScriptPlayers();require(((cell*)inst.m_pDataArea)[13]==(cell)&bots[0],"restored checkpoint handle recognized");
 for(int count=2;count<=4;count++)for(int survivor=1;survivor<count;survivor++){
  CPlayer::m_nPlayerCount=count;for(int n=0;n<count;n++)bots[n].dead=n!=survivor;
  ((cell*)inst.m_pDataArea)[13]=(cell)&bots[0];CMAScriptTypes::RefreshCoopScriptPlayers();
  require(((cell*)inst.m_pDataArea)[13]==(cell)&bots[survivor],"any surviving slot can become script actor");
 }
 for(int n=0;n<4;n++)bots[n].dead=true;((cell*)inst.m_pDataArea)[13]=(cell)&bots[0];CMAScriptTypes::RefreshCoopScriptPlayers();
 require(((cell*)inst.m_pDataArea)[13]==(cell)&bots[0],"all dead does not bind a dying actor");
 int cip=inst.m_oAMX.cip;inst.m_oAMX.cip=h.dat-h.cod-4;
 require(PortScriptPlayerStoreOffset(&inst.m_oAMX,script.m_uDataAreaSize)==-1,"short return sequence rejected");inst.m_oAMX.cip=cip;
 std::free(inst.m_pDataArea);std::free(mem);
 std::printf("PASS: %d retail elevator/player-binding checks; game not launched.\n",checks);
}
'''


def main():
    source = (ROOT / 'ma/App/ma/MAScriptTypes.cpp').read_text()
    start = source.index('struct _CoopScriptPlayerBinding {')
    end = source.index('// Campaign co-op (PC):', start)
    game = (ROOT / 'ma/App/ma/game.cpp').read_text()
    # Production story selection + registration/refresh, with real AMX bytecode.
    code = BASE[:BASE.index('int checkpoint=')] + method(game, 's32 game_GetStoryPlayerIndex(')
    code += source[start:end] + BASE[BASE.index('int checkpoint='):] + CHECKS
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'binding.cpp').write_text(code)
    (OUT / 'fclib.h').write_text('#pragma once\n#include <string.h>\ntypedef bool BOOL;\n'
                               '#define TRUE true\n#define FALSE false\n#define DEVPRINTF(...) ((void)0)\n'
                               'inline int fclib_stricmp(const char* a,const char* b){return _stricmp(a,b);}\n')
    (OUT / 'CMakeLists.txt').write_text(
        'cmake_minimum_required(VERSION 3.20)\nproject(cached_players C CXX)\n'
        f'add_executable(binding binding.cpp "{(ROOT / "ma/Lib/SmallAMX/amx.c").as_posix()}")\n'
        f'target_include_directories(binding PRIVATE "{OUT.as_posix()}" "{(ROOT / "port").as_posix()}" '
        f'"{(ROOT / "ma/Lib/SmallAMX").as_posix()}")\n'
        'target_compile_features(binding PRIVATE cxx_std_14)\ntarget_compile_definitions(binding PRIVATE WIN32 _CRT_SECURE_NO_WARNINGS)\n')
    for cmd in (['cmake', '-S', str(OUT), '-B', str(OUT / 'out'), '-A', 'Win32'],
                ['cmake', '--build', str(OUT / 'out'), '--config', 'Release']):
        r = subprocess.run(cmd, capture_output=True, text=True)
        if r.returncode:
            raise SystemExit(r.stdout + r.stderr)
    subprocess.run([str(OUT / 'out/Release/binding.exe'), str(ROOT / 'gamedata/mst/xewr4adtitn.sma')], check=True)


if __name__ == '__main__':
    main()

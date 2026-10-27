"""Run focused co-op report probes with real rendering and separate test saves."""
from pathlib import Path
import argparse,hashlib,json,os,subprocess,time

ROOT=Path(__file__).resolve().parents[1]

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--case',choices=('washer','lift','panel','gifts','tether','rivet'),required=True)
    p.add_argument('--players',type=int,choices=(1,2,3,4),default=2)
    p.add_argument('--owner',type=int,choices=(1,2,3,4),default=1)
    p.add_argument('--tag',default='after')
    a=p.parse_args();assert a.owner<=a.players
    mode=f'report-batch-{a.case}{a.owner}'
    name=f'coop-batch-{a.case}-{a.players}p-owner{a.owner}-{a.tag}'
    save=ROOT/'build/test-saves'/name;save.mkdir(parents=True,exist_ok=True)
    log=ROOT/'build/logs'/(name+'.log');shots=ROOT/'build/shots'/name;shots.mkdir(parents=True,exist_ok=True)
    exe=ROOT/'build/Release/ma_port.exe'
    if log.exists():log.unlink()
    mission={'washer':'WEMCcity_01','lift':'WEDMmines01','panel':'WEMCcity_02',
             'gifts':'WEMCcity_01','tether':'WEMCcity_02','rivet':'WEDMmines02'}[a.case]
    env={k:v for k,v in os.environ.items() if not k.startswith('MA_PORT_TEST_')}
    env['MA_PORT_TEST_COOP_POLISH']=mode
    cmd=[str(exe),'-data',str(ROOT/'gamedata/files'),'-mission',mission,'-save-dir',str(save),
         '-windowed','-res','1280x720','-mute','-discord-app-id','off','-log',str(log),
         '-asset-log',str(log.with_suffix('.assets.log')),'-test-keys','5:0x20,8:0x20,11:0x20',
         '-shots',str(shots),'-shot-every','30']
    if a.players>1:cmd+=['-coop',str(a.players)]
    if a.case=='rivet':
        assert a.owner==1,'This input probe uses the synthetic P1 mouse.'
        cmd[cmd.index('-mission')]='-level'
        cmd[cmd.index('-test-keys')+1]=','.join(f'g{3+i*.24:.2f}:1' for i in range(30))
        cmd+=['-test-give','Rivet Gun L3']
    (log.with_suffix('.run.json')).write_text(json.dumps({'command':cmd,'exe_sha256':hashlib.sha256(exe.read_bytes()).hexdigest()},indent=2))
    si=subprocess.STARTUPINFO();si.dwFlags|=subprocess.STARTF_USESHOWWINDOW;si.wShowWindow=0
    proc=subprocess.Popen(cmd,cwd=ROOT,env=env,startupinfo=si,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    print(name,'PID',proc.pid,flush=True)
    seen=0
    try:
        deadline=time.monotonic()+180;game_deadline=None
        while proc.poll() is None and time.monotonic()<deadline:
            text=log.read_text(encoding='latin1') if log.exists() else ''
            if game_deadline is None and 'END OF BOOTUP' in text:game_deadline=time.monotonic()+65
            if game_deadline and time.monotonic()>game_deadline:break
            lines=[s for s in text.splitlines() if s.startswith(('BATCH','COOP-TEST')) or '*** CRASH' in s or '*** FANG' in s]
            for s in lines[seen:]:print(s,flush=True)
            seen=len(lines)
            if any(s in text for s in ('BATCH COMPLETE','COOP-TEST FAIL','*** CRASH','*** FANG')):break
            time.sleep(.25)
    finally:
        if proc.poll() is None:proc.terminate()
        proc.wait(timeout=10)
    text=log.read_text(encoding='latin1') if log.exists() else ''
    assert 'BATCH COMPLETE' in text and not any(s in text for s in ('COOP-TEST FAIL','*** CRASH','*** FANG')),str(log)
    print('PASS',name,flush=True)

if __name__=='__main__':main()

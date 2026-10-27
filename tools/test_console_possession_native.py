"""Exercise Level 15's first real arcade with isolated saves and native rendering."""
from pathlib import Path
import argparse,os,subprocess,time

ROOT=Path(__file__).resolve().parents[1]

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--players',type=int,choices=(1,2),default=1)
    p.add_argument('--owner',type=int,choices=(1,2),default=1)
    p.add_argument('--scenario',choices=('normal','cancel','death','restore'),default='normal')
    a=p.parse_args();assert a.owner<=a.players
    name=f'console15-{a.players}p-owner{a.owner}-{a.scenario}'
    save=ROOT/'build/test-saves'/name;save.mkdir(parents=True,exist_ok=True)
    log=ROOT/'build/logs'/(name+'.log');log.parent.mkdir(exist_ok=True)
    if log.exists():log.unlink()
    env={k:v for k,v in os.environ.items() if not k.startswith('MA_PORT_TEST_')}
    env['MA_PORT_TEST_COOP_POLISH']=f'report-console-{a.scenario}{a.owner}'
    cmd=[str(ROOT/'build/Release/ma_port.exe'),'-data',str(ROOT/'gamedata/files'),
         '-mission','WEWRresrch2','-save-dir',str(save),'-windowed','-res','960x720',
         '-mute','-discord-app-id','off','-log',str(log),'-asset-log',str(log.with_suffix('.assets.log')),
         '-test-keys','5:0x20,8:0x20,11:0x20']
    if a.players>1:cmd+=['-coop',str(a.players)]
    si=subprocess.STARTUPINFO();si.dwFlags|=subprocess.STARTF_USESHOWWINDOW;si.wShowWindow=0
    proc=subprocess.Popen(cmd,cwd=ROOT,env=env,startupinfo=si,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    print(name,'PID',proc.pid,flush=True)
    seen=0
    try:
        deadline=time.monotonic()+180
        gameplay_deadline=None
        while proc.poll() is None and time.monotonic()<deadline:
            text=log.read_text(encoding='latin1') if log.exists() else ''
            if gameplay_deadline is None and 'END OF BOOTUP' in text:
                gameplay_deadline=time.monotonic()+65
            if gameplay_deadline is not None and time.monotonic()>gameplay_deadline:break
            lines=[s for s in text.splitlines() if s.startswith(('CONSOLE-TEST','COOP-TEST')) or '*** CRASH' in s or '*** FANG' in s]
            for s in lines[seen:]:print(s,flush=True)
            seen=len(lines)
            if 'CONSOLE-TEST COMPLETE' in text or 'COOP-TEST FAIL' in text or '*** CRASH' in text or '*** FANG' in text:break
            time.sleep(.25)
    finally:
        if proc.poll() is None:proc.terminate()
        proc.wait(timeout=10)
    text=log.read_text(encoding='latin1') if log.exists() else ''
    assert 'CONSOLE-TEST COMPLETE' in text and all(x not in text for x in ('COOP-TEST FAIL','*** CRASH','*** FANG')),str(log)
    print('PASS:',name,flush=True)

if __name__=='__main__':main()

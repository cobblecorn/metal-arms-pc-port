"""Exercise the real Mil City slope/doorway descent with an isolated native save."""
import argparse
import os
from pathlib import Path
import subprocess
import time

ROOT=Path(__file__).resolve().parents[1]

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--players',type=int,choices=(1,2),default=1)
    p.add_argument('--owner',type=int,choices=(1,2),default=1)
    a=p.parse_args()
    assert a.owner<=a.players
    name=f'slope-doorway-{a.players}p-owner{a.owner}'
    save=ROOT/'build/test-saves'/name;save.mkdir(parents=True,exist_ok=True)
    log=ROOT/'build/logs'/(name+'.log');log.parent.mkdir(exist_ok=True)
    if log.exists():log.unlink()
    env={k:v for k,v in os.environ.items() if not k.startswith('MA_PORT_TEST_')}
    env['MA_PORT_TEST_COOP_POLISH']='report-slope'+str(a.owner)
    cmd=[str(ROOT/'build/Release/ma_port.exe'),'-data',str(ROOT/'gamedata/files'),
         '-mission','WEMCcity_03','-save-dir',str(save),'-windowed','-res','960x720',
         '-mute','-discord-app-id','off','-log',str(log),'-asset-log',str(log.with_suffix('.assets.log'))]
    if a.players>1:cmd+=['-coop',str(a.players)]
    si=subprocess.STARTUPINFO();si.dwFlags|=subprocess.STARTF_USESHOWWINDOW;si.wShowWindow=0
    proc=subprocess.Popen(cmd,cwd=ROOT,env=env,startupinfo=si,stdout=subprocess.DEVNULL)
    print('Isolated slope probe PID',proc.pid,flush=True)
    try:
        deadline=time.monotonic()+55
        while proc.poll() is None and time.monotonic()<deadline:
            text=log.read_text(encoding='latin1') if log.exists() else ''
            if 'SLOPE-TEST COMPLETE' in text or 'COOP-TEST FAIL' in text or '*** FANG' in text:break
            time.sleep(.25)
    finally:
        if proc.poll() is None:proc.terminate()
        proc.wait(timeout=10)
    text=log.read_text(encoding='latin1') if log.exists() else ''
    for line in text.splitlines():
        if line.startswith(('COOP-TEST','SLOPE-TEST start','SLOPE-TEST COMPLETE')):print(line)
    assert 'SLOPE-TEST COMPLETE' in text and 'COOP-TEST FAIL' not in text and '*** FANG' not in text,str(log)
    assert text.count('COOP-TEST PASS: native player descends past the doorway trap')==6,str(log)
    print('PASS:',name)

if __name__=='__main__':main()

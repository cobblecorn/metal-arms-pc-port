"""Check authored/alternate miner deaths in an isolated native campaign run."""
import argparse
import os
from pathlib import Path
import subprocess
import time

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--players',type=int,choices=(1,2),default=1)
    args=parser.parse_args();name=f'miner-death-{args.players}p'
    save=ROOT/'build/test-saves'/name;save.mkdir(parents=True,exist_ok=True)
    shots=ROOT/'build/shots'/name;shots.mkdir(parents=True,exist_ok=True)
    # This folder is exclusively owned by this fixture. Do not review stale frames.
    assert shots.resolve().parent==(ROOT/'build/shots').resolve()
    for old in shots.glob('*.bmp'):old.unlink()
    log=ROOT/'build/logs'/(name+'.log')
    if log.exists():log.unlink()
    env={k:v for k,v in os.environ.items() if not k.startswith('MA_PORT_TEST_')}
    env['MA_PORT_TEST_COOP_POLISH']='report-miner-death'
    cmd=[str(ROOT/'build/Release/ma_port.exe'),'-data',str(ROOT/'gamedata/files'),
         '-mission','WEDiinvas01','-save-dir',str(save),'-windowed','-res','960x720',
         '-mute','-discord-app-id','off','-log',str(log),'-asset-log',str(log.with_suffix('.assets.log')),
         '-test-keys','5:0x20,8:0x20,11:0x20',
         '-shots',str(shots),'-shot-every','60']
    if args.players>1:cmd+=['-coop',str(args.players)]
    si=subprocess.STARTUPINFO();si.dwFlags|=subprocess.STARTF_USESHOWWINDOW;si.wShowWindow=0
    proc=subprocess.Popen(cmd,cwd=ROOT,env=env,startupinfo=si,stdout=subprocess.DEVNULL)
    print('Isolated miner-death probe PID',proc.pid,flush=True)
    try:
        deadline=time.monotonic()+50
        while proc.poll() is None and time.monotonic()<deadline:
            text=log.read_text(encoding='latin1') if log.exists() else ''
            if 'MINER-DEATH COMPLETE' in text or '*** FANG' in text:break
            time.sleep(.25)
    finally:
        if proc.poll() is None:proc.terminate()
        proc.wait(timeout=10)
    text=log.read_text(encoding='latin1') if log.exists() else ''
    for line in text.splitlines():
        if line.startswith(('MINER-DEATH','COOP-TEST')) or 'PartPool' in line or 'initialize death breakup' in line:print(line)
    assert 'MINER-DEATH COMPLETE' in text and 'COOP-TEST FAIL' not in text and '*** FANG' not in text,str(log)
    print('PASS:',name)

if __name__=='__main__':main()

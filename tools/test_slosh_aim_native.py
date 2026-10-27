"""Probe the real Slosh muzzle and fire direction in Fire It Up with isolated saves."""
from pathlib import Path
import argparse,os,subprocess,time

ROOT=Path(__file__).resolve().parent.parent

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline',action='store_true')
    parser.add_argument('--res',default='1280x720')
    parser.add_argument('--shots',action='store_true')
    parser.add_argument('--players',type=int,choices=(1,2,3,4),default=1)
    parser.add_argument('--owner',type=int,choices=(1,2,3,4),default=1)
    args=parser.parse_args()
    assert args.owner<=args.players
    name=f'slosh-aim-{"baseline" if args.baseline else "fixed"}-{args.players}p-owner{args.owner}-{args.res}'
    work=ROOT/'build/test-saves'/name;work.mkdir(parents=True,exist_ok=True)
    log=ROOT/'build/logs'/(name+'.log');log.parent.mkdir(exist_ok=True)
    if log.exists():log.unlink()
    mode=('report-slosh-baseline' if args.baseline else 'report-slosh-aim')+str(args.owner)
    env=dict(os.environ,MA_PORT_TEST_COOP_POLISH=mode)
    env.pop('MA_PORT_TEST_KEYS',None)
    cmd=[str(ROOT/'build/Release/ma_port.exe'),'-data',str(ROOT/'gamedata/files'),'-mission','WERRreactr2',
         '-save-dir',str(work),'-discord-app-id','off','-windowed','-res',args.res,'-mute','-log',str(log),'-asset-log',str(log.with_suffix('.assets.log'))]
    cmd+=['-test-keys',','.join(f'g{7+i*.24:.2f}:1' for i in range(18))]
    if args.shots:cmd+=['-shots',str(ROOT/'build/logs'/(name+'-shots')),'-shot-every','60']
    if args.players>1:cmd+=['-coop',str(args.players)]
    startup=subprocess.STARTUPINFO();startup.dwFlags|=subprocess.STARTF_USESHOWWINDOW;startup.wShowWindow=4 if args.shots else 0
    proc=subprocess.Popen(cmd,cwd=ROOT,env=env,startupinfo=startup,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    print('Native Slosh aim probe PID',proc.pid,flush=True)
    deadline=time.monotonic()+65;last=[]
    try:
        while proc.poll() is None and time.monotonic()<deadline:
            text=log.read_text(encoding='latin-1') if log.exists() else ''
            lines=[s for s in text.splitlines() if s.startswith('SLOSH-AIM') or s.startswith('COOP-TEST FAIL')]
            for line in lines[len(last):]:print(line,flush=True)
            last=lines
            if 'SLOSH-AIM COMPLETE' in text or 'COOP-TEST FAIL' in text or '*** FANG' in text:break
            time.sleep(.25)
    finally:
        if proc.poll() is None:proc.terminate()
        proc.wait(timeout=10)
    text=log.read_text(encoding='latin-1') if log.exists() else ''
    assert 'SLOSH-AIM COMPLETE' in text and 'COOP-TEST FAIL' not in text and '*** FANG' not in text,str(log)
    print('PASS:',name,flush=True)

if __name__=='__main__':main()

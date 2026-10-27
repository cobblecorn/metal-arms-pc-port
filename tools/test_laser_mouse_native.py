"""Run native mouse tap/hold input through Glitch's real weapon and grenade handling."""
from pathlib import Path
import argparse,os,subprocess,time

ROOT=Path(__file__).resolve().parent.parent

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--upgrade',type=int,choices=(1,2,3),default=2);args=parser.parse_args()
    name=f'laser-mouse-controls-L{args.upgrade}'
    work=ROOT/'build/test-saves'/name;work.mkdir(parents=True,exist_ok=True)
    log=ROOT/'build/logs'/(name+'.log');log.parent.mkdir(exist_ok=True)
    if log.exists():log.unlink()
    keys=['g8:0x02']
    for start,count,key in [(12,18,2),(20,6,1),(25,5,2)]:
        keys += [f'g{start+i*.24:.2f}:{key}' for i in range(count)]
    # The input bridge holds each scheduled key for 250 ms, so 240-ms spacing
    # creates one continuous hold without touching the desktop's physical keys.
    env=dict(os.environ,MA_PORT_TEST_COOP_POLISH='report-laser-input')
    env.pop('MA_PORT_TEST_KEYS',None)
    cmd=[str(ROOT/'build/Release/ma_port.exe'),'-data',str(ROOT/'gamedata/files'),'-level','WEDMmines02',
         '-test-give',f'Laser L{args.upgrade}','-test-keys',','.join(keys),'-save-dir',str(work),'-discord-app-id','off',
         '-windowed','-res','960x720','-mute','-log',str(log),'-asset-log',str(log.with_suffix('.assets.log'))]
    startup=subprocess.STARTUPINFO();startup.dwFlags|=subprocess.STARTF_USESHOWWINDOW;startup.wShowWindow=0
    proc=subprocess.Popen(cmd,cwd=ROOT,env=env,startupinfo=startup,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    print('Native laser input probe PID',proc.pid,flush=True)
    deadline=time.monotonic()+75;seen=0
    try:
        while proc.poll() is None and time.monotonic()<deadline:
            text=log.read_text(encoding='latin-1') if log.exists() else ''
            lines=[s for s in text.splitlines() if s.startswith('LASER-INPUT') or s.startswith('COOP-TEST FAIL')]
            for line in lines[seen:]:print(line,flush=True)
            seen=len(lines)
            if 'LASER-INPUT COMPLETE' in text or 'COOP-TEST FAIL' in text or '*** FANG' in text:break
            time.sleep(.25)
    finally:
        if proc.poll() is None:proc.terminate()
        proc.wait(timeout=10)
    text=log.read_text(encoding='latin-1') if log.exists() else ''
    assert 'LASER-INPUT COMPLETE' in text and 'COOP-TEST FAIL' not in text and '*** FANG' not in text,str(log)
    print('PASS: native right tap / charged hold / left repeat / low-energy release',flush=True)

if __name__=='__main__':main()

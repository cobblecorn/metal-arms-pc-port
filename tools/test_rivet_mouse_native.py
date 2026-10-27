"""Check real right-click Rivet charging with grenade and scope secondary items."""
from pathlib import Path
import argparse, hashlib, json, os, subprocess, time

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--players', type=int, choices=(1, 2), default=1)
    args = parser.parse_args()
    name = f'rivet-mouse-controls-{args.players}p'
    log = ROOT / 'build/logs' / (name + '.log')
    if log.exists():
        log.unlink()
    save = ROOT / 'build/test-saves' / name
    save.mkdir(parents=True, exist_ok=True)
    keys = ['g8:0x02', 'g29:0x02']
    for start in (12, 34):
        keys += [f'g{start+i*.24:.2f}:2' for i in range(50)]
    env = {k: v for k, v in os.environ.items() if not k.startswith('MA_PORT_TEST_')}
    env['MA_PORT_TEST_COOP_POLISH'] = 'report-rivet-input'
    exe = ROOT / 'build/Release/ma_port.exe'
    command = [str(exe), '-data', str(ROOT / 'gamedata/files'), '-level', 'WEDMmines02',
               '-test-give', 'Rivet Gun L3', '-test-keys', ','.join(keys), '-save-dir', str(save),
               '-discord-app-id', 'off', '-windowed', '-res', '960x720', '-mute', '-log', str(log)]
    if args.players > 1:
        command[command.index('-level')] = '-mission'
        command[command.index('-mission')+1] = 'WEDMmines03'
        command[command.index('-test-keys')+1] += ',5:0x20,8:0x20,11:0x20'
        command += ['-coop', str(args.players)]
    log.with_suffix('.run.json').write_text(json.dumps({'command': command,
        'exe_sha256': hashlib.sha256(exe.read_bytes()).hexdigest()}, indent=2))
    startup = subprocess.STARTUPINFO()
    startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    startup.wShowWindow = 0
    process = subprocess.Popen(command, cwd=ROOT, env=env, startupinfo=startup,
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(name, 'PID', process.pid, flush=True)
    deadline, seen = time.monotonic() + 85, 0
    try:
        while process.poll() is None and time.monotonic() < deadline:
            text = log.read_text(encoding='cp1252') if log.exists() else ''
            lines = [s for s in text.splitlines() if s.startswith(('RIVET-INPUT', 'COOP-TEST FAIL'))
                     or '*** FANG' in s or '*** CRASH' in s]
            for line in lines[seen:]:
                print(line, flush=True)
            seen = len(lines)
            if any(s in text for s in ('RIVET-INPUT COMPLETE', 'COOP-TEST FAIL', '*** FANG', '*** CRASH')):
                break
            time.sleep(.25)
    finally:
        if process.poll() is None:
            process.terminate()
        process.wait(timeout=10)
    assert log.exists(), f'Native client exited before opening the log: exit={process.returncode}'
    text = log.read_text(encoding='cp1252')
    assert 'RIVET-INPUT COMPLETE' in text and not any(s in text for s in ('COOP-TEST FAIL', '*** FANG', '*** CRASH')), log
    print('PASS real input: grenade tap, charged rivet release, scope tap, scoped charge', flush=True)


if __name__ == '__main__':
    main()

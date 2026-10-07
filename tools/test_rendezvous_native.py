"""Exercise the authored console, DET pack and ending in isolated level-29 runs.

The native opt-in fixture supplies only setup/inputs; retail scripts handle
possession, explosions and the ending. This helper closes only its own process.
"""
from pathlib import Path
import argparse, os, re, subprocess, time

root = Path(__file__).resolve().parent.parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--players', type=int, choices=(1, 2, 3, 4), default=2)
parser.add_argument('--mode', choices=('timer', 'ending'), required=True)
parser.add_argument('--exe', type=Path, default=root/'build/encounter-candidate/Release/ma_port.exe')
args = parser.parse_args()
assert args.mode != 'ending' or args.players > 1, 'Ending repair is co-op only.'
mode = 'rendezvous-' + args.mode
log = root/f'build/logs/{mode}-{args.players}-after.log'
save = root/f'build/test-saves/{mode}-{args.players}'
save.mkdir(parents=True, exist_ok=True)
if log.exists(): log.unlink()
exe = args.exe.resolve()
env = dict(os.environ, MA_PORT_TEST_COOP_POLISH=mode)
command = [str(exe), '-data', str(root/'gamedata/files'), '-mission', 'WEMCcity_02',
           '-save-dir', str(save), '-discord-app-id', 'off', '-res', '640x480',
           '-instance-label', f'{mode} {args.players}', '-port-diag', '-log', str(log)]
if args.players > 1: command += ['-coop', str(args.players)]
p = subprocess.Popen(command, cwd=exe.parent, env=env,
                     stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
print('OWN PID', p.pid, flush=True)
expected = ('DET timer is cleared despite changed possession ownership' if args.mode == 'timer'
            else 'END OF LEVEL COMPLETE SCREEN')
try:
    deadline = time.monotonic() + 110
    while p.poll() is None and time.monotonic() < deadline:
        text = log.read_text(errors='replace') if log.exists() else ''
        if expected in text or 'COOP-TEST FAIL' in text or '*** FANG ASSERTION' in text: break
        time.sleep(.3)
finally:
    if p.poll() is None: p.terminate()
    p.wait(timeout=10)
text = log.read_text(errors='replace')
for line in text.splitlines():
    if any(x in line for x in ('COOP-TEST', 'Rendezvous ending returned', 'LOAD MARKER')): print(line)
assert expected in text, 'Test did not reach its final check.'
assert 'COOP-TEST FAIL' not in text and '*** FANG ASSERTION' not in text
if args.mode == 'timer':
    registered = re.search(r'countdown registered: HUD=(\w+) type=0 timer=(\w+)', text)
    assert registered, 'Retail countdown was not registered on a HUD.'
    hud, timer = registered.groups()
    assert f'countdown cleared: HUD={hud} type=0 timer={timer}' in text
if args.mode == 'ending':
    assert 'PASS: ending restores Glitch as dialogue actor' in text
    assert 'Rendezvous ending returned' in text
    completion = re.search(r"natural completion on 'Player0': authored ([\d.]+) elapsed ([\d.]+)", text)
    assert completion, 'No natural dialogue completion diagnostic.'
    authored, elapsed = map(float, completion.groups())
    assert 3.80 < authored < 3.83 and 4.50 < elapsed < 5.2, completion.group()
    print(completion.group())
print('PASS', args.players, 'players:', args.mode)

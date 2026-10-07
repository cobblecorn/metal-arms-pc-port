"""Verify authored Night Sneak fabricators in isolated native campaign runs.

Use --players 1,2,4 to cover solo and co-op, and --machine lower or upper.
Only the process created by each test is closed; no existing game is touched.
"""
from pathlib import Path
import argparse
import os
import re
import subprocess
import time
import uuid

ROOT = Path(__file__).resolve().parent.parent
FINAL = 'COOP-TEST PASS: Jumper exits fabricator with players nearby'
SAMPLE = re.compile(
    r'FABRICATOR-TEST bot=\(([-\d.]+),([-\d.]+),([-\d.]+)\) '
    r'building=(\d+) powered=(\d+) thought=(\S+)')


def verify(text):
    if 'COOP-TEST FAIL' in text or 'FANG ASSERTION' in text:
        raise RuntimeError('Native fixture failed or raised a FANG assertion.')
    if FINAL not in text:
        raise RuntimeError('Native fixture did not reach its final PASS within 50 seconds.')
    origin = None
    built_powered = False
    departed = False
    nearby_departure = False
    resumed_attack = False
    retreated = False
    for line in text.splitlines():
        if 'FABRICATOR-TEST players retreat' in line:
            retreated = True
        sample = SAMPLE.search(line)
        if not sample:
            continue
        x, _, z = map(float, sample.groups()[:3])
        building, powered = map(int, sample.groups()[3:5])
        thought = sample.group(6).upper()
        if building:
            origin = (x, z)
        elif origin is not None:
            built_powered |= bool(powered)
            if (x-origin[0])**2 + (z-origin[1])**2 > 144:
                departed = True
                nearby_departure |= not retreated
            resumed_attack |= departed and powered and 'ATTACK' in thought
    checks = {
        'construction observed': origin is not None,
        'completed bot powered': built_powered,
        'left machine before player retreat': nearby_departure,
        'combat resumed after departure': resumed_attack,
    }
    for label, passed in checks.items():
        print(('PASS' if passed else 'FAIL') + ': ' + label, flush=True)
    missing = [label for label, passed in checks.items() if not passed]
    if missing:
        raise RuntimeError('; '.join(missing))


def run(exe, players, machine):
    mode = 'fabricator-upper' if machine == 'upper' else 'fabricator-nearby'
    run_id = f'{machine}-{players}-{uuid.uuid4().hex[:8]}'
    log = ROOT / 'build/logs/fabricator-native' / (run_id + '.log')
    save = ROOT / 'build/test-saves/fabricator-native' / run_id
    log.parent.mkdir(parents=True, exist_ok=True)
    save.mkdir(parents=True, exist_ok=True)
    log.unlink(missing_ok=True)
    env = dict(os.environ, MA_PORT_TEST_COOP_POLISH=mode)
    env.pop('MA_PORT_COOP_HOLD', None)
    command = [str(exe), '-data', str(ROOT / 'gamedata/files'),
               '-mission', 'WECDsneak02', '-res', '640x480',
               '-save-dir', str(save), '-discord-app-id', 'off',
               '-instance-label', f'Fabricator test {machine} {players}',
               '-log', str(log), '-port-diag']
    if players > 1:
        command += ['-coop', str(players)]
    process = subprocess.Popen(command, cwd=exe.parent, env=env,
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f'OWN PID {process.pid}: {machine}, {players} player(s); log {log}', flush=True)
    deadline = time.monotonic() + 50
    fixture_ready = None
    try:
        while process.poll() is None and time.monotonic() < deadline:
            text = log.read_text(errors='replace') if log.exists() else ''
            if FINAL in text or 'COOP-TEST FAIL' in text or 'FANG ASSERTION' in text:
                break
            if 'Night Sneak fabricator section loaded' in text:
                if fixture_ready is None:
                    fixture_ready = time.monotonic()
                if 'FABRICATOR-TEST bot=' not in text and time.monotonic()-fixture_ready > 15:
                    print('No construction observed after fixture setup; check player/controller readiness.', flush=True)
                    break
            time.sleep(.3)
    finally:
        if process.poll() is None:
            process.terminate()
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait(timeout=5)
    text = log.read_text(errors='replace') if log.exists() else ''
    for line in text.splitlines():
        if 'FABRICATOR-TEST' in line or 'COOP-TEST' in line or 'ASSERTION' in line:
            print(line)
    try:
        verify(text)
    except RuntimeError as error:
        raise RuntimeError(f'{machine}, {players} player(s): {error}; inspect {log}') from error
    print(f'PASS: {machine} fabricator, {players} player(s).', flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--players', default='2', help='One count or comma-separated counts: 1,2,4; four players require a ready join setup.')
    parser.add_argument('--machine', choices=('lower', 'upper'), required=True)
    parser.add_argument('--exe', type=Path, default=ROOT / 'build/encounter-candidate/Release/ma_port.exe')
    args = parser.parse_args()
    try:
        players = [int(count) for count in args.players.split(',')]
    except ValueError:
        parser.error('--players requires comma-separated integers.')
    if not players or any(count not in (1, 2, 4) for count in players):
        parser.error('--players accepts only 1, 2, and 4.')
    exe = args.exe.resolve()
    if not exe.is_file():
        parser.error('Build the candidate first or supply --exe.')
    try:
        for count in dict.fromkeys(players):
            run(exe, count, args.machine)
    except RuntimeError as error:
        raise SystemExit(str(error))


if __name__ == '__main__':
    main()

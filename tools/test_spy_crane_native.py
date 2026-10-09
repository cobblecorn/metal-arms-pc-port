"""Run the real Spy Factory wrench/claw/assembly sequence with isolated saves.

The opt-in native fixture grants the wrench, selects the console's destroyed
mesh and moves the actor between the authored pickup grates. Actual wrench use,
all three claw pickups, conveyor assembly and transmissions run normally.
Injected strafe keys test headless-body movement away from the machine, without
focus. Partners require their normal connected controllers. Only this tool's
game process is closed; personal saves are never used.
"""
from pathlib import Path
import argparse
import hashlib
import os
import subprocess
import time
import uuid

ROOT = Path(__file__).resolve().parents[1]
REQUIRED = (
    "retail wrench disassembles P1 for the crane",
    "head pickup starts the authored crane camera and listener callback",
    "headless body rebuilds and retains P1 human controls",
    "normal keyboard movement moves the headless body while crane camera waits",
    "torso claw collects body while head stays on first crane",
    "all three retail claws collect Glitch parts",
    "crane camera finishes and hands off to authored conveyor assembly stage",
    "retail builder assembly animation loads and starts",
    "completed factory assembly restores intact P1 and human control",
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--players", type=int, choices=(1, 2, 3, 4), default=1)
    parser.add_argument("--exe", type=Path, default=ROOT / "build/Release/ma_port.exe")
    args = parser.parse_args()
    exe = args.exe.resolve()
    run_id = f"{args.players}-{uuid.uuid4().hex[:8]}"
    log_dir = ROOT / "build/logs/spy-crane-native" / run_id
    save = ROOT / "build/test-saves/spy-crane-native" / run_id
    shots = ROOT / "build/shots/spy-crane-native" / run_id
    for directory in (log_dir, save, shots):
        directory.mkdir(parents=True, exist_ok=False)
    log = log_dir / "native.log"
    env = dict(os.environ, MA_PORT_TEST_COOP_POLISH="spy-crane")
    env.pop("MA_PORT_COOP_HOLD", None)
    keys = ",".join(f"g{second}:0x44" for second in range(99, 105))
    command = [str(exe), "-data", str(ROOT / "gamedata/files"),
               "-mission", "WECFfacty01", "-save-dir", str(save),
               "-discord-app-id", "off", "-res", "800x600",
               "-instance-label", f"Spy crane scope test {args.players}",
               "-log", str(log), "-port-diag", "-test-keys", keys,
               "-shots", str(shots), "-shot-every", "900"]
    if args.players > 1:
        command += ["-coop", str(args.players)]
    final = ("solo post-assembly transmission restores normal control"
             if args.players == 1 else
             "partner presence and inspection movement leave P1 responsive")
    digest = hashlib.sha256(exe.read_bytes()).hexdigest()
    process = subprocess.Popen(command, cwd=exe.parent, env=env,
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"OWN PID {process.pid}; EXE SHA256 {digest}; log {log}", flush=True)
    try:
        deadline = time.monotonic() + 300
        while process.poll() is None and time.monotonic() < deadline:
            text = log.read_text(errors="replace") if log.exists() else ""
            if (f"COOP-TEST PASS: {final}" in text or
                    "COOP-TEST FAIL" in text or "*** FANG ASSERTION" in text):
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
    text = log.read_text(errors="replace") if log.exists() else ""
    for line in text.splitlines():
        if "COOP-TEST" in line or line.startswith("*** "):
            print(line)
    missing = [check for check in (*REQUIRED, final)
               if f"COOP-TEST PASS: {check}" not in text]
    errors = [line for line in text.splitlines()
              if "COOP-TEST FAIL" in line or line.startswith("*** ") or
              "[ FAUDIO ] Error" in line or "SCRIPT ERROR" in line.upper()]
    if missing or errors:
        raise SystemExit(f"FAIL: missing checks {missing}; runtime errors {errors}; log {log}")
    print(f"PASS: {args.players} player(s), {text.count('COOP-TEST PASS:')} native checks, "
          f"all claw pickups and assembly completed; log {log}")


if __name__ == "__main__":
    main()

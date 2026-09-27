"""Run the game for a test and summarize what its log says.

Runs build/<config>/ma_port.exe from the repository root with the retail data in gamedata/files. Test runs
are muted and keep Discord off (the user often has their own game open), log with -port-diag to
build/logs/<name>.log, present without vsync (so frame times show the real cost), and are closed after
--seconds. Only the process this script started is closed; never the user's own game.

usage: python tools/port_run.py [options] [-- extra game arguments]
  --config Release|Debug   which build (default Release: the one to play; Debug has the engine asserts)
  --mission WORLD          start that campaign mission (e.g. wedmmines01); default: the front end
  --seconds N              how long to run (default 60)
  --name NAME              log/shot name (default: the mission or "front")
  --shots FRAMES           save a back-buffer BMP every FRAMES frames to build/shots/NAME/ (cleared first)
  --test-keys KEYS         e.g. "g8:0x1B" presses Escape 8 s after gameplay starts (pauses a mission);
                           "30:0x0D" presses Enter 30 s after launch (no g: from process start)
  --stall-ms MS            log the game thread's stack for frames longer than MS (default 100)
  --vsync                  keep vsync on
  --audio                  play audio (only when the user asked to hear something)
  --keep                   start the game and return at once, printing its PID (drive it with
                           tools/menu_drive.py --pid PID ..., then run --stop PID --name NAME)
  --stop PID               close that ma_port.exe process, then summarize --name's log
  --summary                only summarize build/logs/NAME.log (and the newest shot)

The summary lists: PORT-PERF lines (fps, worst frame, work before Present; the work figure only means
something without vsync), the longest PORT-HITCH frames, each PORT-STALL stack (the game thread's code
frames), crash/assert/run-time-check reports, and audio errors. With --shots, the newest shot is also
saved as build/shots/NAME/latest.png (needs Pillow).
"""
import argparse
import glob
import os
import re
import shutil
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def summarize(name):
    log_path = os.path.join(ROOT, "build", "logs", name + ".log")
    if not os.path.exists(log_path):
        print("no log at", log_path)
        return 1
    with open(log_path, "r", encoding="latin-1") as handle:
        lines = handle.read().splitlines()
    print("== %s (%d lines)" % (log_path, len(lines)))

    perf = [line for line in lines if line.startswith("PORT-PERF")]
    print("\n-- frame rate (PORT-PERF, every 10 s)")
    for line in perf:
        print("  " + line[len("PORT-PERF "):])
    if not perf:
        print("  (none: the run was shorter than 10 s of drawing, or -port-diag was off)")

    hitches = []
    for line in lines:
        match = re.match(r"PORT-HITCH frame (\d+): ([\d.]+) ms", line)
        if match:
            hitches.append((float(match.group(2)), int(match.group(1))))
    print("\n-- hitches: %d frames over 40 ms and 2.5x the average" % len(hitches))
    for ms, frame in sorted(hitches, reverse=True)[:8]:
        print("  frame %d: %.1f ms" % (frame, ms))

    print("\n-- stalls (PORT-STALL: where the game thread was during a long frame)")
    stalls = 0
    for index, line in enumerate(lines):
        if not line.startswith("PORT-STALL"):
            continue
        stalls += 1
        frames = []
        for follow in lines[index + 1:index + 60]:
            if follow.startswith("PORT-STALL"):
                break
            if follow.startswith("    #"):
                frames.append(follow.strip())
            if len(frames) >= 14:
                break
        named = [f for f in frames if "(" in f][:7] or frames[:5]
        print("  " + line[len("PORT-STALL "):])
        for frame in named:
            print("      " + re.sub(r"^#\d+ 0x[0-9a-f]+ ", "", frame).replace(ROOT + os.sep, "").replace(ROOT.replace("/", "\\") + "\\", ""))
    if not stalls:
        print("  none")

    print("\n-- crashes, asserts, run-time checks")
    reports = 0
    for index, line in enumerate(lines):
        if line.startswith("*** "):
            reports += 1
            print("  " + line)
            for follow in lines[index + 1:index + 8]:
                if follow.startswith("    "):
                    print("  " + follow)
    if not reports:
        print("  none")

    audio_errors = [line for line in lines if "[ FAUDIO ] Error" in line]
    print("\n-- audio errors: %d" % len(audio_errors))
    for line in audio_errors[:10]:
        print("  " + line)

    shots = sorted(glob.glob(os.path.join(ROOT, "build", "shots", name, "*.bmp")))
    if shots:
        print("\n-- shots: %d in build/shots/%s" % (len(shots), name))
        try:
            from PIL import Image
            for path in reversed(shots[-2:]):
                try:
                    Image.open(path).save(os.path.join(ROOT, "build", "shots", name, "latest.png"))
                    print("  newest: build/shots/%s/latest.png (from %s)" % (name, os.path.basename(path)))
                    break
                except Exception:
                    continue
        except ImportError:
            print("  (install Pillow to get latest.png)")
    return 0


def stop(pid):
    # only ever close a ma_port.exe
    result = subprocess.run(["tasklist", "/FI", "PID eq %d" % pid, "/FO", "CSV", "/NH"], capture_output=True, text=True)
    if "ma_port.exe" not in result.stdout.lower():
        print("PID %d is not ma_port.exe; not closing it" % pid)
        return
    subprocess.run(["taskkill", "/F", "/PID", str(pid)], capture_output=True)
    time.sleep(0.3)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--config", default="Release", choices=["Release", "Debug"])
    parser.add_argument("--mission")
    parser.add_argument("--seconds", type=float, default=60.0)
    parser.add_argument("--name")
    parser.add_argument("--shots", type=int)
    parser.add_argument("--test-keys")
    parser.add_argument("--stall-ms", type=int)
    parser.add_argument("--vsync", action="store_true")
    parser.add_argument("--audio", action="store_true")
    parser.add_argument("--keep", action="store_true")
    parser.add_argument("--stop", type=int)
    parser.add_argument("--summary", action="store_true")
    parser.add_argument("extra", nargs="*")
    args = parser.parse_args()
    name = args.name or args.mission or "front"

    if args.summary:
        return summarize(name)
    if args.stop:
        stop(args.stop)
        return summarize(name)

    exe = os.path.join(ROOT, "build", args.config, "ma_port.exe")
    if not os.path.exists(exe):
        print("build it first: cmake --build build --config %s --target ma_port" % args.config)
        return 1
    os.makedirs(os.path.join(ROOT, "build", "logs"), exist_ok=True)
    command = [exe, "-data", "gamedata/files", "-port-diag", "-discord-app-id", "off",
               "-log", "build/logs/%s.log" % name]
    if args.mission:
        command += ["-mission", args.mission]
    if not args.vsync:
        command.append("-no-vsync")
    if not args.audio:
        command.append("-no-audio")
    if args.shots:
        shot_dir = os.path.join(ROOT, "build", "shots", name)
        shutil.rmtree(shot_dir, ignore_errors=True)
        os.makedirs(shot_dir)
        command += ["-shots", "build/shots/" + name, "-shot-every", str(args.shots)]
    if args.test_keys:
        command += ["-test-keys", args.test_keys]
    command += args.extra
    environment = dict(os.environ)
    if args.stall_ms:
        environment["MA_PORT_STALL_MS"] = str(args.stall_ms)

    process = subprocess.Popen(command, cwd=ROOT, env=environment,
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print("started PID %d: %s" % (process.pid, " ".join(command[1:])))
    if args.keep:
        print("drive it with: python tools/menu_drive.py --pid %d ..." % process.pid)
        print("then: python tools/port_run.py --stop %d --name %s" % (process.pid, name))
        return 0
    deadline = time.time() + args.seconds
    while time.time() < deadline and process.poll() is None:
        time.sleep(0.5)
    if process.poll() is None:
        process.kill()
        process.wait()
    else:
        print("the game exited by itself (code %s)" % process.returncode)
    time.sleep(0.3)
    return summarize(name)


if __name__ == "__main__":
    sys.exit(main())

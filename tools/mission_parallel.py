"""Launch and report several isolated mission tests at once.

usage: python tools/mission_parallel.py [options] WORLD [WORLD ...]
  --config Debug|Release   build to run (default Debug for asserts)
  --seconds N              max run time per mission (default 60)
  --jobs N                 simultaneous instances (default 4)
  --coop N                 experimental 2-4 player campaign load for each mission
  --run-name NAME          log prefix (default: current timestamp)
  --shots FRAMES           capture a BMP every FRAMES frames
  --stall-ms MS            capture game-thread stacks above this frame time
  --audio                  explicit; audio is enabled by default in every instance
  --mute                   run silently in every instance (Bink audio is disabled too)
  --no-audio               skip audio setup entirely (faster loads; sound errors are then meaningless)
  --test-keys KEYS         passed to every instance, e.g. "10:0x20,14:0x20,18:0x20,24:0x20" (Space skips
                           intro movies and only jumps in gameplay)
  --game-args "ARGS"       extra game arguments passed to every instance (e.g. "-test-win-level 60")
  --quiet                  skip the per-mission summaries (use tools/log_errors.py on the run instead)

Every instance has separate engine, asset, screenshot, and save paths. Audio is on and Discord is off
by default. It terminates only the exact game processes it starts.
"""
import argparse
import os
import re
import shlex
import subprocess
import sys
import time
from datetime import datetime

from port_run import summarize

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def slug(value):
    return re.sub(r"[^A-Za-z0-9_-]+", "_", value).strip("_") or "mission"


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--config", default="Debug", choices=["Release", "Debug"])
    parser.add_argument("--seconds", type=float, default=60.0)
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--coop", type=int, choices=[2, 3, 4])
    parser.add_argument("--run-name")
    parser.add_argument("--shots", type=int)
    parser.add_argument("--stall-ms", type=int)
    audio_group = parser.add_mutually_exclusive_group()
    audio_group.add_argument("--audio", action="store_true", help="enable audio (the default)")
    audio_group.add_argument("--mute", action="store_true", help="run silently in every instance")
    audio_group.add_argument("--no-audio", action="store_true", help="skip audio setup")
    parser.add_argument("--test-keys")
    parser.add_argument("--game-args", default="", help="extra game arguments for every instance, e.g. \"-test-win-level 60\"")
    parser.add_argument("--quiet", action="store_true")
    parser.add_argument("missions", nargs="+")
    args = parser.parse_args()

    if args.seconds <= 0 or args.jobs <= 0 or (args.shots is not None and args.shots <= 0):
        parser.error("seconds, jobs, and shots must be positive")

    exe = os.path.join(ROOT, "build", args.config, "ma_port.exe")
    if not os.path.exists(exe):
        print("build it first: cmake --build build --config %s --target ma_port" % args.config)
        return 1

    run_name = slug(args.run_name or datetime.now().strftime("%Y%m%d_%H%M%S"))[:40]
    log_dir = os.path.join(ROOT, "build", "logs")
    shot_root = os.path.join(ROOT, "build", "shots")
    save_root = os.path.join(ROOT, "build", "test-saves", run_name)
    os.makedirs(log_dir, exist_ok=True)
    os.makedirs(save_root, exist_ok=True)

    jobs = []
    for index, mission in enumerate(args.missions, 1):
        log_name = "%s_%02d_%s" % (run_name, index, slug(mission))
        shot_dir = os.path.join(shot_root, log_name)
        save_dir = os.path.join(save_root, "%02d_%s" % (index, slug(mission)))
        os.makedirs(save_dir, exist_ok=True)
        command = [exe, "-data", "gamedata/files", "-port-diag", "-discord-app-id", "off",
                   "-log", "build/logs/%s.log" % log_name,
                   "-asset-log", "build/logs/%s-assets.log" % log_name,
                   "-mission", mission, "-no-vsync", "-save-dir", save_dir,
                   "-instance-label", log_name]
        if args.no_audio:
            command.append("-no-audio")
        elif args.mute:
            command.append("-mute")
        if args.coop:
            command += ["-coop", str(args.coop)]
        if args.test_keys:
            command += ["-test-keys", args.test_keys]
        if args.game_args:
            command += shlex.split(args.game_args)
        if args.shots:
            os.makedirs(shot_dir, exist_ok=True)
            command += ["-shots", os.path.join("build", "shots", log_name), "-shot-every", str(args.shots)]
        jobs.append((mission, log_name, command))

    pending = list(jobs)
    active = {}
    completed = []
    audio_mode = "no audio" if args.no_audio else ("muted" if args.mute else "audio enabled")
    print("Run %s: %d mission(s), up to %d concurrent, %.1f s each (%s, %s)" %
          (run_name, len(jobs), args.jobs, args.seconds, args.config, audio_mode))
    try:
        while pending or active:
            while pending and len(active) < args.jobs:
                mission, log_name, command = pending.pop(0)
                env = dict(os.environ)
                if args.stall_ms:
                    env["MA_PORT_STALL_MS"] = str(args.stall_ms)
                process = subprocess.Popen(command, cwd=ROOT, env=env,
                                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                active[process] = (mission, log_name, time.monotonic() + args.seconds)
                print("started %-20s PID %d -> build/logs/%s.log" % (mission, process.pid, log_name))

            now = time.monotonic()
            for process, (mission, log_name, deadline) in list(active.items()):
                exit_code = process.poll()
                timed_out = False
                if exit_code is None and now >= deadline:
                    process.kill()
                    exit_code = process.wait()
                    timed_out = True
                if exit_code is not None:
                    completed.append((mission, log_name, exit_code, timed_out))
                    del active[process]
            if active:
                time.sleep(0.25)
    except KeyboardInterrupt:
        for process in active:
            process.kill()
        for process in active:
            process.wait()
        print("Interrupted; stopped only the test processes started by this run.")
        return 130

    by_name = {log_name: (mission, exit_code, timed_out)
               for mission, log_name, exit_code, timed_out in completed}
    if args.quiet:
        for mission, log_name, _command in jobs:
            status = by_name[log_name]
            if not status[2]:
                print("%s exited early (code %s): build/logs/%s.log" % (mission, status[1], log_name))
        print("errors across the run: python tools/log_errors.py build/logs/%s_*.log" % run_name)
        return 0 if all(exit_code == 0 or timed_out for _, _, exit_code, timed_out in completed) else 1
    for mission, log_name, _command in jobs:
        status = by_name[log_name]
        print("\n== %s: %s (exit %s) ==" %
              (mission, "timed out as planned" if status[2] else "exited", status[1]))
        summarize(log_name)
    return 0 if all(exit_code == 0 or timed_out for _, _, exit_code, timed_out in completed) else 1


if __name__ == "__main__":
    sys.exit(main())

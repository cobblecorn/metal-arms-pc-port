"""Drive the game's menus without touching the real mouse or keyboard.

Posts mouse messages to a running ma_port.exe window. The port's menu pointer reads its position and
clicks from these window messages, so this works while the window is in the background and never moves
the desktop cursor. Keys cannot be posted this way (the game reads them with GetAsyncKeyState, which also
needs focus); use the game's -test-keys option for keys instead.

Coordinates are client pixels of the game window (1280x960 by default); the *f commands take fractions
of the client area (0..1) instead.

usage: python tools/menu_drive.py [--pid PID] COMMAND ...
  info                    window handle and client size
  move X Y                move the pointer there (a few steps, as a real mouse would)
  click X Y               move there, then left click
  rclick X Y              right click (Back in menus)
  wheel N                 N wheel notches (+ = up/away)
  fastclicks X Y [X Y..]  left clicks in quick succession (tests the click queue)
  fx FX FY | clickf FX FY | rclickf FX FY    the same with fractions
  waitpause DIR [SECS]    wait until the newest -shots frame in DIR looks like the pause menu (mostly
                          dark blue); prints its path; exit code 1 on timeout. Needs Pillow.

--pid picks the window of that process (from tools/port_run.py --keep); without it, the first ma_port
window found is used, which may be the user's own game. Useful pause-menu spots at 1280x960: Advanced
Settings 630,296; Audio Levels 545,366; Controller Map 570,438.
"""
import ctypes
import ctypes.wintypes as wt
import glob
import sys
import time

user32 = ctypes.WinDLL("user32", use_last_error=True)
kernel32 = ctypes.WinDLL("kernel32")
WM_MOUSEMOVE, WM_LBUTTONDOWN, WM_LBUTTONUP = 0x0200, 0x0201, 0x0202
WM_RBUTTONDOWN, WM_RBUTTONUP, WM_MOUSEWHEEL = 0x0204, 0x0205, 0x020A


def find_window(pid=None):
    found = []
    enum_proc = ctypes.WINFUNCTYPE(wt.BOOL, wt.HWND, wt.LPARAM)

    def callback(hwnd, _):
        if not user32.IsWindowVisible(hwnd):
            return True
        window_pid = wt.DWORD()
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(window_pid))
        if pid is not None:
            if window_pid.value == pid:
                found.append(hwnd)
            return True
        handle = kernel32.OpenProcess(0x1000, False, window_pid.value)
        if handle:
            name = ctypes.create_unicode_buffer(260)
            size = wt.DWORD(260)
            kernel32.QueryFullProcessImageNameW(handle, 0, name, ctypes.byref(size))
            kernel32.CloseHandle(handle)
            if name.value.lower().endswith("ma_port.exe"):
                found.append(hwnd)
        return True

    user32.EnumWindows(enum_proc(callback), 0)
    return found[0] if found else None


def client_size(hwnd):
    rect = wt.RECT()
    user32.GetClientRect(hwnd, ctypes.byref(rect))
    return rect.right, rect.bottom


def lparam(x, y):
    return (int(y) << 16) | (int(x) & 0xFFFF)


def post(hwnd, message, wparam, lp):
    user32.PostMessageW(hwnd, message, wparam, lp)


def move_smooth(hwnd, x, y, steps=4):
    for i in range(1, steps + 1):
        post(hwnd, WM_MOUSEMOVE, 0, lparam(x - (steps - i) * 3, y))
        time.sleep(0.03)


def looks_like_pause_menu(path):
    from PIL import Image
    image = Image.open(path).convert("RGB").resize((16, 12))
    pixels = list(image.getdata())
    r = sum(p[0] for p in pixels) / len(pixels)
    g = sum(p[1] for p in pixels) / len(pixels)
    b = sum(p[2] for p in pixels) / len(pixels)
    return b > 40 and b > 2.5 * r and b > 2 * g


def wait_pause(folder, seconds):
    deadline = time.time() + seconds
    while time.time() < deadline:
        shots = sorted(glob.glob(folder + "/*.bmp"))
        # the newest file may still be being written; check the one before it too
        for path in reversed(shots[-2:]):
            try:
                if looks_like_pause_menu(path):
                    print(path)
                    return 0
            except Exception:
                pass
        time.sleep(0.5)
    print("pause menu not seen")
    return 1


def main(argv):
    pid = None
    if len(argv) >= 2 and argv[0] == "--pid":
        pid = int(argv[1])
        argv = argv[2:]
    if not argv:
        print(__doc__)
        return 2
    command, args = argv[0], argv[1:]
    if command == "waitpause":
        return wait_pause(args[0], float(args[1]) if len(args) > 1 else 60.0)

    hwnd = find_window(pid)
    if not hwnd:
        print("no ma_port window" + (" for pid %d" % pid if pid else ""))
        return 1
    width, height = client_size(hwnd)
    if command in ("fx", "clickf", "rclickf"):
        args = [float(args[0]) * width, float(args[1]) * height]
        command = {"fx": "move", "clickf": "click", "rclickf": "rclick"}[command]

    if command == "info":
        print("hwnd", hwnd, "client", width, height)
    elif command == "move":
        move_smooth(hwnd, float(args[0]), float(args[1]))
    elif command == "click":
        x, y = float(args[0]), float(args[1])
        move_smooth(hwnd, x, y)
        time.sleep(0.1)
        post(hwnd, WM_LBUTTONDOWN, 1, lparam(x, y))
        time.sleep(0.12)
        post(hwnd, WM_LBUTTONUP, 0, lparam(x, y))
    elif command == "rclick":
        x, y = float(args[0]), float(args[1])
        post(hwnd, WM_RBUTTONDOWN, 2, lparam(x, y))
        time.sleep(0.12)
        post(hwnd, WM_RBUTTONUP, 0, lparam(x, y))
    elif command == "fastclicks":
        points = [(float(args[k]), float(args[k + 1])) for k in range(0, len(args), 2)]
        for x, y in points:
            post(hwnd, WM_MOUSEMOVE, 0, lparam(x, y))
            post(hwnd, WM_LBUTTONDOWN, 1, lparam(x, y))
            time.sleep(0.02)
            post(hwnd, WM_LBUTTONUP, 0, lparam(x, y))
            time.sleep(0.02)
    elif command == "wheel":
        notches = int(args[0])
        for _ in range(abs(notches)):
            post(hwnd, WM_MOUSEWHEEL, ((120 if notches > 0 else -120) & 0xFFFF) << 16, 0)
            time.sleep(0.25)
    else:
        print(__doc__)
        return 2
    print("ok", command, width, height)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

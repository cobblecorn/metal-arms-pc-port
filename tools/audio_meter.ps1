# Samples the peak output level of a process's audio session on the default playback device,
# so audio can be checked without listening. Prints one line per sample and a summary.
#
#   powershell -ExecutionPolicy Bypass -File tools\audio_meter.ps1 -ProcessName ma_port -Seconds 30
#
# A peak of 0 for the whole run means the process made no sound (or has no session yet).
param(
	[string]$ProcessName = "ma_port",
	[int]$Seconds = 30,
	[int]$IntervalMs = 250
)

Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;

[Guid("A95664D2-9614-4F35-A746-DE8DB63617E6"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
interface IMMDeviceEnumerator {
	int EnumAudioEndpoints(int dataFlow, int stateMask, out IntPtr devices);
	int GetDefaultAudioEndpoint(int dataFlow, int role, out IMMDevice device);
}

[Guid("D666063F-1587-4E43-81F1-B948E807363F"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
interface IMMDevice {
	int Activate(ref Guid iid, int clsCtx, IntPtr activationParams, [MarshalAs(UnmanagedType.IUnknown)] out object iface);
}

[Guid("77AA99A0-1BD6-484F-8BC7-2C654C9A9B6F"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
interface IAudioSessionManager2 {
	int GetAudioSessionControl(IntPtr sessionGuid, int flags, out IntPtr control);
	int GetSimpleAudioVolume(IntPtr sessionGuid, int flags, out IntPtr volume);
	int GetSessionEnumerator(out IAudioSessionEnumerator sessions);
}

[Guid("E2F5BB11-0570-40CA-ACDD-3AA01277DEE8"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
interface IAudioSessionEnumerator {
	int GetCount(out int count);
	int GetSession(int index, out IAudioSessionControl2 session);
}

[Guid("bfb7ff88-7239-4fc9-8fa2-07c950be9c6d"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
interface IAudioSessionControl2 {
	// IAudioSessionControl
	int GetState(out int state);
	int GetDisplayName(out IntPtr name);
	int SetDisplayName(IntPtr name, IntPtr context);
	int GetIconPath(out IntPtr path);
	int SetIconPath(IntPtr path, IntPtr context);
	int GetGroupingParam(out Guid param);
	int SetGroupingParam(ref Guid param, IntPtr context);
	int RegisterAudioSessionNotification(IntPtr client);
	int UnregisterAudioSessionNotification(IntPtr client);
	// IAudioSessionControl2
	int GetSessionIdentifier(out IntPtr id);
	int GetSessionInstanceIdentifier(out IntPtr id);
	int GetProcessId(out uint pid);
}

[Guid("C02216F6-8C67-4B5B-9D00-D008E73E0064"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
interface IAudioMeterInformation {
	int GetPeakValue(out float peak);
}

[ComImport, Guid("BCDE0395-E52F-467C-8E3D-C4579291692E")]
class MMDeviceEnumeratorCom {}

public static class AudioMeter {
	// Returns the highest current peak across the process's sessions, or -1 if it has none.
	public static float Peak(uint pid) {
		IMMDeviceEnumerator enumerator = (IMMDeviceEnumerator)(new MMDeviceEnumeratorCom());
		IMMDevice device;
		Marshal.ThrowExceptionForHR(enumerator.GetDefaultAudioEndpoint(0, 1, out device));
		Guid iid = typeof(IAudioSessionManager2).GUID;
		object managerObject;
		Marshal.ThrowExceptionForHR(device.Activate(ref iid, 23, IntPtr.Zero, out managerObject));
		IAudioSessionEnumerator sessions;
		Marshal.ThrowExceptionForHR(((IAudioSessionManager2)managerObject).GetSessionEnumerator(out sessions));
		int count;
		sessions.GetCount(out count);
		float best = -1.0f;
		for (int i = 0; i < count; i++) {
			IAudioSessionControl2 session;
			if (sessions.GetSession(i, out session) != 0) continue;
			uint sessionPid;
			if (session.GetProcessId(out sessionPid) != 0 || sessionPid != pid) continue;
			float peak;
			if (((IAudioMeterInformation)session).GetPeakValue(out peak) == 0 && peak > best) best = peak;
		}
		return best;
	}
}
"@

$deadline = (Get-Date).AddSeconds($Seconds)
$samples = 0; $audible = 0; $maxPeak = 0.0
while ((Get-Date) -lt $deadline) {
	$process = Get-Process -Name $ProcessName -ErrorAction SilentlyContinue | Select-Object -First 1
	if ($process) {
		$peak = [AudioMeter]::Peak([uint32]$process.Id)
		$samples++
		if ($peak -gt 0.001) { $audible++ }
		if ($peak -gt $maxPeak) { $maxPeak = $peak }
		"{0:HH:mm:ss.f} peak {1:N3}" -f (Get-Date), $peak
	}
	Start-Sleep -Milliseconds $IntervalMs
}
"samples $samples, audible $audible, max peak {0:N3}" -f $maxPeak

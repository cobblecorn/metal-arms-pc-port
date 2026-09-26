#!/bin/sh
# Launches each registered mission for a few seconds and summarizes what went wrong.
# Usage (from the repo root, in the foreground - a backgrounded loop can outlive its caller):
#   sh tools/mission_sweep.sh <seconds> <world>...
# Logs go to build/logs/sweep/<world>.log (derived from retail data: keep them under build/).
SECS=$1; shift
OUT=build/logs/sweep
mkdir -p $OUT
for W in "$@"; do
	L=$OUT/$W.log
	timeout $SECS ./build/Debug/ma_port.exe -mission $W -log $L > $OUT/$W.out 2>&1
	RC=$?
	taskkill //F //IM ma_port.exe > /dev/null 2>&1
	LOADED=$(grep -c "END OF LOADING" $L)
	CRASH=$(grep -c '\*\*\* CRASH' $L)
	ASSERTS=$(grep -c 'CRT ASSERT' $L)
	SCRIPT=$(grep -c 'SCRIPT ERROR' $L)
	DATA=$(grep -c -i -E 'could not read|trouble parsing|problem while|mismatch|Error in definition|has only|not of the type' $L)
	BANKS=$(grep -c 'LOADED SFX BANK' $L)
	AUDIO=$(grep -c -E '\[ FAUDIO \] Error|gcaudio: .*(not|could)|could not load the referenced wav bank' $L)
	STREAMS=$(grep -c "Stream '.*' ready" $L)
	MEMORY=$(grep -c -i -E 'not enough memory|out of memory|fres_Alloc.*fail|failed to allocate' $L)
	echo "$W rc=$RC loaded=$LOADED crash=$CRASH asserts=$ASSERTS script=$SCRIPT data=$DATA sfx_banks=$BANKS audio_errors=$AUDIO streams=$STREAMS memory=$MEMORY"
done

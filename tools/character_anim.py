"""Decode a raw, big-endian Metal Arms ``.mtx`` animation resource.

The result is deliberately plain JSON data so it can be consumed by Blender,
asset converters, or scripts outside the game runtime. Track times are emitted
in seconds and quaternion values use ``[x, y, z, w]`` order.

The serialized layout and compression constants mirror ``FAnim_t`` in
``ma/Lib/Fang2/fanim.h`` and the validation/conversion in ``port/gcdata.cpp``.
"""

from __future__ import annotations

import json
import math
import os
import struct
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple, Union


HEADER_SIZE = 32
BONE_SIZE = 36
MAX_BONES = 127  # FDATA_MAX_BONE_COUNT

COMP_TRANSLATION = 0x0001
COMP_ORIENTATION = 0x0002
TIME_8BIT = 0x0010
TIME_16BIT = 0x0020
FRAMECOUNT_8BIT = 0x0040
KNOWN_FLAGS = COMP_TRANSLATION | COMP_ORIENTATION | TIME_8BIT | TIME_16BIT | FRAMECOUNT_8BIT


class AnimationFormatError(ValueError):
    """Raised when an ``.mtx`` file is truncated or structurally invalid."""


def _range(data: bytes, offset: int, count: int, stride: int, label: str,
           min_offset: int = 0, min_count: int = 0,
           alignment: int = 1) -> Tuple[int, int]:
    if count < min_count:
        raise AnimationFormatError(f"{label}: expected at least {min_count} keys, got {count}")
    if offset < min_offset:
        raise AnimationFormatError(f"{label}: invalid offset {offset}")
    if alignment > 1 and offset % alignment:
        raise AnimationFormatError(f"{label}: offset {offset} is not {alignment}-byte aligned")
    size = count * stride
    if size < 0 or offset > len(data) or size > len(data) - offset:
        raise AnimationFormatError(
            f"{label}: range [{offset}, {offset + size}) exceeds resource size {len(data)}"
        )
    return offset, offset + size


def _read_c_string(data: bytes, offset: int, label: str, lower_bound: int) -> str:
    if offset < lower_bound or offset >= len(data):
        raise AnimationFormatError(f"{label}: invalid string offset {offset}")
    end = data.find(b"\0", offset)
    if end < 0:
        raise AnimationFormatError(f"{label}: unterminated string")
    value = data[offset:end].decode("latin-1")
    if not value:
        raise AnimationFormatError(f"{label}: empty bone name")
    return value


def _finite(value: float, label: str) -> float:
    if not math.isfinite(value):
        raise AnimationFormatError(f"{label}: non-finite value {value!r}")
    return value


def _decode_time_values(data: bytes, offset: int, count: int, flags: int,
                        duration: float, label: str) -> List[float]:
    if flags & TIME_8BIT:
        raw = list(data[offset:offset + count])
        unit = 128.0
    elif flags & TIME_16BIT:
        raw = [v[0] for v in struct.iter_unpack(">H", data[offset:offset + count * 2])]
        unit = 32768.0
    else:
        raw = [v[0] for v in struct.iter_unpack(">f", data[offset:offset + count * 4])]
        unit = 1.0

    times = [_finite((float(v) / unit) * duration, label) for v in raw]
    previous = -math.inf
    for key_index, time in enumerate(times):
        if time < previous:
            raise AnimationFormatError(f"{label}: key times are not monotonic at index {key_index}")
        previous = time
    return times


def _decode_scales(data: bytes, offset: int, count: int, label: str) -> List[float]:
    values = [v[0] for v in struct.iter_unpack(">f", data[offset:offset + count * 4])]
    return [_finite(value, f"{label}[{i}]") for i, value in enumerate(values)]


def _decode_translations(data: bytes, offset: int, count: int, compressed: bool,
                         label: str) -> List[List[float]]:
    values: List[List[float]] = []
    if compressed:
        for i, row in enumerate(struct.iter_unpack(">hhh", data[offset:offset + count * 6])):
            values.append([_finite(component / 256.0, f"{label}[{i}]") for component in row])
    else:
        for i, row in enumerate(struct.iter_unpack(">fff", data[offset:offset + count * 12])):
            values.append([_finite(component, f"{label}[{i}]") for component in row])
    return values


def _decode_orientations(data: bytes, offset: int, count: int, compressed: bool,
                         label: str) -> List[List[float]]:
    values: List[List[float]] = []
    if compressed:
        for i, row in enumerate(struct.iter_unpack(">hhhh", data[offset:offset + count * 8])):
            values.append([_finite(component / 16384.0, f"{label}[{i}]") for component in row])
    else:
        for i, row in enumerate(struct.iter_unpack(">ffff", data[offset:offset + count * 16])):
            values.append([_finite(component, f"{label}[{i}]") for component in row])
    return values


def parse_animation(resource: Union[bytes, bytearray, memoryview],
                    resource_name: Optional[str] = None) -> Dict[str, Any]:
    """Parse raw GameCube ``.mtx`` bytes into a JSON-compatible animation.

    ``resource_name`` is used if the serialized 16-byte animation name is
    empty; passing a filename such as ``arzaidle001.mtx`` yields the name
    ``arzaidle001``. The source stores times in normalized animation units,
    so emitted key times are converted to seconds using the header duration.
    """
    data = bytes(resource)
    if len(data) < HEADER_SIZE:
        raise AnimationFormatError(f"truncated animation header ({len(data)} bytes)")

    header_name = data[:16].split(b"\0", 1)[0].decode("latin-1").strip()
    flags, bone_count, duration, reciprocal, bone_offset = struct.unpack_from(">HHffI", data, 16)
    if flags & ~KNOWN_FLAGS:
        raise AnimationFormatError(f"unknown animation flags 0x{flags:04x}")
    if (flags & TIME_8BIT) and (flags & TIME_16BIT):
        raise AnimationFormatError("8-bit and 16-bit time encodings are both enabled")
    if not 0 < bone_count <= MAX_BONES:
        raise AnimationFormatError(f"invalid bone count {bone_count}")
    duration = _finite(duration, "total duration")
    reciprocal = _finite(reciprocal, "reciprocal duration")
    if duration <= 0.0:
        raise AnimationFormatError(f"invalid total duration {duration}")
    # Some source resources round the reciprocal independently, so only reject
    # grossly inconsistent values rather than requiring an exact float match.
    if reciprocal <= 0.0 or abs((duration * reciprocal) - 1.0) > 0.02:
        raise AnimationFormatError(
            f"duration fields disagree (duration={duration}, reciprocal={reciprocal})"
        )
    bone_start, bone_end = _range(
        data, bone_offset, bone_count, BONE_SIZE, "bone table", min_offset=HEADER_SIZE,
        alignment=4,
    )

    if flags & TIME_8BIT:
        time_stride = 1
    elif flags & TIME_16BIT:
        time_stride = 2
    else:
        time_stride = 4
    trans_stride = 6 if flags & COMP_TRANSLATION else 12
    orient_stride = 8 if flags & COMP_ORIENTATION else 16
    trans_alignment = 2 if flags & COMP_TRANSLATION else 4
    orient_alignment = 2 if flags & COMP_ORIENTATION else 4

    tracks: List[Dict[str, Any]] = []
    array_ranges: List[Tuple[int, int, str]] = []

    for bone_index in range(bone_count):
        bone_pos = bone_start + bone_index * BONE_SIZE
        (name_offset,) = struct.unpack_from(">I", data, bone_pos)
        s_count, t_count, o_count = struct.unpack_from(">HHH", data, bone_pos + 4)
        s_time_offset, t_time_offset, o_time_offset = struct.unpack_from(">III", data, bone_pos + 12)
        s_data_offset, t_data_offset, o_data_offset = struct.unpack_from(">III", data, bone_pos + 24)
        bone_name = _read_c_string(data, name_offset, f"bone {bone_index} name", bone_end)

        counts = (s_count, t_count, o_count)
        time_offsets = (s_time_offset, t_time_offset, o_time_offset)
        data_offsets = (s_data_offset, t_data_offset, o_data_offset)
        data_strides = (4, trans_stride, orient_stride)
        data_alignments = (4, trans_alignment, orient_alignment)
        track_labels = ("scale", "translation", "rotation")
        validated: List[Tuple[int, int, int, int]] = []
        for kind, count in enumerate(counts):
            label = f"bone {bone_name} {track_labels[kind]}"
            if (flags & FRAMECOUNT_8BIT) and count >= 256:
                raise AnimationFormatError(f"{label}: 8-bit frame count cannot represent {count} keys")
            time_range = _range(data, time_offsets[kind], count, time_stride,
                                f"{label} times", min_offset=bone_end, min_count=2,
                                alignment=time_stride)
            data_range = _range(data, data_offsets[kind], count, data_strides[kind],
                                f"{label} data", min_offset=bone_end, min_count=2,
                                alignment=data_alignments[kind])
            array_ranges.append((*time_range, f"{label} times"))
            array_ranges.append((*data_range, f"{label} data"))

        s_times = _decode_time_values(data, s_time_offset, s_count, flags, duration,
                                      f"bone {bone_name} scale times")
        t_times = _decode_time_values(data, t_time_offset, t_count, flags, duration,
                                      f"bone {bone_name} translation times")
        o_times = _decode_time_values(data, o_time_offset, o_count, flags, duration,
                                      f"bone {bone_name} rotation times")
        s_values = _decode_scales(data, s_data_offset, s_count, f"bone {bone_name} scale")
        t_values = _decode_translations(data, t_data_offset, t_count,
                                        bool(flags & COMP_TRANSLATION),
                                        f"bone {bone_name} translation")
        o_values = _decode_orientations(data, o_data_offset, o_count,
                                        bool(flags & COMP_ORIENTATION),
                                        f"bone {bone_name} rotation")

        tracks.append({
            "bone": bone_name,
            "scale": [{"time": time, "value": value} for time, value in zip(s_times, s_values)],
            "translation": [{"time": time, "value": value} for time, value in zip(t_times, t_values)],
            "rotation": [{"time": time, "value_xyzw": value} for time, value in zip(o_times, o_values)],
        })

    # Match the game's loader's conservative structural checks: key arrays are
    # independent and may not overlap one another or the fixed bone table.
    array_ranges.sort(key=lambda item: (item[0], item[1]))
    for previous, current in zip(array_ranges, array_ranges[1:]):
        if previous[1] > current[0]:
            raise AnimationFormatError(
                f"overlapping arrays: {previous[2]} [{previous[0]}, {previous[1]}) and "
                f"{current[2]} [{current[0]}, {current[1]})"
            )

    if header_name:
        animation_name = header_name
    elif resource_name:
        # Treat either path separator as a separator so this helper also works
        # when a Windows resource path is passed from a non-Windows host.
        animation_name = os.fspath(resource_name).replace("\\", "/").rsplit("/", 1)[-1]
        if animation_name.lower().endswith(".mtx"):
            animation_name = animation_name[:-4]
    else:
        animation_name = "unnamed"

    if flags & TIME_8BIT:
        time_encoding = "uint8_normalized_1_over_128"
    elif flags & TIME_16BIT:
        time_encoding = "uint16_normalized_1_over_32768"
    else:
        time_encoding = "float32_normalized"

    return {
        "format": "metal_arms_fanim",
        "version": 1,
        "name": animation_name,
        "flags": flags,
        "duration_seconds": duration,
        "time_encoding": time_encoding,
        "bone_count": bone_count,
        "tracks": tracks,
    }


def read_animation(path: Union[str, os.PathLike[str]]) -> Dict[str, Any]:
    """Read and parse a raw ``.mtx`` file from disk."""
    source = Path(path)
    return parse_animation(source.read_bytes(), source.name)


def write_json(source_path: Union[str, os.PathLike[str]],
               output_path: Union[str, os.PathLike[str]]) -> Dict[str, Any]:
    """Convert a raw ``.mtx`` file to indented UTF-8 JSON and return its data."""
    animation = read_animation(source_path)
    target = Path(output_path)
    target.parent.mkdir(parents=True, exist_ok=True)
    with target.open("w", encoding="utf-8", newline="\n") as stream:
        json.dump(animation, stream, ensure_ascii=False, indent=2, allow_nan=False)
        stream.write("\n")
    return animation


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", help="raw big-endian .mtx file")
    parser.add_argument("output", nargs="?", help="output JSON path (default: source basename .json)")
    args = parser.parse_args()
    source = Path(args.source)
    output = Path(args.output) if args.output else source.with_suffix(".json")
    parsed = write_json(source, output)
    print(f"Wrote {output} ({parsed['bone_count']} bones, {parsed['duration_seconds']:.3f}s)")

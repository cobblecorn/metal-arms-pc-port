"""Inspect serialized Fang GameCube game-data tables (not text CSV).

Offsets are relative to the start of the game-data header. Extract an MST entry
with mst_list.py first. Output is JSON and contains proprietary asset values;
keep generated reports under the ignored build/ directory.
"""
import argparse
import json
import math
import struct
from pathlib import Path


def decode(data):
    def span(offset, length):
        if offset < 0 or length < 0 or offset > len(data) or length > len(data) - offset:
            raise ValueError(f"Range outside file: offset={offset}, length={length}")
        return data[offset:offset + length]

    def unpack(fmt, offset):
        return struct.unpack(fmt, span(offset, struct.calcsize(fmt)))

    def string(offset, length, wide=False):
        width = 2 if wide else 1
        raw = span(offset, (length + 1) * width)
        if raw[-width:] != bytes(width):
            raise ValueError(f"Unterminated string at {offset}")
        return raw[:-width].decode("utf-16-be" if wide else "cp1252")

    size, count, tables_offset, flags = unpack(">IIII", 0)
    if size != len(data) or not 0 < count <= 65535:
        raise ValueError("Invalid game-data header size or table count")
    if flags & 1:
        raise ValueError("Expected serialized offsets, not fixed-up pointers")
    span(tables_offset, count * 16)
    tables = []
    for i in range(count):
        key_offset, key_length, fields_count, index, fields_offset = unpack(">IIHHI", tables_offset + i * 16)
        if index != i:
            raise ValueError(f"Table index {index} does not match position {i}")
        key = string(key_offset, key_length)
        span(fields_offset, fields_count * 12)
        fields = []
        for j in range(fields_count):
            pos = fields_offset + j * 12
            kind, value_offset, length = unpack(">III", pos)
            if kind == 1:
                value, = unpack(">f", pos + 4)
                if not math.isfinite(value):
                    raise ValueError(f"Nonfinite float in {key}, field {j}")
                type_name = "float"
            elif kind in (0, 2):
                value = string(value_offset, length, kind == 2)
                type_name = "wstring" if kind == 2 else "string"
            else:
                raise ValueError(f"Unknown field type {kind} in {key}, field {j}")
            fields.append({"index": j, "type": type_name, "value": value})
        tables.append({"name": key, "index": index, "fields": fields})
    return {"byte_order": "big", "size": size, "tables": tables}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("path", type=Path)
    parser.add_argument("--table", help="Only emit the named table (case insensitive)")
    parser.add_argument("--output", type=Path, help="Write JSON here instead of stdout")
    args = parser.parse_args()
    try:
        result = decode(args.path.read_bytes())
        if args.table:
            result["tables"] = [t for t in result["tables"] if t["name"].casefold() == args.table.casefold()]
            if not result["tables"]:
                raise ValueError(f"Table not found: {args.table}")
        output = json.dumps(result, indent=2, ensure_ascii=False, allow_nan=False) + "\n"
        if args.output:
            args.output.write_text(output, encoding="utf-8")
        else:
            print(output, end="")
    except (ValueError, OSError) as error:
        parser.exit(1, f"{args.path}: {error}\n")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""
hbin_pack.py — .hbin unified binary format packer / reader.

Packs a kernel image (ELF) into the .hbin unified binary format:

    [ 96-byte header ] [ 12-byte segment descriptor * N ] [ payload ]

Header fields (see docs/os/bin-format.md for the full spec):
    magic 'HBIN', version, header_size, entry offset, load address,
    segment count, build timestamp (unix seconds, UTC), sha256 of the
    payload, payload size.

Segment descriptor (one per segment): type / offset / size (u32 LE each).
    type 1 = KERNEL_ELF — the raw kernel image bytes.

The payload is the byte region wrapped by the segment table; the header
sha256 is computed over exactly those bytes, so a reader can verify
integrity without trusting any stored value.

CLI:
    python3 tools/hbin_pack.py <elf> -o out.hbin [--entry 0x100000] [--load 0x100000]
    python3 tools/hbin_pack.py -o out.hbin --info     # reader mode

When --entry / --load are omitted they are auto-detected from the ELF
(e_entry and the lowest PT_LOAD virtual address); on a 32-bit ELF that
yields load 0x100000 / entry 0x100010 for the bare-metal Harmony kernel.
"""

import argparse
import hashlib
import struct
import sys
import time

# --- format constants (single source of truth for the .hbin v1 layout) ---
MAGIC = b"HBIN"
VERSION = 1
HEADER_SIZE = 96          # bytes of the fixed header
SEG_DESC_SIZE = 12        # bytes of one segment descriptor (type/offset/size)
SEG_KERNEL_ELF = 1        # segment type: raw kernel image (ELF) bytes

DEFAULT_LOAD = 0x100000   # conventional bare-metal load address (1 MiB)

# header field offsets (little-endian)
OFF_MAGIC = 0
OFF_VERSION = 4
OFF_HEADER_SIZE = 8
OFF_ENTRY_OFFSET = 12
OFF_LOAD_ADDRESS = 16
OFF_SEGMENT_COUNT = 20
OFF_BUILD_TIME = 24
OFF_PAYLOAD_SHA256 = 32
OFF_PAYLOAD_SIZE = 64
# 24 bytes reserved (offset 72..96) — zero in v1


def _u32(data, off):
    return struct.unpack_from("<I", data, off)[0]


def _u64(data, off):
    return struct.unpack_from("<Q", data, off)[0]


def _pack_u32(v):
    return struct.pack("<I", v)


def _pack_u64(v):
    return struct.pack("<Q", v)


def elf_entry(elf):
    """Return the ELF32/ELF64 entry point (e_entry)."""
    if len(elf) < 0x18:
        raise ValueError("not an ELF: too small")
    if elf[:4] != b"\x7fELF":
        raise ValueError("not an ELF: bad magic")
    if elf[4] == 1:                       # ELFCLASS32
        return _u32(elf, 0x18)
    if elf[4] == 2:                       # ELFCLASS64
        return struct.unpack_from("<Q", elf, 0x18)[0]
    raise ValueError("not an ELF: unknown class %d" % elf[4])


def elf_load_base(elf):
    """Lowest PT_LOAD virtual address; None when the ELF has no PT_LOAD."""
    if len(elf) < 0x34 or elf[:4] != b"\x7fELF":
        raise ValueError("not an ELF")
    is64 = elf[4] == 2
    phoff = struct.unpack_from("<Q" if is64 else "<I", elf, 0x20 if is64 else 0x1C)[0]
    phentsz = struct.unpack_from("<H", elf, 0x36 if is64 else 0x2A)[0]
    phnum = struct.unpack_from("<H", elf, 0x38 if is64 else 0x2C)[0]
    if phentsz < (56 if is64 else 32):
        raise ValueError("bad ELF: program header entry too small")
    loads = []
    for i in range(phnum):
        base = phoff + i * phentsz
        p_type = _u32(elf, base)              # 1 = PT_LOAD
        p_vaddr = struct.unpack_from("<Q" if is64 else "<I", elf, base + (0x10 if is64 else 8))[0]
        if p_type == 1:
            loads.append(p_vaddr)
    return min(loads) if loads else None


def _build_hbin(elf, entry, load, timestamp):
    """Build .hbin bytes from a kernel ELF + resolved load/entry addresses."""
    payload = bytes(elf)
    seg_table = struct.pack("<III", SEG_KERNEL_ELF, 0, len(payload))
    entry_offset = entry - load
    if entry_offset < 0:
        raise ValueError("entry 0x%x is below load address 0x%x" % (entry, load))
    header = bytearray(HEADER_SIZE)
    header[OFF_MAGIC:OFF_MAGIC + 4] = MAGIC
    struct.pack_into("<I", header, OFF_VERSION, VERSION)
    struct.pack_into("<I", header, OFF_HEADER_SIZE, HEADER_SIZE)
    struct.pack_into("<I", header, OFF_ENTRY_OFFSET, entry_offset)
    struct.pack_into("<I", header, OFF_LOAD_ADDRESS, load)
    struct.pack_into("<I", header, OFF_SEGMENT_COUNT, 1)
    struct.pack_into("<Q", header, OFF_BUILD_TIME, timestamp)
    header[OFF_PAYLOAD_SHA256:OFF_PAYLOAD_SHA256 + 32] = hashlib.sha256(payload).digest()
    struct.pack_into("<Q", header, OFF_PAYLOAD_SIZE, len(payload))
    # reserved bytes 72..96 stay zero (v1)
    return bytes(header) + seg_table + payload


def parse_hbin(data):
    """Parse .hbin bytes into an info dict. Raises ValueError on corruption."""
    if len(data) < HEADER_SIZE:
        raise ValueError("file too small (%d bytes < %d-byte header)" % (len(data), HEADER_SIZE))
    if data[OFF_MAGIC:OFF_MAGIC + 4] != MAGIC:
        raise ValueError("bad magic: %r (expected 'HBIN')" % data[:4])
    version = _u32(data, OFF_VERSION)
    header_size = _u32(data, OFF_HEADER_SIZE)
    if header_size < HEADER_SIZE:
        raise ValueError("header_size %d < fixed header %d" % (header_size, HEADER_SIZE))
    if len(data) < header_size:
        raise ValueError("file too small for header_size %d" % header_size)
    entry_offset = _u32(data, OFF_ENTRY_OFFSET)
    load = _u32(data, OFF_LOAD_ADDRESS)
    seg_count = _u32(data, OFF_SEGMENT_COUNT)
    build_time = _u64(data, OFF_BUILD_TIME)
    payload_sha256 = bytes(data[OFF_PAYLOAD_SHA256:OFF_PAYLOAD_SHA256 + 32])
    payload_size = _u64(data, OFF_PAYLOAD_SIZE)

    table_off = header_size
    table_bytes = seg_count * SEG_DESC_SIZE
    payload_off = table_off + table_bytes
    if len(data) < payload_off + payload_size:
        raise ValueError("file too small: payload %d bytes at %d but file is %d"
                         % (payload_size, payload_off, len(data)))
    if len(data) != payload_off + payload_size:
        raise ValueError("trailing bytes: file is %d, header+table+payload = %d"
                         % (len(data), payload_off + payload_size))

    segments = []
    for i in range(seg_count):
        base = table_off + i * SEG_DESC_SIZE
        stype = _u32(data, base)
        soff = _u32(data, base + 4)
        ssize = _u32(data, base + 8)
        if soff + ssize > payload_size:
            raise ValueError("segment %d out of bounds: offset %d + size %d > payload %d"
                             % (i, soff, ssize, payload_size))
        segments.append({"type": stype, "offset": soff, "size": ssize})

    payload = data[payload_off:payload_off + payload_size]
    actual_sha = hashlib.sha256(payload).hexdigest()
    return {
        "version": version,
        "header_size": header_size,
        "entry_offset": entry_offset,
        "load_address": load,
        "entry_address": load + entry_offset,
        "segment_count": seg_count,
        "build_time": build_time,
        "payload_sha256": payload_sha256.hex(),
        "payload_sha256_ok": payload_sha256.hex() == actual_sha,
        "actual_sha256": actual_sha,
        "payload_size": payload_size,
        "file_size": len(data),
        "segments": segments,
    }


def format_info(info, name="<hbin>"):
    """Human-readable rendering of a parsed .hbin (reader mode output)."""
    lines = [
        "hbin: %s" % name,
        "  magic             : HBIN",
        "  version           : %d" % info["version"],
        "  header_size       : %d bytes" % info["header_size"],
        "  entry_offset      : 0x%x" % info["entry_offset"],
        "  load_address      : 0x%x" % info["load_address"],
        "  entry_address     : 0x%x" % info["entry_address"],
        "  segment_count     : %d" % info["segment_count"],
        "  build_time        : %d (%s)" % (info["build_time"],
                                           time.strftime("%Y-%m-%d %H:%M:%S UTC", time.gmtime(info["build_time"]))),
        "  payload_size      : %d bytes" % info["payload_size"],
        "  file_size         : %d bytes" % info["file_size"],
        "  payload_sha256    : %s" % info["payload_sha256"],
        "  sha256 verify     : %s" % ("OK" if info["payload_sha256_ok"] else "MISMATCH"),
    ]
    for i, seg in enumerate(info["segments"]):
        lines.append("  segment[%d]        : type=%d offset=0x%x size=%d"
                     % (i, seg["type"], seg["offset"], seg["size"]))
    return "\n".join(lines)


def main(argv=None):
    ap = argparse.ArgumentParser(
        description="Pack a kernel ELF into the .hbin unified binary format, "
                    "or print the parsed header of an existing .hbin (--info).")
    ap.add_argument("elf", nargs="?", help="kernel image (ELF) to pack")
    ap.add_argument("-o", "--output", required=True, metavar="out.hbin",
                    help="output .hbin file (also read in --info mode)")
    ap.add_argument("--entry", type=lambda v: int(v, 0), default=None, metavar="0x100000",
                    help="entry point address (default: auto-detect from ELF, e.g. 0x100010)")
    ap.add_argument("--load", type=lambda v: int(v, 0), default=None, metavar="0x100000",
                    help="load address (default: lowest PT_LOAD vaddr, e.g. 0x100000)")
    ap.add_argument("--info", action="store_true",
                    help="reader mode: parse --output and print header fields + sha256")
    args = ap.parse_args(argv)

    if not args.elf and not args.info:
        ap.error("an <elf> input is required unless --info is used")

    if args.elf:
        with open(args.elf, "rb") as f:
            elf = f.read()

        load = args.load if args.load is not None else (elf_load_base(elf) or DEFAULT_LOAD)
        entry = args.entry if args.entry is not None else elf_entry(elf)
        timestamp = int(time.time())
        hbin = _build_hbin(elf, entry, load, timestamp)
        with open(args.output, "wb") as f:
            f.write(hbin)
        info = parse_hbin(hbin)
        print(format_info(info, args.output))
        print("wrote %s (%d bytes)" % (args.output, len(hbin)))
        return 0 if info["payload_sha256_ok"] else 1

    # reader mode: no ELF given, only --info -> parse the existing file
    with open(args.output, "rb") as f:
        data = f.read()
    try:
        info = parse_hbin(data)
    except ValueError as e:
        print("ERROR: %s: %s" % (args.output, e), file=sys.stderr)
        return 1
    print(format_info(info, args.output))
    return 0 if info["payload_sha256_ok"] else 1


if __name__ == "__main__":
    sys.exit(main())

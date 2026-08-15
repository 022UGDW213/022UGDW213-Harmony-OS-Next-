#!/usr/bin/env python3
"""
hbin_check.py — verifier for the .hbin unified binary format.

Checks (see docs/os/bin-format.md):
    1. magic           — file starts with the 4-byte 'HBIN' magic
    2. version         — format version is supported (1)
    3. header_size     — sane (>= fixed 96-byte header) and fits in the file
    4. layout          — file is exactly header + segment table + payload
    5. segment bounds  — every segment descriptor lies inside the payload
    6. sha256          — recomputed payload digest matches the header value

Exits 0 when every check passes, 1 otherwise. Reuses the single parser in
hbin_pack.py so the packer, reader and verifier can never drift apart.

Usage:
    python3 tools/hbin_check.py <file.hbin>
"""

import argparse
import os
import sys

try:
    from hbin_pack import HEADER_SIZE, MAGIC, SEG_DESC_SIZE, VERSION, parse_hbin
except ImportError:  # pragma: no cover - fallback when run from another cwd
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    from hbin_pack import HEADER_SIZE, MAGIC, SEG_DESC_SIZE, VERSION, parse_hbin


def main(argv=None):
    ap = argparse.ArgumentParser(
        description="Verify a .hbin unified binary file (magic/version/layout/"
                    "segment bounds/sha256).")
    ap.add_argument("hbin", metavar="file.hbin", help=".hbin file to verify")
    args = ap.parse_args(argv)

    with open(args.hbin, "rb") as f:
        data = f.read()

    checks = []
    checks.append(("magic 'HBIN'", data[:4] == MAGIC,
                   "%r" % data[:4] if data[:4] != MAGIC else MAGIC.decode()))

    ok = True
    try:
        info = parse_hbin(data)
        version_ok = info["version"] == VERSION
        header_ok = HEADER_SIZE <= info["header_size"] <= len(data)
        layout_ok = info["file_size"] == info["header_size"] \
            + info["segment_count"] * SEG_DESC_SIZE + info["payload_size"]
        # parse_hbin already validated segment bounds and payload size
        bounds_ok = all(seg["offset"] + seg["size"] <= info["payload_size"]
                        for seg in info["segments"])
        sha_ok = info["payload_sha256_ok"]
        checks += [
            ("version %d supported" % VERSION, version_ok, "got %d" % info["version"]),
            ("header_size sane", header_ok, "%d bytes" % info["header_size"]),
            ("layout exact (header+segments+payload)", layout_ok,
             "file=%d expected=%d" % (info["file_size"], info["header_size"]
                                      + info["segment_count"] * SEG_DESC_SIZE
                                      + info["payload_size"])),
            ("segment bounds", bounds_ok, "%d segments" % info["segment_count"]),
            ("payload sha256", sha_ok, "stored=%s computed=%s"
             % (info["payload_sha256"], info["actual_sha256"])),
        ]
    except ValueError as e:
        checks.append(("parse", False, str(e)))

    ok = all(passed for _, passed, _ in checks)
    print("hbin_check: %s" % args.hbin)
    for name, passed, detail in checks:
        print("  [%s] %-40s %s" % ("PASS" if passed else "FAIL", name, detail))
    if not ok:
        print("RESULT: FAIL")
        return 1
    print("RESULT: OK — %d checks passed" % len(checks))
    return 0


if __name__ == "__main__":
    sys.exit(main())

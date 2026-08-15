# .hbin — Unified Binary Format (v1)

The `.hbin` format is the unified binary packaging for the Harmony OS Next
bare-metal kernel. A single `.hbin` file wraps a kernel image together with
machine-readable load metadata (entry/load addresses, segment table, build
timestamp) and a SHA-256 integrity digest, so a bootloader/loader can load and
verify the kernel without re-parsing the embedded ELF.

Reference implementation (single source of truth):

| Tool | File | Role |
|---|---|---|
| Packer / reader | `tools/hbin_pack.py` | writes `.hbin`, prints parsed header (`--info`) |
| Verifier | `tools/hbin_check.py` | validates magic/version/layout/segments/sha256 |
| Build pipeline | `scripts/build-hbin.sh` | builds the kernel, packs it, prints the header |

## 1. File layout

```
┌──────────────────────────────┐ offset 0
│  Header  (96 bytes, fixed)  │
├──────────────────────────────┤ offset 96
│  Segment descriptors         │ 12 bytes × segment_count
├──────────────────────────────┤ offset 96 + 12 × segment_count
│  Payload                     │ payload_size bytes (the wrapped image data)
└──────────────────────────────┘
```

All integers are little-endian. All addresses are 32-bit (this kernel is a
32-bit i386 ELF; the format is intentionally 32-bit for v1).

## 2. Header (96 bytes, fixed in v1)

| Offset | Size | Field | Description |
|---|---|---|---|
| 0 | 4 | `magic` | ASCII `HBIN` |
| 4 | 4 | `version` | Format version — `1` (u32) |
| 8 | 4 | `header_size` | Total header size in bytes — `96` (u32) |
| 12 | 4 | `entry_offset` | Entry offset relative to the payload base (u32) |
| 16 | 4 | `load_address` | Address where the payload is loaded (u32) |
| 20 | 4 | `segment_count` | Number of segment descriptors (u32) |
| 24 | 8 | `build_time` | Build timestamp, unix seconds UTC (u64) |
| 32 | 32 | `payload_sha256` | SHA-256 of the payload bytes |
| 64 | 8 | `payload_size` | Payload size in bytes (u64) |
| 72 | 24 | `reserved` | Zero in v1 |

Effective entry address: `entry_address = load_address + entry_offset`.

### 2.1 Example (packed Harmony kernel)

```
magic            : HBIN
version          : 1
header_size      : 96 bytes
entry_offset     : 0x10
load_address     : 0x100000
entry_address    : 0x100010
segment_count    : 1
build_time       : <unix seconds, UTC>
payload_size     : 16200 bytes
payload_sha256   : 3746762bb1817874ce61291fafdb38c06420948e496d31232a814f3cbc5b1eef
```

## 3. Segment descriptors (12 bytes each)

| Offset | Size | Field | Description |
|---|---|---|---|
| 0 | 4 | `type` | Segment type (u32), see registry below |
| 4 | 4 | `offset` | Byte offset of the segment data from the payload base (u32) |
| 8 | 4 | `size` | Segment data size in bytes (u32) |

A segment must satisfy `offset + size <= payload_size`.

### 3.1 Segment type registry (v1)

| Type | Name | Meaning |
|---|---|---|
| 1 | `KERNEL_ELF` | Raw kernel image bytes — the ELF produced by `server/native/build_baremetal_local.sh` |

Future types (flat binary, multiboot image, initrd, …) reserve type numbers
≥ 2; the format supports multiple segments per file, each referencing a
disjoint region of the single payload.

## 4. Payload and digest

The payload is the byte region spanned by the segment table; with one
`KERNEL_ELF` segment it is exactly the kernel ELF file. `payload_sha256` in
the header is `SHA-256(payload)`; a reader must recompute it and compare, and
must not trust a stored digest alone. This makes the digest cover every
wrapped segment.

## 5. Verification rules (`tools/hbin_check.py`)

1. **magic** — file starts with `HBIN`.
2. **version** — version equals the supported `1`.
3. **header_size** — ≥ 96 and fits within the file.
4. **layout** — file size is exactly `header_size + segment_count × 12 + payload_size` (no trailing bytes).
5. **segment bounds** — every segment satisfies `offset + size <= payload_size`.
6. **sha256** — recomputed payload digest equals `payload_sha256`.

The check exits `0` only when all six pass.

## 6. CLI usage

Pack a kernel ELF (entry/load auto-detected from the ELF header; explicit
values override):

```sh
python3 tools/hbin_pack.py server/native/harmony_kernel -o build/harmony-kernel.hbin
python3 tools/hbin_pack.py <elf> -o out.hbin --entry 0x100000 --load 0x100000
```

Reader mode — print the parsed header fields and the sha256 of an existing
file (no re-pack):

```sh
python3 tools/hbin_pack.py -o build/harmony-kernel.hbin --info
```

Verify a file:

```sh
python3 tools/hbin_check.py build/harmony-kernel.hbin
```

One-shot pipeline (build kernel → pack → print header):

```sh
bash scripts/build-hbin.sh
```

## 7. Design notes

- The entry is stored as an *offset* so the loader can compute the real entry
  address once the load address is known (e.g. after KASLR-style relocation).
- The header is fixed-size with a `header_size` field, so a future v2 can
  append fields while old readers still parse `header_size` bytes.
- No compression in v1: the payload is the image verbatim; the digest is
  computed on the uncompressed bytes so verification is independent of any
  future compression stage.

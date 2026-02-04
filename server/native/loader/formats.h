#ifndef HARMONY_LOADER_FORMATS_H
#define HARMONY_LOADER_FORMATS_H

#include <stdint.h>
#include <stddef.h>

// Binary Types
typedef enum {
    BINARY_TYPE_UNKNOWN = 0,
    BINARY_TYPE_PE,
    BINARY_TYPE_ELF,
    BINARY_TYPE_MACHO
} binary_type_t;

// --- ELF (Linux) ---
typedef struct {
    unsigned char e_ident[16]; // Magic: 0x7f 'E' 'L' 'F'
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} elf64_header_t;

// --- PE (Windows) ---
typedef struct {
    uint16_t e_magic;    // Magic: 'M' 'Z'
    // ... DOS stub ...
    uint32_t e_lfanew;   // Offset to NT Header
} dos_header_t;

typedef struct {
    uint32_t Signature;  // 'P' 'E' \0 \0
    // ... File Header ...
    // ... Optional Header (contains EntryPoint) ...
    // Simplified for detection
} pe_nt_headers_t;

// --- Mach-O (macOS) ---
typedef struct {
    uint32_t magic;      // Magic: 0xfeedfacf (64-bit)
    uint32_t cputype;
    uint32_t cpusubtype;
    uint32_t filetype;
    uint32_t ncmds;
    uint32_t sizeofcmds;
    uint32_t flags;
    uint32_t reserved;
} mach_header_64_t;

// Generic Executable Image (In-Memory Representation)
typedef struct {
    binary_type_t type;
    uint64_t entry_point;
    void* raw_data;
    size_t size;
} executable_image_t;

#endif // HARMONY_LOADER_FORMATS_H

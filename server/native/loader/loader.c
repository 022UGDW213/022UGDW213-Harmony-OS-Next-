#include "loader/formats.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Helper to read 32-bit int from buffer (Little Endian)
static uint32_t read_u32(const uint8_t* buf) {
    return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

// Detection Logic
binary_type_t loader_detect_format(const uint8_t* buffer, size_t size) {
    if (size < 4) return BINARY_TYPE_UNKNOWN;

    // 1. Check ELF (Linux)
    // Magic: 0x7f 'E' 'L' 'F'
    if (buffer[0] == 0x7f && buffer[1] == 'E' && buffer[2] == 'L' && buffer[3] == 'F') {
        return BINARY_TYPE_ELF;
    }

    // 2. Check PE (Windows)
    // Magic: 'M' 'Z' (DOS Header)
    if (buffer[0] == 'M' && buffer[1] == 'Z') {
        return BINARY_TYPE_PE;
    }

    // 3. Check Mach-O (macOS)
    // Magic: 0xfeedfacf (LE) or 0xcffaedfe (BE)
    // We'll check standard Little Endian 64-bit
    uint32_t magic = read_u32(buffer);
    if (magic == 0xfeedfacf) {
        return BINARY_TYPE_MACHO;
    }

    return BINARY_TYPE_UNKNOWN;
}

// Loading Logic (Stub - Parses Header)
// In a real OS, this would map segments to memory
int loader_parse_header(const uint8_t* buffer, size_t size, executable_image_t* out_img) {
    binary_type_t type = loader_detect_format(buffer, size);
    if (type == BINARY_TYPE_UNKNOWN) return -1;
    
    out_img->type = type;
    out_img->raw_data = (void*)buffer; // unsafe reference, but fine for sim
    out_img->size = size;

    if (type == BINARY_TYPE_ELF) {
        // Read Entry point from ELF64 header
        if (size < sizeof(elf64_header_t)) return -2;
        const elf64_header_t* elf = (const elf64_header_t*)buffer;
        out_img->entry_point = elf->e_entry;
        printf("   [Loader] Detected ELF. Entry: 0x%llx\n", (unsigned long long)elf->e_entry);
    } 
    else if (type == BINARY_TYPE_PE) {
        // PE Entry Point is deeper in NT Optional Header. 
        // For detection we just confirm type.
        out_img->entry_point = 0; // Placeholder
        printf("   [Loader] Detected PE (Windows).\n");
    }
    else if (type == BINARY_TYPE_MACHO) {
        // Mach-O Entry Point is in LC_MAIN or LC_UNIXTHREAD command
        // For detection we just confirm type.
        out_img->entry_point = 0; // Placeholder
        printf("   [Loader] Detected Mach-O (macOS).\n");
    }

    return 0;
}

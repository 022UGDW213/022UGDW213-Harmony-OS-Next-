#include "loader/loader.h"
#include <stdio.h>
#include <string.h>

void test_loader() {
    printf("🔄 Universal Binary Loader Test\n");
    printf("===============================\n");

    // 1. Test ELF Detection
    uint8_t elf_mock[64];
    memset(elf_mock, 0, 64);
    elf_mock[0] = 0x7f; elf_mock[1] = 'E'; elf_mock[2] = 'L'; elf_mock[3] = 'F'; // Magic
    // e_entry is offset 24 (64-bit)
    uint64_t entry = 0x400080;
    memcpy(&elf_mock[24], &entry, 8);

    executable_image_t img;
    if (loader_parse_header(elf_mock, 64, &img) == 0) {
        if (img.type == BINARY_TYPE_ELF && img.entry_point == entry) {
            printf("✅ Detected ELF (Entry: 0x%llx)\n", (unsigned long long)img.entry_point);
        } else {
            printf("❌ Failed ELF check\n");
        }
    } else {
        printf("❌ Failed to parse ELF\n");
    }

    // 2. Test PE Detection
    uint8_t pe_mock[64];
    memset(pe_mock, 0, 64);
    pe_mock[0] = 'M'; pe_mock[1] = 'Z'; // Magic
    
    if (loader_parse_header(pe_mock, 64, &img) == 0) {
        if (img.type == BINARY_TYPE_PE) {
            printf("✅ Detected PE (Windows)\n");
        } else {
            printf("❌ Failed PE check\n");
        }
    }

    // 3. Test Mach-O Detection
    uint8_t macho_mock[64];
    memset(macho_mock, 0, 64);
    // Magic 0xfeedfacf (LE) -> cf fa ed fe
    macho_mock[0] = 0xcf; macho_mock[1] = 0xfa; macho_mock[2] = 0xed; macho_mock[3] = 0xfe;

    if (loader_parse_header(macho_mock, 64, &img) == 0) {
        if (img.type == BINARY_TYPE_MACHO) {
            printf("✅ Detected Mach-O (macOS)\n");
        } else {
            printf("❌ Failed Mach-O check (Type=%d)\n", img.type);
        }
    }

    printf("\n✅ Loader tests completed.\n");
}

int main() {
    test_loader();
    return 0;
}

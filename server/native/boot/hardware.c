#include "boot/hardware.h"
#include <stdio.h>
#include <string.h>

static system_info_t sys_info;

void hardware_detect(void) {
    printf("🔍 Detecting Hardware...\n");
    
    // Simulate CPUID extraction
    strcpy(sys_info.cpu_vendor, "HarmonyCPU");
    sys_info.cpu_family = 6;
    sys_info.cpu_model = 142;
    sys_info.has_fpu = 1;
    sys_info.has_apic = 1;
    
    // Simulate Memory Map E820
    sys_info.total_memory_mb = 128; // 128 MB physical RAM simulated
    sys_info.reserved_memory_mb = 16; // Kernel + Reserved
    sys_info.available_memory_mb = sys_info.total_memory_mb - sys_info.reserved_memory_mb;
    
    printf("   CPU: %s Family %d Model %d\n", sys_info.cpu_vendor, sys_info.cpu_family, sys_info.cpu_model);
    printf("   Memory: %u MB Total (%u MB Available)\n", sys_info.total_memory_mb, sys_info.available_memory_mb);
}

const system_info_t* hardware_get_info(void) {
    return &sys_info;
}

void hardware_print_info(void) {
    printf("\n💻 System Information:\n");
    printf("   Vendor:   %s\n", sys_info.cpu_vendor);
    printf("   Memory:   %u MB\n", sys_info.total_memory_mb);
    printf("   Features: %s %s\n", 
           sys_info.has_fpu ? "FPU" : "",
           sys_info.has_apic ? "APIC" : "");
}

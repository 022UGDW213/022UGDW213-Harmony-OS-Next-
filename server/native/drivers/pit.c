// drivers/pit.c - 8253/8254 Programmable Interval Timer, real port I/O.
// Channel 0, mode 2 (rate generator), 100 Hz by default. The IRQ0
// handler (pit_tick) increments a tick counter and a seconds counter.
#include "pit.h"
#include "../kernel/idt.h"
#include "../drivers/pic.h"

#define PIT_CH0_DATA 0x40
#define PIT_CMD      0x43
#define PIT_FREQ     1193182   /* 1.193182 MHz base clock */

static volatile uint64_t pit_ticks = 0;
static volatile uint32_t pit_tick_count = 0;  // wraps at hz (no 64-bit div)
static volatile uint32_t pit_seconds = 0;
static uint32_t pit_hz = 100;

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

void pit_init(uint32_t hz) {
    if (hz == 0) {
        hz = 100;
    }
    pit_hz = hz;

    uint32_t divisor = PIT_FREQ / hz;   // 32-bit division, inlined by gcc
    if (divisor > 0xFFFF) {
        divisor = 0xFFFF;
    }

    // Command: channel 0, lobyte/hibyte access, mode 2 (rate generator), binary
    outb(PIT_CMD, 0x36);
    outb(PIT_CH0_DATA, (uint8_t)(divisor & 0xFF));        // divisor low
    outb(PIT_CH0_DATA, (uint8_t)((divisor >> 8) & 0xFF)); // divisor high
}

uint64_t pit_get_ticks(void) {
    return pit_ticks;
}

uint32_t pit_get_seconds(void) {
    return pit_seconds;
}

// IRQ0 hook. Holds the interrupt disabled during the call; the EOI is
// sent by the generic isr_handler dispatch after this returns.
void pit_tick(registers_t* r) {
    (void)r;
    pit_ticks++;
    // Update whole seconds at exactly hz ticks (100 Hz -> 100 ticks/s)
    pit_tick_count++;
    if (pit_tick_count >= pit_hz) {
        pit_tick_count = 0;
        pit_seconds++;
    }
}

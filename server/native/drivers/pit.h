#ifndef HARMONY_PIT_H
#define HARMONY_PIT_H

#include <stdint.h>
#include "../kernel/idt.h"   /* registers_t for the IRQ hook signature */

// Program PIT channel 0 in mode 2 (rate generator) at the given Hz
// (real I/O to 0x43/0x40). Default 100 Hz for this kernel.
void pit_init(uint32_t hz);

// Tick counter incremented by the IRQ0 handler. With a 100 Hz PIT,
// each tick is 10 ms.
uint64_t pit_get_ticks(void);

// Whole seconds since boot (updated by the tick handler).
uint32_t pit_get_seconds(void);

// Tick hook installed as IRQ0 handler.
void pit_tick(registers_t* r);

#endif // HARMONY_PIT_H

// kernel/shell.c - interactive shell over PS/2 keyboard + COM1 serial RX.
#include "shell.h"
#include "../drivers/keyboard.h"
#include "../drivers/pit.h"
#include "../drivers/serial.h"
#include "../drivers/vga.h"
#include "kprintf.h"
#include "meminfo.h"
#include "string.h"
#include <stdint.h>

#define CMD_LINE_MAX 128

static char cmd_buf[CMD_LINE_MAX];
static int cmd_len = 0;

// Write one char to both outputs (serial first, then VGA).
static void shell_echo(char c) {
    serial_putc(c);
    vga_putc(c);
}

static void shell_prompt(void) {
    shell_echo('\r');
    shell_echo('\n');
    kprintf("harmony> ");
}

// Feed one input char into the line editor; returns 1 when Enter is hit.
static int shell_input(char c) {
    if (c == '\r' || c == '\n') {
        if (cmd_len > 0) {
            shell_echo('\n');
            return 1;
        }
        return 0;
    }
    if (c == '\b') {
        if (cmd_len > 0) {
            cmd_len--;
            shell_echo('\b');
        }
        return 0;
    }
    if (c >= ' ' && c < 0x7F && cmd_len < CMD_LINE_MAX - 1) {
        cmd_buf[cmd_len++] = c;
        shell_echo(c);
    }
    return 0;
}

static void cmd_help(void) {
    kprintf("HarmonyOS Next bare-metal commands:\n");
    kprintf("  help      - this list\n");
    kprintf("  uptime    - seconds since boot (PIT tick counter)\n");
    kprintf("  meminfo   - real detected RAM (Multiboot info)\n");
    kprintf("  echo <t>  - print the given text\n");
    kprintf("  clear     - clear the VGA screen\n");
    kprintf("  reboot    - reset via PS/2 controller (0x64 -> 0xFE)\n");
}

static void cmd_uptime(void) {
    uint32_t secs = pit_get_seconds();
    uint32_t ticks_lo = (uint32_t)pit_get_ticks();
    kprintf("Uptime: %u seconds (%u ms) - PIT ticks=%u @ 100 Hz\n",
            secs, ticks_lo * 10, ticks_lo);
}

static void cmd_meminfo(void) {
    kprintf("RAM total (multiboot mem_lower+mem_upper): %u KiB (%u MiB)\n",
            meminfo_total_kb(), meminfo_total_kb() >> 10);
    kprintf("RAM available (mmap): %u MiB across %u entries\n",
            meminfo_available_mb(), meminfo_mmap_entries());
}

static void cmd_reboot(void) {
    kprintf("Rebooting via PS/2 controller...\n");
    // 8042: pulse the reset line (port 0x64, command 0xFE)
    __asm__ volatile ( "outb %0, %1" : : "a"((uint8_t)0xFE), "Nd"((uint16_t)0x64) );
    while (1) { __asm__ volatile ("hlt"); }
}

// Tokenize: split at first space; returns pointer to rest (or NULL).
static const char* cmd_split(const char* line, char* word) {
    const char* p = line;
    while (*p && *p != ' ') {
        *word++ = *p++;
    }
    *word = '\0';
    if (*p == ' ') {
        while (*p == ' ') p++;
        return *p ? p : NULL;
    }
    return NULL;
}

static void shell_dispatch(char* line) {
    char word[CMD_LINE_MAX];
    const char* rest = cmd_split(line, word);

    if (strcmp(word, "help") == 0) {
        cmd_help();
    } else if (strcmp(word, "uptime") == 0) {
        cmd_uptime();
    } else if (strcmp(word, "meminfo") == 0) {
        cmd_meminfo();
    } else if (strcmp(word, "echo") == 0) {
        kprintf("%s\n", rest ? rest : "");
    } else if (strcmp(word, "clear") == 0) {
        vga_clear();
    } else if (strcmp(word, "reboot") == 0) {
        cmd_reboot();
    } else if (word[0] == '\0') {
        // empty line: just a new prompt
    } else {
        kprintf("Unknown command: %s (try 'help')\n", word);
    }
}

void shell_run(void) {
    cmd_len = 0;
    shell_prompt();

    while (1) {
        char c;
        // COM1 serial RX (line status 0x3FD bit 0)
        if (serial_getc(&c) && shell_input(c)) {
            cmd_buf[cmd_len] = '\0';
            shell_dispatch(cmd_buf);
            cmd_len = 0;
            shell_prompt();
        }
        // PS/2 keyboard (polled, non-blocking)
        if (keyboard_get_char_nb(&c) && shell_input(c)) {
            cmd_buf[cmd_len] = '\0';
            shell_dispatch(cmd_buf);
            cmd_len = 0;
            shell_prompt();
        }
    }
}

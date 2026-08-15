#ifndef HARMONY_SHELL_H
#define HARMONY_SHELL_H

// Interactive command shell. Reads REAL input from both the PS/2 keyboard
// (polled 0x60/0x64) and COM1 serial RX (0x3FD bit 0), echoes to VGA and
// serial, and dispatches commands. Never returns.
void shell_run(void);

#endif // HARMONY_SHELL_H

[BITS 64]
[ORG 0x100000]
[global _start]
[extern kmain]

section .multiboot_header
align 8
header_start:
    dd 0xe85250d6
    dd 0
    dd header_end - header_start
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))
header_end:

section .text
_start:
    cli
    mov rsp, stack_top
    call kmain
    hlt

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

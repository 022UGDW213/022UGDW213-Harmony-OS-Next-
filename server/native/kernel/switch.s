/* kernel/switch.s - REAL IRQ0-driven context switch.
 *
 * Called from scheduler_tick (C) with the target task's saved frame pointer
 * as the first argument (see kernel/isr.s for the frame layout: a
 * registers_t built by isr_common_stub). We switch stacks to that frame and
 * run exactly the epilogue isr_common_stub would have run, so `iret`
 * resumes the target task at its saved eip/cs/eflags — as if it had just
 * been interrupted. This function never returns.
 */
.section .text
.global context_switch
# void context_switch(uint32_t new_esp)
context_switch:
    movl 4(%esp), %eax      /* new_esp = first argument (frame base) */
    movl %eax, %esp
    popa                    /* edi, esi, ebp, [esp], ebx, edx, ecx, eax */
    popl %ds
    popl %es
    popl %fs
    popl %gs
    addl $8, %esp           /* skip int_no + err_code */
    iret                    /* eip, cs, eflags from the saved frame */

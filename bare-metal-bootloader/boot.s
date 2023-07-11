.globl _start
_start:
    mov sp,#0x08000000
    bl main

.space 0x200000-0x8004,0

halt: b halt

.globl _start
_start:
    mov sp,#0x8000
    bl main

halt: b halt

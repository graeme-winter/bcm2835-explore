
MEMORY
{
    RAM : ORIGIN = 0x8000, LENGTH = 128M
}

SECTIONS
{
    .text : { *(.text*) } > RAM
    .data : { *(.data) } > RAM
    .bss : { *(.bss*) } > RAM
}

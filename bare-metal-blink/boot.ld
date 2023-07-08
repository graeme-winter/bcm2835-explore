
MEMORY
{
    RAM : ORIGIN = 0x8000, LENGTH = 0x10000
}

SECTIONS
{
    .text : { *(.text*) } > RAM
    .bss : { *(.bss*) } > RAM
}

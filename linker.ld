ENTRY(boot_start)

SECTIONS
{
    . = 0x100000;
    
    .multiboot2 BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot2)
    }
    
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.text)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }
    
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }
    
    /DISCARD/ : {
        *(.note*)
        *(.iplt*)
        *(.igot*)
        *(.rel*)
        *(.comment)
    }
}
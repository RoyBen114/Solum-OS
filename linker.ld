ENTRY(_start)

SECTIONS
{
    . = 1M;
    
    /* 确保 multiboot header 在最前面 */
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
}
section .multiboot2
header_start:
    ; Multiboot2 头
    dd 0xe85250d6                ; 魔数
    dd 0                         ; i386
    dd header_end - header_start 
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; checksum

    ; 结束标签
    align 8
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
header_end:

section .text
global _start
extern kernel_main
_start:
    ; 调用内核主函数
    call kernel_main
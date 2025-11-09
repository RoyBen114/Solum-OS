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
    mov ebx, MSG1
    call print_string_pm
    call kernel_main
    hlt

MSG1 db "[INFO] Swithed into protected mode", 0

print_string_pm:
    pusha
    mov edx, 0xB8000

print_string_pm_loop:
    mov al, [ebx] ; [ebx]为字符的地址
    mov ah, 0x0F

    cmp al, 0 ; 查看是否是字符串末尾
    je print_string_pm_done

    mov [edx], ax ; 将字符+颜色属性写入到视频存储区域
    add ebx, 1 
    add edx, 2 

    jmp print_string_pm_loop

print_string_pm_done:
    popa
    ret
section .multiboot2
header_start:
    dd 0xe85250d6               
    dd 0                        
    dd header_end - header_start 
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) 

    ; 结束标签
    align 8
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size

header_end:

section .text
global boot_start
extern kernel_main
boot_start:
    mov ebx, MSG0
    mov ecx, 0
    mov edx, 0
    call print
    mov esp, stack_top
    mov ebx, MSG1
    mov ecx, 0
    mov edx, 1
    call print
    call kernel_main
    hlt

MSG0 db "[INFO] Switched into protected mode", 0
MSG1 db "[INFO] Updated stack pointer register", 0

print:  ;EBX=字符串, ECX=X, EDX=Y
    pusha

    ; 计算目标内存地址: edi = 0xB8000 + y*160 + x*2
    mov edi, edx
    imul edi, 160         
    lea edi, [edi + ecx*2] 
    add edi, 0xB8000     
    mov ah, 0x0F          

.print_loop:
    mov al, [ebx]         
    cmp al, 0             
    je .done
    mov [edi], ax         
    add edi, 2            
    inc ebx               
    jmp .print_loop

.done:
    popa
    ret

section .bss
align 4096
stack_bottom:
    resb 64
stack_top:
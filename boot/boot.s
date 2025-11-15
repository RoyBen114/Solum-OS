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

section .bss
align 4096
; 页表结构 - 放在BSS段，避免重定位问题
pml4_table:
    resb 4096
pdpt_table:
    resb 4096
pd_table:
    resb 4096

stack_bottom:
    resb 8192
stack_top:

section .data
align 8
; GDT for 64-bit mode
gdt64:
    dq 0                    ; 空描述符
    dq 0x0020980000000000   ; 代码段描述符
    dq 0x0020920000000000   ; 数据段描述符
gdt64_len: equ $ - gdt64

gdtr64:
    dw gdt64_len - 1
    dq gdt64

section .text
bits 32
global boot_start

boot_start:
    ; 保存multiboot信息到非易失寄存器
    mov esi, ebx

    ;设置栈指针
    mov esp, stack_top
    
    ; 设置基础页表
    call setup_paging
    
    ; 启用PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; 设置PML4 - 使用绝对地址
    mov eax, pml4_table
    mov cr3, eax

    ; 启用长模式
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; 启用分页
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ; 加载GDT并跳转到64位代码
    lgdt [gdtr64]
    jmp 0x08:long_mode_start

setup_paging:
    pusha
    
    ; 清零页表
    mov edi, pml4_table
    mov ecx, 4096 * 3
    xor eax, eax
    rep stosb

    ; 设置PML4条目指向PDPT
    mov eax, pdpt_table
    or eax, 0x03  ; Present + Writeable
    mov dword [pml4_table + 0], eax

    ; 设置PML4的最后一个条目指向自己（用于递归映射）
    mov eax, pml4_table
    or eax, 0x03
    mov dword [pml4_table + 511 * 8], eax

    ; 设置PDPT条目指向PD
    mov eax, pd_table
    or eax, 0x03  ; Present + Writeable
    mov dword [pdpt_table + 0], eax

    ; 设置PD条目（2MB大页）- 映射前1GB物理内存
    mov eax, 0x00000083  ; Present + Writeable + 2MB Page + Global
    mov ecx, 512
    mov edi, pd_table
    
.set_pd_entries:
    mov dword [edi], eax
    mov dword [edi + 4], 0
    add eax, 0x200000  ; 下一个2MB页
    add edi, 8
    loop .set_pd_entries
    
    popa
    ret

section .text
bits 64
long_mode_start:
    ; 设置段寄存器
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; 设置栈
    mov rsp, stack_top

    ; 恢复multiboot信息
    mov rdi, rsi

    ; 调用内核主函数
    extern kernel_main
    call kernel_main

    hlt
    jmp $
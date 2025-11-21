section .multiboot2
header_start:
    ; Multiboot2 头
    dd 0xe85250d6                ; Magic number
    dd 0                         ; Architecture (0 = i386, 4 = MIPS)
    dd header_end - header_start ; Header length
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; Checksum

    ; 架构标签 - 指定为x86_64
    align 8
    dw 0    ; type: architecture
    dw 0    ; flags
    dd 8    ; size
    dd 4    ; architecture: x86_64

    ; 结束标签
    align 8
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size

header_end:

section .bss
align 4096

; 页表
pml4_table:
    resb 4096
pdpt_table:
    resb 4096
pd_table:
    resb 4096 * 4

; 栈
stack_bottom:
    resb 16384
stack_top:

section .data
align 8

; GDT
gdt64:
    dq 0                    ; 空描述符
    dq 0x0020980000000000   ; 代码段描述符
    dq 0x0020920000000000   ; 数据段描述符
gdt64_len: equ $ - gdt64

gdtr64:
    dw gdt64_len - 1
    dq gdt64

; 引导信息
boot_info:
    .memory_map_ptr: dq 0
    .memory_map_entries: dd 0
    .framebuffer_addr: dq 0
    .framebuffer_width: dd 0
    .framebuffer_height: dd 0
    .framebuffer_pitch: dd 0
    .framebuffer_bpp: dd 0

section .text
bits 32
global boot_start

boot_start:
    ; 保存 multiboot2 信息
    mov esi, ebx
    mov edi, eax

    ; 设置栈
    mov esp, stack_top
    
    jz .goto_long_mode

.goto_long_mode:
    ; 设置页表
    call setup_paging
    
    ; 启用 PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; 设置 PML4
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

    ; 加载 GDT 并跳转 kernel_entry
    lgdt [gdtr64]
    jmp 0x08:kernel_entry

; 设置分页
setup_paging:
    pusha
    
    ; 清零页表
    mov edi, pml4_table
    mov ecx, 4096 * 6
    xor eax, eax
    rep stosb

    ; 设置 PML4 条目指向 PDPT
    mov eax, pdpt_table
    or eax, 0x03  ; Present + Writeable
    mov dword [pml4_table + 0], eax
    
    ; 设置更高半内核映射
    mov dword [pml4_table + 256 * 8], eax
    mov dword [pml4_table + 511 * 8], eax

    ; 设置 PDPT 条目指向 PD
    mov eax, pd_table
    or eax, 0x03
    mov dword [pdpt_table + 0], eax
    mov dword [pdpt_table + 1 * 8], eax
    mov dword [pdpt_table + 2 * 8], eax
    mov dword [pdpt_table + 3 * 8], eax

    ; 设置 PD 条目 - 映射前 4GB 内存
    mov eax, 0x00000083  ; Present + Writeable + 2MB Page
    mov ecx, 2048        ; 映射 4GB (2048 * 2MB)
    mov edi, pd_table
    
.set_pd_entries:
    mov dword [edi], eax
    mov dword [edi + 4], 0
    add eax, 0x200000
    add edi, 8
    loop .set_pd_entries

    mov eax, 0xB8000
    shr eax, 21          ; 获取页目录索引
    shl eax, 3           ; 乘以 8（每个条目8字节）
    add eax, pd_table
    
    mov dword [eax], 0x00000000 | 0x83
    popa
    ret

bits 64

kernel_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    mov rsp, stack_top
    
    ; 调用内核主函数
    extern kernel_main
    mov rdi, boot_info
    call kernel_main
    
    hlt
    jmp $
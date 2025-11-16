section .multiboot2
header_start:
    ; Multiboot2 头
    dd 0xe85250d6                ; Magic number
    dd 0                         ; Architecture (0 = i386, 4 = MIPS)
    dd header_end - header_start ; Header length
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; Checksum

    ; 架构标签 - 指定为x86_64
    align 8
    dw 6    ; type: EFI boot services
    dw 0    ; flags
    dd 12   ; size
    dd 2    ; architecture: x86_64

    ; 结束标签
    align 8
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size

header_end:

section .bss
align 4096

; 页表 - 在 BIOS 模式下使用
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
    .boot_type: dd 0        ; 0 = BIOS, 1 = UEFI
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

    ; 检测引导环境
    call detect_boot_environment

    ; 如果是 UEFI 且已经在长模式，跳转到 UEFI 处理
    cmp dword [boot_info.boot_type], 1
    jne .bios_path
    
    ; 检查是否已经在长模式
    mov eax, cr0
    test eax, 0x80000000    ; 检查分页位
    jz .bios_path
    
    mov ecx, 0xC0000080
    rdmsr
    test eax, 1 << 8        ; 检查长模式位
    jz .bios_path
    
    ; UEFI 已经在长模式
    lgdt [gdtr64]
    jmp 0x08:uefi_long_mode_entry

.bios_path:
    ; BIOS 路径或 UEFI 需要设置长模式
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

    ; 加载 GDT 并跳转到长模式
    lgdt [gdtr64]
    jmp 0x08:bios_long_mode_entry

; 检测引导环境
detect_boot_environment:
    push eax
    push ebx
    push ecx
    
    ; 默认假设 BIOS
    mov dword [boot_info.boot_type], 0
    
    ; 检查 multiboot2 magic
    cmp edi, 0x36d76289
    jne .done
    
    ; 解析 multiboot2 信息，查找 EFI 相关标签
    mov eax, esi        ; multiboot2 信息地址
    add eax, 8          ; 跳过 total_size 和 reserved
    
.parse_tags:
    mov ebx, [eax]      ; 标签类型
    mov ecx, [eax + 4]  ; 标签大小
    
    ; 检查结束标签
    cmp ebx, 0
    je .done
    
    ; 检查 EFI 系统表标签 (类型 12)
    cmp ebx, 12
    jne .next_tag
    
    ; 找到 EFI 系统表标签，说明是 UEFI 引导
    mov dword [boot_info.boot_type], 1
    jmp .done
    
.next_tag:
    ; 跳到下一个标签
    add eax, ecx
    ; 对齐到 8 字节
    add eax, 7
    and eax, ~7
    jmp .parse_tags

.done:
    pop ecx
    pop ebx
    pop eax
    ret

; 设置分页 (BIOS 模式使用)
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
    
    ; 特别映射 VGA 内存区域 (0xB8000)
    ; 计算 VGA 区域所在的 2MB 页
    mov eax, 0xB8000
    shr eax, 21          ; 获取页目录索引
    shl eax, 3           ; 乘以 8（每个条目8字节）
    add eax, pd_table
    
    ; 设置 VGA 区域为可写，使用 4KB 页
    mov dword [eax], 0x00000000 | 0x83  ; 2MB 页包含 VGA
    popa
    ret

section .text
bits 64

; UEFI 长模式入口
uefi_long_mode_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    mov rsp, stack_top
    
    ; 解析 multiboot2 信息获取帧缓冲区等
    mov rdi, rsi
    call parse_multiboot2_info
    
    ; 调用内核主函数
    extern kernel_main
    mov rdi, boot_info
    call kernel_main
    
    hlt
    jmp $

; BIOS 长模式入口
bios_long_mode_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rsp, stack_top

    ; 解析 multiboot2 信息
    mov rdi, rsi
    call parse_multiboot2_info

    ; 调用内核主函数
    extern kernel_main
    mov rdi, boot_info
    call kernel_main

    hlt
    jmp $

; 解析 multiboot2 信息
parse_multiboot2_info:
    push rbx
    push rcx
    push rdx
    push rsi
    
    ; rdi 包含 multiboot2 信息地址
    mov rbx, rdi
    
    ; 检查 multiboot2 magic
    cmp dword [rbx], 0x36d76289
    jne .done
    
    ; 获取总大小并跳过前8字节
    mov ecx, [rbx]      ; total_size
    add rbx, 8          ; 跳过 total_size 和 reserved
    
.parse_tags:
    ; 检查是否超出信息结构
    mov eax, [rbx]      ; 标签类型
    mov edx, [rbx + 4]  ; 标签大小
    
    ; 检查结束标签
    cmp eax, 0
    je .done
    cmp edx, 0
    je .done
    cmp edx, 8
    jb .done
    
    ; 基本内存信息 (类型 4)
    cmp eax, 4
    jne .check_mmap
    
    ; 处理基本内存信息
    ; 这里可以保存内存信息
    jmp .next_tag

.check_mmap:
    ; 内存映射 (类型 6)
    cmp eax, 6
    jne .check_framebuffer
    
    ; 保存内存映射信息
    mov [boot_info.memory_map_ptr], rbx
    mov eax, [rbx + 16] ; 条目大小
    mov ecx, [rbx + 20] ; 条目数量
    mov [boot_info.memory_map_entries], ecx
    jmp .next_tag

.check_framebuffer:
    ; 帧缓冲区信息 (类型 8)
    cmp eax, 8
    jne .next_tag
    
    ; 保存帧缓冲区信息
    mov eax, [rbx + 8]  ; 地址低32位
    mov edx, [rbx + 12] ; 地址高32位
    shl rdx, 32
    or rax, rdx
    mov [boot_info.framebuffer_addr], rax
    
    mov eax, [rbx + 16] ; 宽度
    mov [boot_info.framebuffer_width], eax
    
    mov eax, [rbx + 20] ; 高度
    mov [boot_info.framebuffer_height], eax
    
    mov eax, [rbx + 24] ; 每行字节数
    mov [boot_info.framebuffer_pitch], eax
    
    mov eax, [rbx + 28] ; 每像素位数
    mov [boot_info + 32], eax  ; 在 boot_info 中添加 bpp 字段

.next_tag:
    ; 跳到下一个标签
    add rbx, rdx        ; 使用完整的标签大小
    ; 对齐到 8 字节
    add rbx, 7
    and rbx, ~7
    
    ; 检查是否超出 multiboot2 信息结构
    mov rax, rdi
    add rax, [rdi]      ; 起始地址 + total_size
    cmp rbx, rax
    jae .done
    
    jmp .parse_tags

.done:
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    ret
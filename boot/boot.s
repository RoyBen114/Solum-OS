;
; Copyright (C) 2025 Roy Roy123ty@hotmail.com
; 
; This file is part of Solum OS
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
; 
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
; 
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>.
;

section .multiboot2
header_start:
    dd 0xe85250d6                
    dd 0                        
    dd header_end - header_start 
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) 

    ; architecture - x86_64
    align 8
    dw 0    
    dw 0    
    dd 8    
    dd 4   

    ; frame buffer
    align 8
    dw 8
    dw 1
    dd 20
    dd 1024
    dd 768
    dd 32

    ; end
    align 8
    dw 0   
    dw 0   
    dd 8  

header_end:

section .bss
align 4096

; paging table
pml4_table:
    resb 4096
pdpt_table:
    resb 4096
pd_table:
    resb 4096 * 4

; stack
stack_bottom:
    resb 65536
stack_top:
    
section .data
align 8

; GDT
gdt64:
    dq 0                   
    dq 0x0020980000000000  
    dq 0x0020920000000000   
gdt64_len: equ $ - gdt64

gdtr64:
    dw gdt64_len - 1
    dq gdt64

global multiboot2_info_addr
multiboot2_info_addr: dq 0

section .text
bits 32
global boot_start

boot_start:
    ; store multiboot2 info
    mov dword [multiboot2_info_addr], ebx
    mov dword [multiboot2_info_addr + 4], 0

    mov esp, stack_top
    
    jmp .goto_long_mode

.goto_long_mode:

    call setup_paging
    
    ; enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; setup PML4
    mov eax, pml4_table
    mov cr3, eax

    ; enable long mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ; load GDT and jmp kernel_entry
    lgdt [gdtr64]
    jmp 0x08:kernel_entry

; setup paging
setup_paging:
    push edi
    push esi
    
    ; clear page table
    mov edi, pml4_table
    mov ecx, (4096 * 6) / 4
    xor eax, eax
    rep stosd

    ; setup PML4
    ; entry 0: map low 4GB
    mov eax, pdpt_table
    or eax, 0x03  ; Present + Writeable
    mov [pml4_table], eax
    
    ; entry 256: high half kernel mapping (0xFFFF800000000000)
    mov [pml4_table + 256 * 8], eax
    
    ; entry 511: high half kernel mapping (0xFFFFFFFF80000000)  
    mov [pml4_table + 511 * 8], eax

    ; setup PDPT entry - map 4 1GB area
    mov eax, pd_table
    or eax, 0x03
    mov [pdpt_table + 0 * 8], eax
    mov [pdpt_table + 1 * 8], eax
    mov [pdpt_table + 2 * 8], eax
    mov [pdpt_table + 3 * 8], eax

    ; setup PD entry - map front 4GB memory to 2MB page
    mov eax, 0x00000083  ; Present + Writeable + 2MB Page
    mov ecx, 2048        ; 2048 * 2MB = 4GB
    mov edi, pd_table
    
.set_pd_entries:
    mov [edi], eax
    mov dword [edi + 4], 0
    add eax, 0x200000 
    add edi, 8
    loop .set_pd_entries

    pop esi
    pop edi
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

    extern kernel_main
    mov rdi, [multiboot2_info_addr]
    call kernel_main
    
    hlt
    jmp $
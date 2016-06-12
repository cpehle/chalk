global start
extern long_mode_start

section .text
bits 32

start:
    mov esp, stack_top

    call set_up_page_tables
    call enable_paging
    call enable_sse

    lgdt [gdt64.pointer]

    mov ax, gdt64.data
    mov ss, ax
    mov ds, ax
    mov es, ax

    mov dword [0xb8000], 0x2f4b2f4f
    jmp gdt64.code:long_mode_start

enable_sse:
    mov eax, cr0
    and ax, 0xfffb
    or ax, 0x2
    mov cr0, eax
    mov eax, cr4
    or ax, 3 << 9
    mov cr4, eax
    ret

set_up_page_tables:
    mov eax, p3_table
    or eax, p3_table
    or eax, 0b11
    mov [p4_table], eax
    mov eax, p2_table
    or eax, 0b11
    mov [p3_table], eax
    mov ecx, 0
.map_p2_table:
    mov eax, 0x200000
    mul ecx
    or eax, 0b10000011
    mov [p2_table + ecx * 8], eax

    inc ecx
    cmp ecx, 512
    jne .map_p2_table

    ret

enable_paging:
    ;; load p4 to cr3 register (this is how the cpu accesses the table)
    mov eax, p4_table
    mov cr3, eax
    ;; Enable physical address extension
    mov eax, cr4
    or eax, 1<<5
    mov cr4, eax
    ;; Set long mode bit
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr
    ;; Enable paging in cr0 register
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret

error:
    mov dword [0xb8000], 0x4f524f45
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov byte  [0xb800a], al
    hlt



section .bss
align 4096
p4_table:
    resb 4096
p3_table:
    resb 4096
p2_table:
    resb 4096
stack_bottom:
    resb 4096
stack_top:

section .rodata
gdt64:
    dq 0                        ; zero entry
.code: equ $ - gdt64
    dq (1<<44) | (1<<47) | (1<<41) | (1<<43) | (1<<53) ; code segment
.data: equ $ - gdt64
    dq (1<<44) | (1<<47) | (1<<41)                     ; data segment
.pointer:
    dw $ - gdt64 - 1
    dq gdt64

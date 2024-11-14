bits 64

%define write   1
%define open    2
%define close   3
%define exit    60

%define flags   0x41

section .text

global _start
    _start:
        endbr64         ; Virtualy ends branching
        push rbp        ; Saving current stack pointer to stack
        mov rbp, rsp    ; Setting base pointer to current stack

        ; open(path, flags)
        mov rax, open   ; Select write syscall
        mov rdi, path   ; Submitting filepath
        mov rsi, flags  ; Write and create
        mov rdx, 0o666  ; -rw-rw-r--
        syscall

        mov rdi, rax
        cmp rax, -1     ; Error-check
        jle endprog

        push rdi

        ; write(rdi, string, len)
        mov rax, write  ; Select write syscall
        mov rsi, string ; Output data
        mov rdx, len    ; Length of output data
        syscall

        pop rdi

        ; close(rdi)
        mov rax, close
        syscall

        xor rdi, rdi

    endprog:
        pop rbp         ; Resetting stack

        ; exit(rdi)
        mov rax, exit
        syscall


section .rdata
    path    db  "./test.txt",0
    string  db  "Test string",0xa
    len     equ $-string

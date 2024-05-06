.intel_syntax noprefix
.globl main
main:
        push rbp
        mov rbp, rsp
        sub rsp, 800
	call act
	push rax
	push 0
	call expc
  pop rcx
	push rax
        pop rdi
        pop rax
        cmp rax, rdi
        sete al
        movzb rax, al
        push rax
	pop rax
	cmp rax, 0
	je .Lelse0
	push 0
        pop rax
	mov rsp, rbp
	pop rbp
	ret
        jmp .Lend0
.Lelse0:
	push 1
        pop rax
	mov rsp, rbp
	pop rbp
	ret
.Lend0:
        pop rax
	push 0
        pop rax
	mov rsp, rbp
	pop rbp
	ret

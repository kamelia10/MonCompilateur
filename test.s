			# This code was produced by the CERI Compiler
	.data
	.align 8
a:	.quad 0
b:	.quad 0
c:	.quad 0
z:	.quad 0
	.text
	.globl main
main:
	movq %rsp, %rbp
	push $8
	push $3
	pop %rbx
	pop %rax
	cmpq %rbx, %rax
	je True1
	push $0
	jmp EndCompare1
True1:
	push $-1
EndCompare1:
	push $4
	push $2
	push $2
	pop %rbx
	pop %rax
	mulq %rbx
	push %rax
	pop %rbx
	pop %rax
	cmpq %rbx, %rax
	je True2
	push $0
	jmp EndCompare2
True2:
	push $-1
EndCompare2:
	pop %rbx
	pop %rax
	orq %rbx, %rax
	push %rax
	pop z(%rip)
	push $5
	push $65
	pop %rbx
	pop %rax
	movq $0, %rdx
	divq %rbx
	push %rax
	push $2
	pop %rbx
	pop %rax
	addq %rbx, %rax
	push %rax
	push $7
	push $5
	pop %rbx
	pop %rax
	movq $0, %rdx
	divq %rbx
	push %rdx
	pop %rbx
	pop %rax
	cmpq %rbx, %rax
	jb True3
	push $0
	jmp EndCompare3
True3:
	push $-1
EndCompare3:
	pop b(%rip)
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function

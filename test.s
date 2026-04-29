			# This code was produced by the CERI Compiler
	.data
	.align 8
a:	.quad 0
b:	.quad 0
c:	.quad 0
	.text
	.globl main
main:
	movq %rsp, %rbp
	push $0
	pop a(%rip)
	push $3
	pop b(%rip)
WhileBegin1:
	push a(%rip)
	push b(%rip)
	pop %rbx
	pop %rax
	cmpq %rbx, %rax
	jb BoolTrue2
	push $0
	jmp BoolEnd2
BoolTrue2:
	push $-1
BoolEnd2:
	pop %rax
	cmpq $0, %rax
	je WhileEnd1
	push a(%rip)
	push $1
	pop %rbx
	pop %rax
	addq %rbx, %rax
	push %rax
	pop a(%rip)
	push a(%rip)
	push $2
	pop %rbx
	pop %rax
	mulq %rbx
	push %rax
	pop c(%rip)
	jmp WhileBegin1
WhileEnd1:
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function

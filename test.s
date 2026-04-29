			# This code was produced by the CERI Compiler
	.data
	.align 8
a:	.quad 0
b:	.quad 0
	.text
	.globl main
main:
	movq %rsp, %rbp
	push $5
	pop a(%rip)
	push a(%rip)
	push $3
	pop %rbx
	pop %rax
	cmpq %rbx, %rax
	ja BoolTrue2
	push $0
	jmp BoolEnd2
BoolTrue2:
	push $-1
BoolEnd2:
	pop %rax
	cmpq $0, %rax
	je ElsePart1
	push $1
	pop b(%rip)
ElsePart1:
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function

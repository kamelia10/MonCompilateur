			# This code was produced by the CERI Compiler
	.data
	.align 8
i:	.quad 0
s:	.quad 0
	.text
	.globl main
main:
	movq %rsp, %rbp
	push $1
	pop i(%rip)
	push $0
	pop s(%rip)
	push $1
	pop i(%rip)
ForBegin1:
	push $5
	pop %rbx
	movq i(%rip), %rax
	cmpq %rbx, %rax
	ja ForEnd1
	push s(%rip)
	push i(%rip)
	pop %rbx
	pop %rax
	addq %rbx, %rax
	push %rax
	pop s(%rip)
	addq $1, i(%rip)
	jmp ForBegin1
ForEnd1:
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function

			# This code was produced by the CERI Compiler
	.data
	.align 8
FormatString1:	.string "%llu\n"
a:	.quad 0	# BOOLEAN
b:	.quad 0	# BOOLEAN
c:	.quad 0	# BOOLEAN
d:	.quad 0	# INTEGER
e:	.quad 0	# INTEGER
	.text
	.globl main
	.extern printf
main:
	movq %rsp, %rbp
	push $5
	push $6
	pop %rbx
	pop %rax
	cmpq %rbx, %rax
	jb BoolTrue1
	push $0
	jmp BoolEnd1
BoolTrue1:
	push $-1
BoolEnd1:
	pop a(%rip)
	push a(%rip)
	pop b(%rip)
	push $5
	pop d(%rip)
	push $10
	pop e(%rip)
	push d(%rip)
	push e(%rip)
	pop %rbx
	pop %rax
	addq %rbx, %rax
	push %rax
	pop %rdx	# The value to be displayed
	leaq FormatString1(%rip), %rcx	# "%llu\n"
	subq $40, %rsp	# shadow space + align stack for Windows printf
	call printf
	addq $40, %rsp	# restore stack
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function

			# This code was produced by the CERI Compiler
	.data
	.align 8
FormatString1:	.string "%llu\n"
a:	.quad 0
	.text
	.globl main
	.extern printf
main:
	movq %rsp, %rbp
	push $5
	push $6
	pop %rbx
	pop %rax
	addq %rbx, %rax
	push %rax
	pop a(%rip)
	push a(%rip)
	push $1
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

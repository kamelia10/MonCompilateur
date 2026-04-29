			# This code was produced by the CERI Compiler
	.data
	.align 8
FormatInteger:	.string "%llu\n"
FormatDouble:	.string "%f\n"
FormatChar:	.string "%c\n"
a:	.quad 0	# INTEGER
x:	.double 0.0	# DOUBLE
c:	.byte 0	# CHAR
b:	.quad 0	# BOOLEAN
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
	popq a(%rip)
	movabsq $4609434218613702656, %rax	# double 1.5
	pushq %rax
	movabsq $4612248968380809216, %rax	# double 2.25
	pushq %rax
	fldl 8(%rsp)
	fldl (%rsp)
	addq $16, %rsp
	faddp %st, %st(1)
	subq $8, %rsp
	fstpl (%rsp)
	popq x(%rip)
	push $65	# char 'A'
	pop %rax
	movb %al, c(%rip)
	pushq a(%rip)
	push $3
	pop %rbx
	pop %rax
	cmpq %rbx, %rax
	ja BoolTrue1
	push $0
	jmp BoolEnd1
BoolTrue1:
	push $-1
BoolEnd1:
	popq b(%rip)
	pushq a(%rip)
	pop %rdx
	leaq FormatInteger(%rip), %rcx
	subq $40, %rsp
	call printf
	addq $40, %rsp
	pushq x(%rip)
	pop %rdx
	movq %rdx, %xmm1
	leaq FormatDouble(%rip), %rcx
	subq $40, %rsp
	call printf
	addq $40, %rsp
	movzbq c(%rip), %rax
	push %rax
	pop %rdx
	leaq FormatChar(%rip), %rcx
	subq $40, %rsp
	call printf
	addq $40, %rsp
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function

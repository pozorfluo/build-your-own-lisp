	.file	"lisp.c"
	.section	.rodata
.LC0:
	.string	"list"
.LC1:
	.string	"head"
.LC2:
	.string	"tail"
.LC3:
	.string	"join"
.LC4:
	.string	"cons"
.LC5:
	.string	"len"
.LC6:
	.string	"init"
.LC7:
	.string	"eval"
.LC8:
	.string	"+"
.LC9:
	.string	"-"
.LC10:
	.string	"*"
.LC11:
	.string	"/"
.LC12:
	.string	"%"
.LC13:
	.string	"^"
.LC14:
	.string	">"
.LC15:
	.string	"<"
.LC16:
	.string	"{"
.LC17:
	.string	"}"
.LC18:
	.string	"("
.LC19:
	.string	")"
	.section	.data.rel.local,"aw",@progbits
	.align 32
	.type	vocabulary, @object
	.size	vocabulary, 168
vocabulary:
	.quad	.LC0
	.quad	.LC1
	.quad	.LC2
	.quad	.LC3
	.quad	.LC4
	.quad	.LC5
	.quad	.LC6
	.quad	.LC7
	.quad	.LC8
	.quad	.LC9
	.quad	.LC10
	.quad	.LC11
	.quad	.LC12
	.quad	.LC13
	.quad	.LC14
	.quad	.LC15
	.quad	.LC16
	.quad	.LC17
	.quad	.LC18
	.quad	.LC19
	.quad	0
	.text
	.globl	new_lispvalue_number
	.type	new_lispvalue_number, @function
new_lispvalue_number:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movsd	%xmm0, -24(%rbp)
	movl	$56, %edi
	call	malloc@PLT
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movl	$1, (%rax)
	movq	-8(%rbp), %rax
	movsd	-24(%rbp), %xmm0
	movsd	%xmm0, 8(%rax)
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	new_lispvalue_number, .-new_lispvalue_number
	.globl	new_lispvalue_error
	.type	new_lispvalue_error, @function
new_lispvalue_error:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movl	$56, %edi
	call	malloc@PLT
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movl	$0, (%rax)
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	strlen@PLT
	addq	$1, %rax
	movq	%rax, %rdi
	call	malloc@PLT
	movq	%rax, %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 16(%rax)
	movq	-8(%rbp), %rax
	movq	16(%rax), %rax
	movq	-24(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	strcpy@PLT
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	new_lispvalue_error, .-new_lispvalue_error
	.globl	new_lispvalue_symbol
	.type	new_lispvalue_symbol, @function
new_lispvalue_symbol:
.LFB4:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movl	$56, %edi
	call	malloc@PLT
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movl	$2, (%rax)
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	strlen@PLT
	addq	$1, %rax
	movq	%rax, %rdi
	call	malloc@PLT
	movq	%rax, %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 24(%rax)
	movq	-8(%rbp), %rax
	movq	24(%rax), %rax
	movq	-24(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	strcpy@PLT
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	new_lispvalue_symbol, .-new_lispvalue_symbol
	.globl	new_lispvalue_function
	.type	new_lispvalue_function, @function
new_lispvalue_function:
.LFB5:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movl	$56, %edi
	call	malloc@PLT
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movl	$3, (%rax)
	movq	-8(%rbp), %rax
	movq	-24(%rbp), %rdx
	movq	%rdx, 32(%rax)
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	new_lispvalue_function, .-new_lispvalue_function
	.globl	new_lispenv
	.type	new_lispenv, @function
new_lispenv:
.LFB6:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	$24, %edi
	call	malloc@PLT
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movl	$0, (%rax)
	movq	-8(%rbp), %rax
	movq	$0, 8(%rax)
	movq	-8(%rbp), %rax
	movq	$0, 16(%rax)
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	new_lispenv, .-new_lispenv
	.globl	new_lispvalue_sexpr
	.type	new_lispvalue_sexpr, @function
new_lispvalue_sexpr:
.LFB7:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	$56, %edi
	call	malloc@PLT
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movl	$4, (%rax)
	movq	-8(%rbp), %rax
	movl	$0, 40(%rax)
	movq	-8(%rbp), %rax
	movq	$0, 48(%rax)
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	new_lispvalue_sexpr, .-new_lispvalue_sexpr
	.globl	new_lispvalue_qexpr
	.type	new_lispvalue_qexpr, @function
new_lispvalue_qexpr:
.LFB8:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	$56, %edi
	call	malloc@PLT
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movl	$5, (%rax)
	movq	-8(%rbp), %rax
	movl	$0, 40(%rax)
	movq	-8(%rbp), %rax
	movq	$0, 48(%rax)
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	new_lispvalue_qexpr, .-new_lispvalue_qexpr
	.globl	delete_lispvalue
	.type	delete_lispvalue, @function
delete_lispvalue:
.LFB9:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$5, %eax
	ja	.L26
	movl	%eax, %eax
	leaq	0(,%rax,4), %rdx
	leaq	.L18(%rip), %rax
	movl	(%rdx,%rax), %eax
	movslq	%eax, %rdx
	leaq	.L18(%rip), %rax
	addq	%rdx, %rax
	jmp	*%rax
	.section	.rodata
	.align 4
	.align 4
.L18:
	.long	.L17-.L18
	.long	.L27-.L18
	.long	.L20-.L18
	.long	.L27-.L18
	.long	.L22-.L18
	.long	.L22-.L18
	.text
.L17:
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rax, %rdi
	call	free@PLT
	jmp	.L23
.L20:
	movq	-24(%rbp), %rax
	movq	24(%rax), %rax
	movq	%rax, %rdi
	call	free@PLT
	jmp	.L23
.L22:
	movl	$0, -4(%rbp)
	jmp	.L24
.L25:
	movq	-24(%rbp), %rax
	movq	48(%rax), %rax
	movl	-4(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	addl	$1, -4(%rbp)
.L24:
	movq	-24(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	-4(%rbp), %eax
	jg	.L25
	movq	-24(%rbp), %rax
	movq	48(%rax), %rax
	movq	%rax, %rdi
	call	free@PLT
	jmp	.L23
.L26:
	nop
	jmp	.L23
.L27:
	nop
.L23:
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	free@PLT
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE9:
	.size	delete_lispvalue, .-delete_lispvalue
	.globl	delete_lispenv
	.type	delete_lispenv, @function
delete_lispenv:
.LFB10:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movl	$0, -4(%rbp)
	jmp	.L29
.L30:
	movq	-24(%rbp), %rax
	movq	8(%rax), %rax
	movl	-4(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movq	%rax, %rdi
	call	free@PLT
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	movl	-4(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	addl	$1, -4(%rbp)
.L29:
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	cmpl	-4(%rbp), %eax
	jg	.L30
	movq	-24(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, %rdi
	call	free@PLT
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rax, %rdi
	call	free@PLT
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	free@PLT
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE10:
	.size	delete_lispenv, .-delete_lispenv
	.section	.rodata
.LC20:
	.string	"unbound symbol !"
	.text
	.globl	get_lispenv
	.type	get_lispenv, @function
get_lispenv:
.LFB11:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movl	$0, -4(%rbp)
	jmp	.L32
.L35:
	movq	-32(%rbp), %rax
	movq	24(%rax), %rdx
	movq	-24(%rbp), %rax
	movq	8(%rax), %rax
	movl	-4(%rbp), %ecx
	movslq	%ecx, %rcx
	salq	$3, %rcx
	addq	%rcx, %rax
	movq	(%rax), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	strcmp@PLT
	testl	%eax, %eax
	jne	.L33
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	movl	-4(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movq	%rax, %rdi
	call	copy_lispvalue
	jmp	.L34
.L33:
	addl	$1, -4(%rbp)
.L32:
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	cmpl	-4(%rbp), %eax
	jg	.L35
	leaq	.LC20(%rip), %rdi
	call	new_lispvalue_error
.L34:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE11:
	.size	get_lispenv, .-get_lispenv
	.globl	put_lispenv
	.type	put_lispenv, @function
put_lispenv:
.LFB12:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$56, %rsp
	.cfi_offset 3, -24
	movq	%rdi, -40(%rbp)
	movq	%rsi, -48(%rbp)
	movq	%rdx, -56(%rbp)
	movl	$0, -20(%rbp)
	jmp	.L37
.L40:
	movq	-48(%rbp), %rax
	movq	24(%rax), %rdx
	movq	-40(%rbp), %rax
	movq	8(%rax), %rax
	movl	-20(%rbp), %ecx
	movslq	%ecx, %rcx
	salq	$3, %rcx
	addq	%rcx, %rax
	movq	(%rax), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	strcmp@PLT
	testl	%eax, %eax
	jne	.L38
	movq	-40(%rbp), %rax
	movq	16(%rax), %rax
	movl	-20(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	movq	-40(%rbp), %rax
	movq	16(%rax), %rax
	movl	-20(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	leaq	(%rax,%rdx), %rbx
	movq	-56(%rbp), %rax
	movq	%rax, %rdi
	call	copy_lispvalue
	movq	%rax, (%rbx)
	jmp	.L36
.L38:
	addl	$1, -20(%rbp)
.L37:
	movq	-40(%rbp), %rax
	movl	(%rax), %eax
	cmpl	-20(%rbp), %eax
	jg	.L40
	movq	-40(%rbp), %rax
	movl	(%rax), %eax
	leal	1(%rax), %edx
	movq	-40(%rbp), %rax
	movl	%edx, (%rax)
	movq	-40(%rbp), %rax
	movl	(%rax), %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	-40(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	realloc@PLT
	movq	%rax, %rdx
	movq	-40(%rbp), %rax
	movq	%rdx, 8(%rax)
	movq	-40(%rbp), %rax
	movl	(%rax), %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	-40(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	realloc@PLT
	movq	%rax, %rdx
	movq	-40(%rbp), %rax
	movq	%rdx, 16(%rax)
	movq	-40(%rbp), %rax
	movq	8(%rax), %rdx
	movq	-40(%rbp), %rax
	movl	(%rax), %eax
	cltq
	salq	$3, %rax
	subq	$8, %rax
	leaq	(%rdx,%rax), %rbx
	movq	-48(%rbp), %rax
	movq	24(%rax), %rax
	movq	%rax, %rdi
	call	strlen@PLT
	addq	$1, %rax
	movq	%rax, %rdi
	call	malloc@PLT
	movq	%rax, (%rbx)
	movq	-48(%rbp), %rax
	movq	24(%rax), %rdx
	movq	-40(%rbp), %rax
	movq	8(%rax), %rcx
	movq	-40(%rbp), %rax
	movl	(%rax), %eax
	cltq
	salq	$3, %rax
	subq	$8, %rax
	addq	%rcx, %rax
	movq	(%rax), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	strcpy@PLT
	movq	-40(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-40(%rbp), %rax
	movl	(%rax), %eax
	cltq
	salq	$3, %rax
	subq	$8, %rax
	leaq	(%rdx,%rax), %rbx
	movq	-56(%rbp), %rax
	movq	%rax, %rdi
	call	copy_lispvalue
	movq	%rax, (%rbx)
.L36:
	addq	$56, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE12:
	.size	put_lispenv, .-put_lispenv
	.section	.rodata
.LC21:
	.string	"invalid number !"
	.text
	.globl	read_lispvalue_number
	.type	read_lispvalue_number, @function
read_lispvalue_number:
.LFB13:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	call	__errno_location@PLT
	movl	$0, (%rax)
	movq	-24(%rbp), %rax
	movq	8(%rax), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	strtod@PLT
	movq	%xmm0, %rax
	movq	%rax, -8(%rbp)
	call	__errno_location@PLT
	movl	(%rax), %eax
	cmpl	$34, %eax
	je	.L42
	movq	-8(%rbp), %rax
	movq	%rax, -32(%rbp)
	movsd	-32(%rbp), %xmm0
	call	new_lispvalue_number
	jmp	.L44
.L42:
	leaq	.LC21(%rip), %rdi
	call	new_lispvalue_error
.L44:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE13:
	.size	read_lispvalue_number, .-read_lispvalue_number
	.globl	add_lispvalue
	.type	add_lispvalue, @function
add_lispvalue:
.LFB14:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-8(%rbp), %rax
	movl	40(%rax), %eax
	leal	1(%rax), %edx
	movq	-8(%rbp), %rax
	movl	%edx, 40(%rax)
	movq	-8(%rbp), %rax
	movl	40(%rax), %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	-8(%rbp), %rax
	movq	48(%rax), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	realloc@PLT
	movq	%rax, %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 48(%rax)
	movq	-8(%rbp), %rax
	movq	48(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	40(%rax), %eax
	cltq
	salq	$3, %rax
	subq	$8, %rax
	addq	%rax, %rdx
	movq	-16(%rbp), %rax
	movq	%rax, (%rdx)
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE14:
	.size	add_lispvalue, .-add_lispvalue
	.section	.rodata
.LC22:
	.string	"number"
.LC23:
	.string	"symbol"
.LC24:
	.string	"sexpr"
.LC25:
	.string	"qexpr"
.LC26:
	.string	"regex"
	.text
	.globl	read_lispvalue
	.type	read_lispvalue, @function
read_lispvalue:
.LFB15:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	(%rax), %rax
	leaq	.LC22(%rip), %rsi
	movq	%rax, %rdi
	call	strstr@PLT
	testq	%rax, %rax
	je	.L48
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	read_lispvalue_number
	jmp	.L49
.L48:
	movq	-24(%rbp), %rax
	movq	(%rax), %rax
	leaq	.LC23(%rip), %rsi
	movq	%rax, %rdi
	call	strstr@PLT
	testq	%rax, %rax
	je	.L50
	movq	-24(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, %rdi
	call	new_lispvalue_symbol
	jmp	.L49
.L50:
	movq	$0, -8(%rbp)
	movq	-24(%rbp), %rax
	movq	(%rax), %rax
	leaq	.LC14(%rip), %rsi
	movq	%rax, %rdi
	call	strcmp@PLT
	testl	%eax, %eax
	je	.L51
	movq	-24(%rbp), %rax
	movq	(%rax), %rax
	leaq	.LC24(%rip), %rsi
	movq	%rax, %rdi
	call	strstr@PLT
	testq	%rax, %rax
	je	.L52
.L51:
	call	new_lispvalue_sexpr
	movq	%rax, -8(%rbp)
	jmp	.L53
.L52:
	movq	-24(%rbp), %rax
	movq	(%rax), %rax
	leaq	.LC25(%rip), %rsi
	movq	%rax, %rdi
	call	strstr@PLT
	testq	%rax, %rax
	je	.L53
	call	new_lispvalue_qexpr
	movq	%rax, -8(%rbp)
.L53:
	movl	$0, -12(%rbp)
	jmp	.L54
.L58:
	movq	-24(%rbp), %rax
	movq	56(%rax), %rax
	movl	-12(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movq	8(%rax), %rax
	leaq	.LC18(%rip), %rsi
	movq	%rax, %rdi
	call	strcmp@PLT
	testl	%eax, %eax
	je	.L59
	movq	-24(%rbp), %rax
	movq	56(%rax), %rax
	movl	-12(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movq	8(%rax), %rax
	leaq	.LC19(%rip), %rsi
	movq	%rax, %rdi
	call	strcmp@PLT
	testl	%eax, %eax
	je	.L59
	movq	-24(%rbp), %rax
	movq	56(%rax), %rax
	movl	-12(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movq	8(%rax), %rax
	leaq	.LC16(%rip), %rsi
	movq	%rax, %rdi
	call	strcmp@PLT
	testl	%eax, %eax
	je	.L59
	movq	-24(%rbp), %rax
	movq	56(%rax), %rax
	movl	-12(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movq	8(%rax), %rax
	leaq	.LC17(%rip), %rsi
	movq	%rax, %rdi
	call	strcmp@PLT
	testl	%eax, %eax
	je	.L59
	movq	-24(%rbp), %rax
	movq	56(%rax), %rax
	movl	-12(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movq	(%rax), %rax
	leaq	.LC26(%rip), %rsi
	movq	%rax, %rdi
	call	strcmp@PLT
	testl	%eax, %eax
	je	.L59
	movq	-24(%rbp), %rax
	movq	56(%rax), %rax
	movl	-12(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movq	%rax, %rdi
	call	read_lispvalue
	movq	%rax, %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	add_lispvalue
	movq	%rax, -8(%rbp)
	jmp	.L57
.L59:
	nop
.L57:
	addl	$1, -12(%rbp)
.L54:
	movq	-24(%rbp), %rax
	movl	48(%rax), %eax
	cmpl	-12(%rbp), %eax
	jg	.L58
	movq	-8(%rbp), %rax
.L49:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE15:
	.size	read_lispvalue, .-read_lispvalue
	.globl	copy_lispvalue
	.type	copy_lispvalue, @function
copy_lispvalue:
.LFB16:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$40, %rsp
	.cfi_offset 3, -24
	movq	%rdi, -40(%rbp)
	movl	$56, %edi
	call	malloc@PLT
	movq	%rax, -32(%rbp)
	movq	-40(%rbp), %rax
	movl	(%rax), %edx
	movq	-32(%rbp), %rax
	movl	%edx, (%rax)
	movq	-40(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$5, %eax
	ja	.L72
	movl	%eax, %eax
	leaq	0(,%rax,4), %rdx
	leaq	.L63(%rip), %rax
	movl	(%rdx,%rax), %eax
	movslq	%eax, %rdx
	leaq	.L63(%rip), %rax
	addq	%rdx, %rax
	jmp	*%rax
	.section	.rodata
	.align 4
	.align 4
.L63:
	.long	.L62-.L63
	.long	.L64-.L63
	.long	.L65-.L63
	.long	.L66-.L63
	.long	.L67-.L63
	.long	.L67-.L63
	.text
.L64:
	movq	-40(%rbp), %rax
	movsd	8(%rax), %xmm0
	movq	-32(%rbp), %rax
	movsd	%xmm0, 8(%rax)
	jmp	.L68
.L66:
	movq	-40(%rbp), %rax
	movq	32(%rax), %rdx
	movq	-32(%rbp), %rax
	movq	%rdx, 32(%rax)
	jmp	.L68
.L62:
	movq	-40(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rax, %rdi
	call	strlen@PLT
	addq	$1, %rax
	movq	%rax, %rdi
	call	malloc@PLT
	movq	%rax, %rdx
	movq	-32(%rbp), %rax
	movq	%rdx, 16(%rax)
	movq	-40(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-32(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	strcpy@PLT
	jmp	.L68
.L65:
	movq	-40(%rbp), %rax
	movq	24(%rax), %rax
	movq	%rax, %rdi
	call	strlen@PLT
	addq	$1, %rax
	movq	%rax, %rdi
	call	malloc@PLT
	movq	%rax, %rdx
	movq	-32(%rbp), %rax
	movq	%rdx, 24(%rax)
	movq	-40(%rbp), %rax
	movq	24(%rax), %rdx
	movq	-32(%rbp), %rax
	movq	24(%rax), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	strcpy@PLT
	jmp	.L68
.L67:
	movq	-40(%rbp), %rax
	movl	40(%rax), %edx
	movq	-32(%rbp), %rax
	movl	%edx, 40(%rax)
	movq	-32(%rbp), %rax
	movl	40(%rax), %eax
	cltq
	salq	$3, %rax
	movq	%rax, %rdi
	call	malloc@PLT
	movq	%rax, %rdx
	movq	-32(%rbp), %rax
	movq	%rdx, 48(%rax)
	movl	$0, -20(%rbp)
	jmp	.L69
.L70:
	movq	-32(%rbp), %rax
	movq	48(%rax), %rax
	movl	-20(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	leaq	(%rax,%rdx), %rbx
	movq	-40(%rbp), %rax
	movq	48(%rax), %rax
	movl	-20(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movq	%rax, %rdi
	call	copy_lispvalue
	movq	%rax, (%rbx)
	addl	$1, -20(%rbp)
.L69:
	movq	-32(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	-20(%rbp), %eax
	jg	.L70
.L72:
	nop
.L68:
	movq	-32(%rbp), %rax
	addq	$40, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE16:
	.size	copy_lispvalue, .-copy_lispvalue
	.globl	print_lispvalue_expr
	.type	print_lispvalue_expr, @function
print_lispvalue_expr:
.LFB17:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movl	%esi, %ecx
	movl	%edx, %eax
	movb	%cl, -28(%rbp)
	movb	%al, -32(%rbp)
	movsbl	-28(%rbp), %eax
	movl	%eax, %edi
	call	putchar@PLT
	movl	$0, -4(%rbp)
	jmp	.L74
.L75:
	movq	-24(%rbp), %rax
	movq	48(%rax), %rax
	movl	-4(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movq	%rax, %rdi
	call	print_lispvalue
	movl	$32, %edi
	call	putchar@PLT
	addl	$1, -4(%rbp)
.L74:
	movq	-24(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	-4(%rbp), %eax
	jg	.L75
	movsbl	-32(%rbp), %eax
	movl	%eax, %edi
	call	putchar@PLT
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE17:
	.size	print_lispvalue_expr, .-print_lispvalue_expr
	.section	.rodata
.LC27:
	.string	"\033[33m%f\033[0m"
.LC28:
	.string	"\033[31mError : %s\033[0m"
.LC29:
	.string	"\033[36m%s\033[0m"
.LC30:
	.string	"\033[32m<function>\033[0m"
	.text
	.globl	print_lispvalue
	.type	print_lispvalue, @function
print_lispvalue:
.LFB18:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$5, %eax
	ja	.L86
	movl	%eax, %eax
	leaq	0(,%rax,4), %rdx
	leaq	.L79(%rip), %rax
	movl	(%rdx,%rax), %eax
	movslq	%eax, %rdx
	leaq	.L79(%rip), %rax
	addq	%rdx, %rax
	jmp	*%rax
	.section	.rodata
	.align 4
	.align 4
.L79:
	.long	.L78-.L79
	.long	.L80-.L79
	.long	.L81-.L79
	.long	.L82-.L79
	.long	.L83-.L79
	.long	.L84-.L79
	.text
.L80:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, -16(%rbp)
	movsd	-16(%rbp), %xmm0
	leaq	.LC27(%rip), %rdi
	movl	$1, %eax
	call	printf@PLT
	jmp	.L85
.L78:
	movq	-8(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rax, %rsi
	leaq	.LC28(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	jmp	.L85
.L81:
	movq	-8(%rbp), %rax
	movq	24(%rax), %rax
	movq	%rax, %rsi
	leaq	.LC29(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	jmp	.L85
.L82:
	leaq	.LC30(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	jmp	.L85
.L83:
	movq	-8(%rbp), %rax
	movl	$41, %edx
	movl	$40, %esi
	movq	%rax, %rdi
	call	print_lispvalue_expr
	jmp	.L85
.L84:
	movq	-8(%rbp), %rax
	movl	$125, %edx
	movl	$123, %esi
	movq	%rax, %rdi
	call	print_lispvalue_expr
	jmp	.L85
.L86:
	nop
.L85:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE18:
	.size	print_lispvalue, .-print_lispvalue
	.globl	print_lispvalue_newline
	.type	print_lispvalue_newline, @function
print_lispvalue_newline:
.LFB19:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	print_lispvalue
	movl	$10, %edi
	call	putchar@PLT
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE19:
	.size	print_lispvalue_newline, .-print_lispvalue_newline
	.section	.rodata
	.align 8
.LC31:
	.string	"first element is not a function !"
	.text
	.globl	eval_lispvalue_sexpr
	.type	eval_lispvalue_sexpr, @function
eval_lispvalue_sexpr:
.LFB20:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$56, %rsp
	.cfi_offset 3, -24
	movq	%rdi, -56(%rbp)
	movq	%rsi, -64(%rbp)
	movl	$0, -20(%rbp)
	jmp	.L89
.L90:
	movq	-64(%rbp), %rax
	movq	48(%rax), %rax
	movl	-20(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	leaq	(%rax,%rdx), %rbx
	movq	-64(%rbp), %rax
	movq	48(%rax), %rax
	movl	-20(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rdx
	movq	-56(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	eval_lispvalue
	movq	%rax, (%rbx)
	addl	$1, -20(%rbp)
.L89:
	movq	-64(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	-20(%rbp), %eax
	jg	.L90
	movl	$0, -24(%rbp)
	jmp	.L91
.L94:
	movq	-64(%rbp), %rax
	movq	48(%rax), %rax
	movl	-24(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movl	(%rax), %eax
	testl	%eax, %eax
	jne	.L92
	movl	-24(%rbp), %edx
	movq	-64(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	take_lispvalue
	jmp	.L93
.L92:
	addl	$1, -24(%rbp)
.L91:
	movq	-64(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	-24(%rbp), %eax
	jg	.L94
	movq	-64(%rbp), %rax
	movl	40(%rax), %eax
	testl	%eax, %eax
	jne	.L95
	movq	-64(%rbp), %rax
	jmp	.L93
.L95:
	movq	-64(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	$1, %eax
	jne	.L96
	movq	-64(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	take_lispvalue
	jmp	.L93
.L96:
	movq	-64(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	pop_lispvalue
	movq	%rax, -32(%rbp)
	movq	-32(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$3, %eax
	je	.L97
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	movq	-64(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC31(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L93
.L97:
	movq	-32(%rbp), %rax
	movq	32(%rax), %rax
	movq	-64(%rbp), %rcx
	movq	-56(%rbp), %rdx
	movq	%rcx, %rsi
	movq	%rdx, %rdi
	call	*%rax
	movq	%rax, -40(%rbp)
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	movq	-40(%rbp), %rax
.L93:
	addq	$56, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE20:
	.size	eval_lispvalue_sexpr, .-eval_lispvalue_sexpr
	.globl	eval_lispvalue
	.type	eval_lispvalue, @function
eval_lispvalue:
.LFB21:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	-32(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$2, %eax
	jne	.L99
	movq	-32(%rbp), %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	get_lispenv
	movq	%rax, -8(%rbp)
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	movq	-8(%rbp), %rax
	jmp	.L100
.L99:
	movq	-32(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$4, %eax
	jne	.L101
	movq	-32(%rbp), %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	eval_lispvalue_sexpr
	jmp	.L100
.L101:
	movq	-32(%rbp), %rax
.L100:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE21:
	.size	eval_lispvalue, .-eval_lispvalue
	.globl	pop_lispvalue
	.type	pop_lispvalue, @function
pop_lispvalue:
.LFB22:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movl	%esi, -28(%rbp)
	movq	-24(%rbp), %rax
	movq	48(%rax), %rax
	movl	-28(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movq	%rax, -8(%rbp)
	movq	-24(%rbp), %rax
	movl	40(%rax), %eax
	subl	-28(%rbp), %eax
	subl	$1, %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	-24(%rbp), %rax
	movq	48(%rax), %rax
	movl	-28(%rbp), %ecx
	movslq	%ecx, %rcx
	addq	$1, %rcx
	salq	$3, %rcx
	addq	%rax, %rcx
	movq	-24(%rbp), %rax
	movq	48(%rax), %rax
	movl	-28(%rbp), %esi
	movslq	%esi, %rsi
	salq	$3, %rsi
	addq	%rsi, %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	memmove@PLT
	movq	-24(%rbp), %rax
	movl	40(%rax), %eax
	leal	-1(%rax), %edx
	movq	-24(%rbp), %rax
	movl	%edx, 40(%rax)
	movq	-24(%rbp), %rax
	movl	40(%rax), %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	-24(%rbp), %rax
	movq	48(%rax), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	realloc@PLT
	movq	%rax, %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, 48(%rax)
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE22:
	.size	pop_lispvalue, .-pop_lispvalue
	.globl	take_lispvalue
	.type	take_lispvalue, @function
take_lispvalue:
.LFB23:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movl	%esi, -28(%rbp)
	movl	-28(%rbp), %edx
	movq	-24(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	pop_lispvalue
	movq	%rax, -8(%rbp)
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE23:
	.size	take_lispvalue, .-take_lispvalue
	.globl	are_all_numbers
	.type	are_all_numbers, @function
are_all_numbers:
.LFB24:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	movl	$0, -4(%rbp)
	jmp	.L107
.L110:
	movq	-24(%rbp), %rax
	movq	48(%rax), %rax
	movl	-4(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movl	(%rax), %eax
	cmpl	$1, %eax
	je	.L108
	movl	$0, %eax
	jmp	.L109
.L108:
	addl	$1, -4(%rbp)
.L107:
	movq	-24(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	-4(%rbp), %eax
	jg	.L110
	movl	$1, %eax
.L109:
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE24:
	.size	are_all_numbers, .-are_all_numbers
	.globl	builtin_add
	.type	builtin_add, @function
builtin_add:
.LFB25:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-16(%rbp), %rcx
	movq	-8(%rbp), %rax
	leaq	.LC8(%rip), %rdx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	builtin_operator
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE25:
	.size	builtin_add, .-builtin_add
	.globl	builtin_sub
	.type	builtin_sub, @function
builtin_sub:
.LFB26:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-16(%rbp), %rcx
	movq	-8(%rbp), %rax
	leaq	.LC9(%rip), %rdx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	builtin_operator
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE26:
	.size	builtin_sub, .-builtin_sub
	.globl	builtin_mul
	.type	builtin_mul, @function
builtin_mul:
.LFB27:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-16(%rbp), %rcx
	movq	-8(%rbp), %rax
	leaq	.LC10(%rip), %rdx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	builtin_operator
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE27:
	.size	builtin_mul, .-builtin_mul
	.globl	builtin_div
	.type	builtin_div, @function
builtin_div:
.LFB28:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-16(%rbp), %rcx
	movq	-8(%rbp), %rax
	leaq	.LC11(%rip), %rdx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	builtin_operator
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE28:
	.size	builtin_div, .-builtin_div
	.globl	builtin_mod
	.type	builtin_mod, @function
builtin_mod:
.LFB29:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-16(%rbp), %rcx
	movq	-8(%rbp), %rax
	leaq	.LC12(%rip), %rdx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	builtin_operator
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE29:
	.size	builtin_mod, .-builtin_mod
	.globl	builtin_pow
	.type	builtin_pow, @function
builtin_pow:
.LFB30:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-16(%rbp), %rcx
	movq	-8(%rbp), %rax
	leaq	.LC13(%rip), %rdx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	builtin_operator
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE30:
	.size	builtin_pow, .-builtin_pow
	.globl	builtin_max
	.type	builtin_max, @function
builtin_max:
.LFB31:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-16(%rbp), %rcx
	movq	-8(%rbp), %rax
	leaq	.LC14(%rip), %rdx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	builtin_operator
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE31:
	.size	builtin_max, .-builtin_max
	.globl	builtin_min
	.type	builtin_min, @function
builtin_min:
.LFB32:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-16(%rbp), %rcx
	movq	-8(%rbp), %rax
	leaq	.LC15(%rip), %rdx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	builtin_operator
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE32:
	.size	builtin_min, .-builtin_min
	.section	.rodata
	.align 8
.LC32:
	.string	"Function 'head' passed incorrect number of arguments !"
	.align 8
.LC33:
	.string	"Function 'head' passed incorrect types !"
.LC34:
	.string	"Function 'head' passed {} !"
	.text
	.globl	builtin_head
	.type	builtin_head, @function
builtin_head:
.LFB33:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	-32(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	$1, %eax
	je	.L128
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC32(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L129
.L128:
	movq	-32(%rbp), %rax
	movq	48(%rax), %rax
	movq	(%rax), %rax
	movl	(%rax), %eax
	cmpl	$5, %eax
	je	.L130
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC33(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L129
.L130:
	movq	-32(%rbp), %rax
	movq	48(%rax), %rax
	movq	(%rax), %rax
	movl	40(%rax), %eax
	testl	%eax, %eax
	jne	.L131
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC34(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L129
.L131:
	movq	-32(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	take_lispvalue
	movq	%rax, -8(%rbp)
	jmp	.L132
.L133:
	movq	-8(%rbp), %rax
	movl	$1, %esi
	movq	%rax, %rdi
	call	pop_lispvalue
	movq	%rax, %rdi
	call	delete_lispvalue
.L132:
	movq	-8(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	$1, %eax
	jg	.L133
	movq	-8(%rbp), %rax
.L129:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE33:
	.size	builtin_head, .-builtin_head
	.section	.rodata
	.align 8
.LC35:
	.string	"Function 'tail' passed incorrect number of arguments !"
	.align 8
.LC36:
	.string	"Function 'tail' passed incorrect types !"
.LC37:
	.string	"Function 'tail' passed {} !"
	.text
	.globl	builtin_tail
	.type	builtin_tail, @function
builtin_tail:
.LFB34:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	-32(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	$1, %eax
	je	.L135
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC35(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L136
.L135:
	movq	-32(%rbp), %rax
	movq	48(%rax), %rax
	movq	(%rax), %rax
	movl	(%rax), %eax
	cmpl	$5, %eax
	je	.L137
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC36(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L136
.L137:
	movq	-32(%rbp), %rax
	movq	48(%rax), %rax
	movq	(%rax), %rax
	movl	40(%rax), %eax
	testl	%eax, %eax
	jne	.L138
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC37(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L136
.L138:
	movq	-32(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	take_lispvalue
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	pop_lispvalue
	movq	%rax, %rdi
	call	delete_lispvalue
	movq	-8(%rbp), %rax
.L136:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE34:
	.size	builtin_tail, .-builtin_tail
	.globl	builtin_list
	.type	builtin_list, @function
builtin_list:
.LFB35:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-16(%rbp), %rax
	movl	$5, (%rax)
	movq	-16(%rbp), %rax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE35:
	.size	builtin_list, .-builtin_list
	.section	.rodata
	.align 8
.LC38:
	.string	"Function 'init' passed incorrect number of arguments !"
	.align 8
.LC39:
	.string	"Function 'init' passed incorrect types !"
.LC40:
	.string	"Function 'init' passed {} !"
	.text
	.globl	builtin_init
	.type	builtin_init, @function
builtin_init:
.LFB36:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	-32(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	$1, %eax
	je	.L142
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC38(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L143
.L142:
	movq	-32(%rbp), %rax
	movq	48(%rax), %rax
	movq	(%rax), %rax
	movl	(%rax), %eax
	cmpl	$5, %eax
	je	.L144
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC39(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L143
.L144:
	movq	-32(%rbp), %rax
	movq	48(%rax), %rax
	movq	(%rax), %rax
	movl	40(%rax), %eax
	testl	%eax, %eax
	jne	.L145
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC40(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L143
.L145:
	movq	-32(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	take_lispvalue
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	48(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	40(%rax), %eax
	cltq
	salq	$3, %rax
	subq	$8, %rax
	addq	%rdx, %rax
	movq	(%rax), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	movq	-8(%rbp), %rax
	movl	40(%rax), %eax
	leal	-1(%rax), %edx
	movq	-8(%rbp), %rax
	movl	%edx, 40(%rax)
	movq	-8(%rbp), %rax
	movl	40(%rax), %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	-8(%rbp), %rax
	movq	48(%rax), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	realloc@PLT
	movq	%rax, %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 48(%rax)
	movq	-8(%rbp), %rax
.L143:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE36:
	.size	builtin_init, .-builtin_init
	.section	.rodata
	.align 8
.LC41:
	.string	"Function 'eval' passed incorrect number of arguments !"
	.align 8
.LC42:
	.string	"Function 'eval' passed incorrect types !"
	.text
	.globl	builtin_eval
	.type	builtin_eval, @function
builtin_eval:
.LFB37:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	-32(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	$1, %eax
	je	.L147
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC41(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L148
.L147:
	movq	-32(%rbp), %rax
	movq	48(%rax), %rax
	movq	(%rax), %rax
	movl	(%rax), %eax
	cmpl	$5, %eax
	je	.L149
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC42(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L148
.L149:
	movq	-32(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	take_lispvalue
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movl	$4, (%rax)
	movq	-8(%rbp), %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	eval_lispvalue
.L148:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE37:
	.size	builtin_eval, .-builtin_eval
	.globl	join_lispvalue
	.type	join_lispvalue, @function
join_lispvalue:
.LFB38:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	jmp	.L151
.L152:
	movq	-16(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	pop_lispvalue
	movq	%rax, %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	add_lispvalue
	movq	%rax, -8(%rbp)
.L151:
	movq	-16(%rbp), %rax
	movl	40(%rax), %eax
	testl	%eax, %eax
	jne	.L152
	movq	-16(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE38:
	.size	join_lispvalue, .-join_lispvalue
	.section	.rodata
	.align 8
.LC43:
	.string	"Function 'join' passed incorrect types !"
	.text
	.globl	builtin_join
	.type	builtin_join, @function
builtin_join:
.LFB39:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movl	$0, -4(%rbp)
	jmp	.L155
.L158:
	movq	-32(%rbp), %rax
	movq	48(%rax), %rax
	movl	-4(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movl	(%rax), %eax
	cmpl	$5, %eax
	je	.L156
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC43(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L157
.L156:
	addl	$1, -4(%rbp)
.L155:
	movq	-32(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	-4(%rbp), %eax
	jg	.L158
	movq	-32(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	pop_lispvalue
	movq	%rax, -16(%rbp)
	jmp	.L159
.L160:
	movq	-32(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	pop_lispvalue
	movq	%rax, %rdx
	movq	-16(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	join_lispvalue
	movq	%rax, -16(%rbp)
.L159:
	movq	-32(%rbp), %rax
	movl	40(%rax), %eax
	testl	%eax, %eax
	jne	.L160
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	movq	-16(%rbp), %rax
.L157:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE39:
	.size	builtin_join, .-builtin_join
	.section	.rodata
	.align 8
.LC44:
	.string	"Function 'cons' passed incorrect number of arguments !"
	.align 8
.LC45:
	.string	"Function 'cons' passed incorrect types !"
	.text
	.globl	builtin_cons
	.type	builtin_cons, @function
builtin_cons:
.LFB40:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	-32(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	$2, %eax
	je	.L162
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC44(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L163
.L162:
	movq	-32(%rbp), %rax
	movq	48(%rax), %rax
	addq	$8, %rax
	movq	(%rax), %rax
	movl	(%rax), %eax
	cmpl	$5, %eax
	je	.L164
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC45(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L163
.L164:
	movq	-32(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	pop_lispvalue
	movq	%rax, -8(%rbp)
	movq	-32(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	pop_lispvalue
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	movl	40(%rax), %eax
	leal	1(%rax), %edx
	movq	-16(%rbp), %rax
	movl	%edx, 40(%rax)
	movq	-16(%rbp), %rax
	movl	40(%rax), %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	-16(%rbp), %rax
	movq	48(%rax), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	realloc@PLT
	movq	%rax, %rdx
	movq	-16(%rbp), %rax
	movq	%rdx, 48(%rax)
	movq	-16(%rbp), %rax
	movl	40(%rax), %eax
	subl	$1, %eax
	cltq
	leaq	0(,%rax,8), %rsi
	movq	-16(%rbp), %rax
	movq	48(%rax), %rax
	movq	-16(%rbp), %rdx
	movq	48(%rdx), %rdx
	leaq	8(%rdx), %rcx
	movq	%rsi, %rdx
	movq	%rax, %rsi
	movq	%rcx, %rdi
	call	memmove@PLT
	movq	-16(%rbp), %rax
	movq	48(%rax), %rax
	movq	-8(%rbp), %rdx
	movq	%rdx, (%rax)
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	movq	-16(%rbp), %rax
.L163:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE40:
	.size	builtin_cons, .-builtin_cons
	.section	.rodata
	.align 8
.LC46:
	.string	"Function 'len' passed incorrect number of arguments !"
	.align 8
.LC47:
	.string	"Function 'len' passed incorrect types !"
	.text
	.globl	builtin_len
	.type	builtin_len, @function
builtin_len:
.LFB41:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	-32(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	$1, %eax
	je	.L166
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC46(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L167
.L166:
	movq	-32(%rbp), %rax
	movq	48(%rax), %rax
	movq	(%rax), %rax
	movl	(%rax), %eax
	cmpl	$5, %eax
	je	.L168
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC47(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L167
.L168:
	movq	-32(%rbp), %rax
	movq	48(%rax), %rax
	movq	(%rax), %rax
	movl	40(%rax), %eax
	pxor	%xmm0, %xmm0
	cvtsi2sd	%eax, %xmm0
	call	new_lispvalue_number
	movq	%rax, -8(%rbp)
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	movq	-8(%rbp), %rax
.L167:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE41:
	.size	builtin_len, .-builtin_len
	.section	.rodata
	.align 8
.LC48:
	.string	"Function 'def' passed incorrect types !"
	.align 8
.LC49:
	.string	"Function 'def' cannot define non-symbol !"
	.align 8
.LC50:
	.string	"Function 'def' passed non-matching number of values and symbols"
	.text
	.globl	builtin_def
	.type	builtin_def, @function
builtin_def:
.LFB42:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$112, %rsp
	movq	%rdi, -104(%rbp)
	movq	%rsi, -112(%rbp)
	movq	-112(%rbp), %rax
	movq	48(%rax), %rax
	movq	(%rax), %rax
	movl	(%rax), %eax
	cmpl	$5, %eax
	je	.L170
	movq	-112(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC48(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L179
.L170:
	movq	-112(%rbp), %rax
	movq	48(%rax), %rax
	movq	(%rax), %rax
	movq	%rax, -16(%rbp)
	movl	$0, -4(%rbp)
	jmp	.L172
.L174:
	movq	-16(%rbp), %rax
	movq	48(%rax), %rax
	movl	-4(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	movl	(%rax), %eax
	cmpl	$2, %eax
	je	.L173
	movq	-112(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC49(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L179
.L173:
	addl	$1, -4(%rbp)
.L172:
	movq	-16(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	-4(%rbp), %eax
	jg	.L174
	movq	-16(%rbp), %rax
	movl	40(%rax), %edx
	movq	-112(%rbp), %rax
	movl	40(%rax), %eax
	subl	$1, %eax
	cmpl	%eax, %edx
	je	.L175
	movq	-112(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC50(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L179
.L175:
	movl	$0, -8(%rbp)
	jmp	.L176
.L177:
	movq	-112(%rbp), %rax
	movq	48(%rax), %rax
	movl	-8(%rbp), %edx
	movslq	%edx, %rdx
	addq	$1, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rdx
	movq	-16(%rbp), %rax
	movq	48(%rax), %rax
	movl	-8(%rbp), %ecx
	movslq	%ecx, %rcx
	salq	$3, %rcx
	addq	%rcx, %rax
	movq	(%rax), %rcx
	movq	-104(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	put_lispenv
	addl	$1, -8(%rbp)
.L176:
	movq	-16(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	-8(%rbp), %eax
	jg	.L177
.L178:
	movq	-112(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	call	new_lispvalue_sexpr
.L179:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE42:
	.size	builtin_def, .-builtin_def
	.section	.rodata
	.align 8
.LC51:
	.string	"Cannot operate on non-number !"
.LC54:
	.string	"Division by Zero !"
	.text
	.globl	builtin_operator
	.type	builtin_operator, @function
builtin_operator:
.LFB43:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	%rdx, -40(%rbp)
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	are_all_numbers
	testl	%eax, %eax
	jne	.L181
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC51(%rip), %rdi
	call	new_lispvalue_error
	jmp	.L182
.L181:
	movq	-32(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	pop_lispvalue
	movq	%rax, -8(%rbp)
	movq	-40(%rbp), %rax
	leaq	.LC9(%rip), %rsi
	movq	%rax, %rdi
	call	strcmp@PLT
	testl	%eax, %eax
	jne	.L184
	movq	-32(%rbp), %rax
	movl	40(%rax), %eax
	testl	%eax, %eax
	jne	.L184
	movq	-8(%rbp), %rax
	movsd	8(%rax), %xmm1
	movsd	.LC52(%rip), %xmm0
	xorpd	%xmm1, %xmm0
	movq	-8(%rbp), %rax
	movsd	%xmm0, 8(%rax)
	jmp	.L184
.L205:
	movq	-32(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	pop_lispvalue
	movq	%rax, -16(%rbp)
	movq	-40(%rbp), %rax
	movzbl	(%rax), %eax
	movsbl	%al, %eax
	subl	$37, %eax
	cmpl	$57, %eax
	ja	.L211
	movl	%eax, %eax
	leaq	0(,%rax,4), %rdx
	leaq	.L187(%rip), %rax
	movl	(%rdx,%rax), %eax
	movslq	%eax, %rdx
	leaq	.L187(%rip), %rax
	addq	%rdx, %rax
	jmp	*%rax
	.section	.rodata
	.align 4
	.align 4
.L187:
	.long	.L186-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L188-.L187
	.long	.L189-.L187
	.long	.L211-.L187
	.long	.L190-.L187
	.long	.L211-.L187
	.long	.L191-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L192-.L187
	.long	.L211-.L187
	.long	.L193-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L211-.L187
	.long	.L194-.L187
	.text
.L189:
	movq	-8(%rbp), %rax
	movsd	8(%rax), %xmm1
	movq	-16(%rbp), %rax
	movsd	8(%rax), %xmm0
	addsd	%xmm1, %xmm0
	movq	-8(%rbp), %rax
	movsd	%xmm0, 8(%rax)
	jmp	.L195
.L190:
	movq	-8(%rbp), %rax
	movsd	8(%rax), %xmm0
	movq	-16(%rbp), %rax
	movsd	8(%rax), %xmm1
	subsd	%xmm1, %xmm0
	movq	-8(%rbp), %rax
	movsd	%xmm0, 8(%rax)
	jmp	.L195
.L188:
	movq	-8(%rbp), %rax
	movsd	8(%rax), %xmm1
	movq	-16(%rbp), %rax
	movsd	8(%rax), %xmm0
	mulsd	%xmm1, %xmm0
	movq	-8(%rbp), %rax
	movsd	%xmm0, 8(%rax)
	jmp	.L195
.L191:
	movq	-16(%rbp), %rax
	movsd	8(%rax), %xmm0
	pxor	%xmm1, %xmm1
	ucomisd	%xmm1, %xmm0
	jp	.L196
	pxor	%xmm1, %xmm1
	ucomisd	%xmm1, %xmm0
	jne	.L196
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	leaq	.LC54(%rip), %rdi
	call	new_lispvalue_error
	movq	%rax, -8(%rbp)
	jmp	.L195
.L196:
	movq	-8(%rbp), %rax
	movsd	8(%rax), %xmm0
	movq	-16(%rbp), %rax
	movsd	8(%rax), %xmm1
	divsd	%xmm1, %xmm0
	movq	-8(%rbp), %rax
	movsd	%xmm0, 8(%rax)
	jmp	.L195
.L186:
	movq	-16(%rbp), %rax
	movsd	8(%rax), %xmm0
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	movapd	%xmm0, %xmm1
	movq	%rax, -48(%rbp)
	movsd	-48(%rbp), %xmm0
	call	fmod@PLT
	movq	%xmm0, %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 8(%rax)
	jmp	.L195
.L194:
	movq	-16(%rbp), %rax
	movsd	8(%rax), %xmm0
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	movapd	%xmm0, %xmm1
	movq	%rax, -48(%rbp)
	movsd	-48(%rbp), %xmm0
	call	pow@PLT
	movq	%xmm0, %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 8(%rax)
	jmp	.L195
.L193:
	movq	-8(%rbp), %rax
	movsd	8(%rax), %xmm0
	movq	-16(%rbp), %rax
	movsd	8(%rax), %xmm1
	ucomisd	%xmm1, %xmm0
	jbe	.L209
	movq	-8(%rbp), %rax
	movsd	8(%rax), %xmm0
	jmp	.L201
.L209:
	movq	-16(%rbp), %rax
	movsd	8(%rax), %xmm0
.L201:
	movq	-8(%rbp), %rax
	movsd	%xmm0, 8(%rax)
	jmp	.L195
.L192:
	movq	-8(%rbp), %rax
	movsd	8(%rax), %xmm1
	movq	-16(%rbp), %rax
	movsd	8(%rax), %xmm0
	ucomisd	%xmm1, %xmm0
	jbe	.L210
	movq	-8(%rbp), %rax
	movsd	8(%rax), %xmm0
	jmp	.L204
.L210:
	movq	-16(%rbp), %rax
	movsd	8(%rax), %xmm0
.L204:
	movq	-8(%rbp), %rax
	movsd	%xmm0, 8(%rax)
	jmp	.L195
.L211:
	nop
.L195:
	movq	-16(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
.L184:
	movq	-32(%rbp), %rax
	movl	40(%rax), %eax
	testl	%eax, %eax
	jg	.L205
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	movq	-8(%rbp), %rax
.L182:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE43:
	.size	builtin_operator, .-builtin_operator
	.globl	add_builtin_lispenv
	.type	add_builtin_lispenv, @function
add_builtin_lispenv:
.LFB44:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	%rdx, -40(%rbp)
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	new_lispvalue_symbol
	movq	%rax, -8(%rbp)
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	call	new_lispvalue_function
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rdx
	movq	-8(%rbp), %rcx
	movq	-24(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	put_lispenv
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	movq	-16(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE44:
	.size	add_builtin_lispenv, .-add_builtin_lispenv
	.section	.rodata
.LC55:
	.string	"add"
.LC56:
	.string	"sub"
.LC57:
	.string	"mul"
.LC58:
	.string	"div"
.LC59:
	.string	"mod"
.LC60:
	.string	"pow"
.LC61:
	.string	"max"
.LC62:
	.string	"min"
.LC63:
	.string	"def"
	.text
	.globl	add_basicbuiltins_lispenv
	.type	add_basicbuiltins_lispenv, @function
add_basicbuiltins_lispenv:
.LFB45:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	leaq	builtin_add(%rip), %rdx
	leaq	.LC55(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_sub(%rip), %rdx
	leaq	.LC56(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_mul(%rip), %rdx
	leaq	.LC57(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_div(%rip), %rdx
	leaq	.LC58(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_mod(%rip), %rdx
	leaq	.LC59(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_pow(%rip), %rdx
	leaq	.LC60(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_max(%rip), %rdx
	leaq	.LC61(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_min(%rip), %rdx
	leaq	.LC62(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_add(%rip), %rdx
	leaq	.LC8(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_sub(%rip), %rdx
	leaq	.LC9(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_mul(%rip), %rdx
	leaq	.LC10(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_div(%rip), %rdx
	leaq	.LC11(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_mod(%rip), %rdx
	leaq	.LC12(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_pow(%rip), %rdx
	leaq	.LC13(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_max(%rip), %rdx
	leaq	.LC14(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_min(%rip), %rdx
	leaq	.LC15(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_head(%rip), %rdx
	leaq	.LC1(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_tail(%rip), %rdx
	leaq	.LC2(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_list(%rip), %rdx
	leaq	.LC0(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_init(%rip), %rdx
	leaq	.LC6(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_eval(%rip), %rdx
	leaq	.LC7(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_join(%rip), %rdx
	leaq	.LC3(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_cons(%rip), %rdx
	leaq	.LC4(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_len(%rip), %rdx
	leaq	.LC5(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	movq	-8(%rbp), %rax
	leaq	builtin_def(%rip), %rdx
	leaq	.LC63(%rip), %rsi
	movq	%rax, %rdi
	call	add_builtin_lispenv
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE45:
	.size	add_basicbuiltins_lispenv, .-add_basicbuiltins_lispenv
	.globl	completion_generator
	.type	completion_generator, @function
completion_generator:
.LFB46:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movl	%esi, -28(%rbp)
	cmpl	$0, -28(%rbp)
	jne	.L216
	movl	$0, match_index.5157(%rip)
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	strlen@PLT
	movl	%eax, length.5158(%rip)
	jmp	.L216
.L218:
	movl	length.5158(%rip), %eax
	movslq	%eax, %rdx
	movq	-24(%rbp), %rcx
	movq	-8(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	strncmp@PLT
	testl	%eax, %eax
	jne	.L216
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	strdup@PLT
	jmp	.L217
.L216:
	movl	match_index.5157(%rip), %eax
	leal	1(%rax), %edx
	movl	%edx, match_index.5157(%rip)
	cltq
	leaq	0(,%rax,8), %rdx
	leaq	vocabulary(%rip), %rax
	movq	(%rdx,%rax), %rax
	movq	%rax, -8(%rbp)
	cmpq	$0, -8(%rbp)
	jne	.L218
	movl	$0, %eax
.L217:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE46:
	.size	completion_generator, .-completion_generator
	.globl	completer
	.type	completer, @function
completer:
.LFB47:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	movl	%edx, -16(%rbp)
	movq	-8(%rbp), %rax
	leaq	completion_generator(%rip), %rsi
	movq	%rax, %rdi
	call	rl_completion_matches@PLT
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE47:
	.size	completer, .-completer
	.section	.rodata
.LC64:
	.string	"\033[1m\033[32mlispy> \033[0m"
	.text
	.globl	print_prompt
	.type	print_prompt, @function
print_prompt:
.LFB48:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	stdout(%rip), %rax
	movq	%rax, %rcx
	movl	$20, %edx
	movl	$1, %esi
	leaq	.LC64(%rip), %rdi
	call	fwrite@PLT
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE48:
	.size	print_prompt, .-print_prompt
	.section	.rodata
.LC65:
	.string	"expr"
.LC66:
	.string	"lispy"
	.align 8
.LC67:
	.string	"number   : /[-]?[0-9]+[.]?[0-9]*([eE][-+]?[0-9]+)?/ ;symbol   : /[a-zA-Z0-9_+\\-*%^\\/\\\\=<>!&]+/ ;sexpr    : '(' <expr>* ')' ;qexpr    : '{' <expr>* '}' ;expr     : <number> | <symbol> | <sexpr> | <qexpr> ;lispy    : /^/ <expr>* /$/ ;"
	.align 8
.LC68:
	.string	"\033[44mLispy version 0.0.0.0.1 \033[0m\033[96mto Exit press CTRL + C\033[0m"
.LC69:
	.string	""
.LC70:
	.string	"<stdin>"
	.text
	.globl	main
	.type	main, @function
main:
.LFB49:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$80, %rsp
	leaq	.LC22(%rip), %rdi
	call	mpc_new@PLT
	movq	%rax, -8(%rbp)
	leaq	.LC23(%rip), %rdi
	call	mpc_new@PLT
	movq	%rax, -16(%rbp)
	leaq	.LC24(%rip), %rdi
	call	mpc_new@PLT
	movq	%rax, -24(%rbp)
	leaq	.LC25(%rip), %rdi
	call	mpc_new@PLT
	movq	%rax, -32(%rbp)
	leaq	.LC65(%rip), %rdi
	call	mpc_new@PLT
	movq	%rax, -40(%rbp)
	leaq	.LC66(%rip), %rdi
	call	mpc_new@PLT
	movq	%rax, -48(%rbp)
	movq	-32(%rbp), %rsi
	movq	-24(%rbp), %rcx
	movq	-16(%rbp), %rdx
	movq	-8(%rbp), %rax
	pushq	-48(%rbp)
	pushq	-40(%rbp)
	movq	%rsi, %r9
	movq	%rcx, %r8
	movq	%rdx, %rcx
	movq	%rax, %rdx
	leaq	.LC67(%rip), %rsi
	movl	$0, %edi
	movl	$0, %eax
	call	mpca_lang@PLT
	addq	$16, %rsp
	leaq	completer(%rip), %rax
	movq	%rax, rl_attempted_completion_function(%rip)
	leaq	.LC68(%rip), %rdi
	call	puts@PLT
	call	new_lispenv
	movq	%rax, -56(%rbp)
	movq	-56(%rbp), %rax
	movq	%rax, %rdi
	call	add_basicbuiltins_lispenv
.L226:
	movl	$0, %eax
	call	print_prompt
	leaq	.LC69(%rip), %rdi
	call	readline@PLT
	movq	%rax, -64(%rbp)
	cmpq	$0, -64(%rbp)
	je	.L226
	movq	-64(%rbp), %rax
	movq	%rax, %rdi
	call	add_history@PLT
	leaq	-80(%rbp), %rcx
	movq	-48(%rbp), %rdx
	movq	-64(%rbp), %rax
	movq	%rax, %rsi
	leaq	.LC70(%rip), %rdi
	call	mpc_parse@PLT
	testl	%eax, %eax
	je	.L224
	movq	-80(%rbp), %rax
	movq	%rax, %rdi
	call	mpc_ast_print@PLT
	movq	-80(%rbp), %rax
	movq	%rax, %rdi
	call	read_lispvalue
	movq	%rax, -72(%rbp)
	movq	-72(%rbp), %rax
	movq	%rax, %rdi
	call	print_lispvalue_newline
	movq	-72(%rbp), %rdx
	movq	-56(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	eval_lispvalue
	movq	%rax, -72(%rbp)
	movq	-72(%rbp), %rax
	movq	%rax, %rdi
	call	print_lispvalue_newline
	movq	-72(%rbp), %rax
	movq	%rax, %rdi
	call	delete_lispvalue
	movq	-80(%rbp), %rax
	movq	%rax, %rdi
	call	mpc_ast_delete@PLT
	jmp	.L225
.L224:
	movq	-80(%rbp), %rax
	movq	%rax, %rdi
	call	mpc_err_print@PLT
	movq	-80(%rbp), %rax
	movq	%rax, %rdi
	call	mpc_err_delete@PLT
.L225:
	movq	-64(%rbp), %rax
	movq	%rax, %rdi
	call	free@PLT
	jmp	.L226
	.cfi_endproc
.LFE49:
	.size	main, .-main
	.local	match_index.5157
	.comm	match_index.5157,4,4
	.local	length.5158
	.comm	length.5158,4,4
	.section	.rodata
	.align 16
.LC52:
	.long	0
	.long	-2147483648
	.long	0
	.long	0
	.ident	"GCC: (Debian 6.3.0-18+deb9u1) 6.3.0 20170516"
	.section	.note.GNU-stack,"",@progbits

	.arch armv8.4-a+fp16+sb+ssbs
	.build_version macos,  14, 0
	.text
	.globl _test_passed
	.zerofill __DATA,__common,_test_passed,8,3
	.globl _test_failed
	.zerofill __DATA,__common,_test_failed,8,3
	.text
	.align	2
	.globl _check_function_add
_check_function_add:
LFB14:
	stp	x29, x30, [sp, -48]!
LCFI0:
	mov	x29, sp
LCFI1:
	str	x0, [x29, 24]
	str	x1, [x29, 16]
	ldr	x1, [x29, 24]
	ldr	x0, [x29, 16]
	add	x0, x1, x0
	str	x0, [x29, 40]
	bl	_add_scanf
	mov	x1, x0
	ldr	x0, [x29, 40]
	cmp	x0, x1
	bne	L2
	adrp	x0, _test_passed@PAGE
	add	x0, x0, _test_passed@PAGEOFF;
	ldr	x0, [x0]
	add	x1, x0, 1
	adrp	x0, _test_passed@PAGE
	add	x0, x0, _test_passed@PAGEOFF;
	str	x1, [x0]
	b	L4
L2:
	adrp	x0, _test_failed@PAGE
	add	x0, x0, _test_failed@PAGEOFF;
	ldr	x0, [x0]
	add	x1, x0, 1
	adrp	x0, _test_failed@PAGE
	add	x0, x0, _test_failed@PAGEOFF;
	str	x1, [x0]
L4:
	nop
	ldp	x29, x30, [sp], 48
LCFI2:
	ret
LFE14:
	.cstring
	.align	3
lC0:
	.ascii "r\0"
	.align	3
lC1:
	.ascii "input_simple/input_simple.txt\0"
	.align	3
lC2:
	.ascii "%zu\0"
	.align	3
lC3:
	.ascii "%lld %lld\0"
	.text
	.align	2
	.globl _test_input
_test_input:
LFB15:
	sub	sp, sp, #80
LCFI3:
	stp	x29, x30, [sp, 16]
LCFI4:
	add	x29, sp, 16
LCFI5:
	adrp	x0, lC0@PAGE
	add	x1, x0, lC0@PAGEOFF;
	adrp	x0, lC1@PAGE
	add	x0, x0, lC1@PAGEOFF;
	bl	_fopen
	str	x0, [x29, 48]
	add	x0, x29, 40
	str	x0, [sp]
	adrp	x0, lC2@PAGE
	add	x1, x0, lC2@PAGEOFF;
	ldr	x0, [x29, 48]
	bl	_fscanf
	str	xzr, [x29, 56]
	b	L6
L7:
	add	x0, x29, 24
	str	x0, [sp, 8]
	add	x0, x29, 32
	str	x0, [sp]
	adrp	x0, lC3@PAGE
	add	x1, x0, lC3@PAGEOFF;
	ldr	x0, [x29, 48]
	bl	_fscanf
	ldr	x0, [x29, 32]
	ldr	x1, [x29, 24]
	bl	_check_function_add
	ldr	x0, [x29, 56]
	add	x0, x0, 1
	str	x0, [x29, 56]
L6:
	ldr	x0, [x29, 40]
	ldr	x1, [x29, 56]
	cmp	x1, x0
	bcc	L7
	ldr	x0, [x29, 48]
	bl	_fclose
	nop
	ldp	x29, x30, [sp, 16]
	add	sp, sp, 80
LCFI6:
	ret
LFE15:
	.cstring
	.align	3
lC4:
	.ascii "input_simple/input_simple_scan.txt\0"
	.align	3
lC5:
	.ascii "open\0"
	.align	3
lC6:
	.ascii "dup2\0"
	.align	3
lC7:
	.ascii "===== Simple test =====\0"
	.align	3
lC8:
	.ascii "Passed %zu tests\12\0"
	.align	3
lC9:
	.ascii "Failed %zu tests\12\0"
	.text
	.align	2
	.globl _main
_main:
LFB16:
	sub	sp, sp, #48
LCFI7:
	stp	x29, x30, [sp, 16]
LCFI8:
	add	x29, sp, 16
LCFI9:
	mov	w1, 0
	adrp	x0, lC4@PAGE
	add	x0, x0, lC4@PAGEOFF;
	bl	_open
	str	w0, [x29, 28]
	ldr	w0, [x29, 28]
	cmn	w0, #1
	bne	L9
	adrp	x0, lC5@PAGE
	add	x0, x0, lC5@PAGEOFF;
	bl	_perror
	mov	w0, 1
	b	L10
L9:
	mov	w1, 0
	ldr	w0, [x29, 28]
	bl	_dup2
	str	w0, [x29, 24]
	ldr	w0, [x29, 24]
	cmn	w0, #1
	bne	L11
	adrp	x0, lC6@PAGE
	add	x0, x0, lC6@PAGEOFF;
	bl	_perror
	mov	w0, 1
	b	L10
L11:
	bl	_test_input
	adrp	x0, lC7@PAGE
	add	x0, x0, lC7@PAGEOFF;
	bl	_puts
	adrp	x0, _test_passed@PAGE
	add	x0, x0, _test_passed@PAGEOFF;
	ldr	x0, [x0]
	str	x0, [sp]
	adrp	x0, lC8@PAGE
	add	x0, x0, lC8@PAGEOFF;
	bl	_printf
	adrp	x0, _test_failed@PAGE
	add	x0, x0, _test_failed@PAGEOFF;
	ldr	x0, [x0]
	str	x0, [sp]
	adrp	x0, lC9@PAGE
	add	x0, x0, lC9@PAGEOFF;
	bl	_printf
	adrp	x0, _test_failed@PAGE
	add	x0, x0, _test_failed@PAGEOFF;
	ldr	x0, [x0]
	cmp	x0, 0
	beq	L12
	mov	w0, 1
	b	L10
L12:
	mov	w0, 0
L10:
	ldp	x29, x30, [sp, 16]
	add	sp, sp, 48
LCFI10:
	ret
LFE16:
	.section __TEXT,__eh_frame,coalesced,no_toc+strip_static_syms+live_support
EH_frame1:
	.set L$set$0,LECIE1-LSCIE1
	.long L$set$0
LSCIE1:
	.long	0
	.byte	0x3
	.ascii "zR\0"
	.uleb128 0x1
	.sleb128 -8
	.uleb128 0x1e
	.uleb128 0x1
	.byte	0x10
	.byte	0xc
	.uleb128 0x1f
	.uleb128 0
	.align	3
LECIE1:
LSFDE1:
	.set L$set$1,LEFDE1-LASFDE1
	.long L$set$1
LASFDE1:
	.long	LASFDE1-EH_frame1
	.quad	LFB14-.
	.set L$set$2,LFE14-LFB14
	.quad L$set$2
	.uleb128 0
	.byte	0x4
	.set L$set$3,LCFI0-LFB14
	.long L$set$3
	.byte	0xe
	.uleb128 0x30
	.byte	0x9d
	.uleb128 0x6
	.byte	0x9e
	.uleb128 0x5
	.byte	0x4
	.set L$set$4,LCFI1-LCFI0
	.long L$set$4
	.byte	0xd
	.uleb128 0x1d
	.byte	0x4
	.set L$set$5,LCFI2-LCFI1
	.long L$set$5
	.byte	0xde
	.byte	0xdd
	.byte	0xc
	.uleb128 0x1f
	.uleb128 0
	.align	3
LEFDE1:
LSFDE3:
	.set L$set$6,LEFDE3-LASFDE3
	.long L$set$6
LASFDE3:
	.long	LASFDE3-EH_frame1
	.quad	LFB15-.
	.set L$set$7,LFE15-LFB15
	.quad L$set$7
	.uleb128 0
	.byte	0x4
	.set L$set$8,LCFI3-LFB15
	.long L$set$8
	.byte	0xe
	.uleb128 0x50
	.byte	0x4
	.set L$set$9,LCFI4-LCFI3
	.long L$set$9
	.byte	0x9d
	.uleb128 0x8
	.byte	0x9e
	.uleb128 0x7
	.byte	0x4
	.set L$set$10,LCFI5-LCFI4
	.long L$set$10
	.byte	0xc
	.uleb128 0x1d
	.uleb128 0x40
	.byte	0x4
	.set L$set$11,LCFI6-LCFI5
	.long L$set$11
	.byte	0xdd
	.byte	0xde
	.byte	0xc
	.uleb128 0x1f
	.uleb128 0
	.align	3
LEFDE3:
LSFDE5:
	.set L$set$12,LEFDE5-LASFDE5
	.long L$set$12
LASFDE5:
	.long	LASFDE5-EH_frame1
	.quad	LFB16-.
	.set L$set$13,LFE16-LFB16
	.quad L$set$13
	.uleb128 0
	.byte	0x4
	.set L$set$14,LCFI7-LFB16
	.long L$set$14
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.set L$set$15,LCFI8-LCFI7
	.long L$set$15
	.byte	0x9d
	.uleb128 0x4
	.byte	0x9e
	.uleb128 0x3
	.byte	0x4
	.set L$set$16,LCFI9-LCFI8
	.long L$set$16
	.byte	0xc
	.uleb128 0x1d
	.uleb128 0x20
	.byte	0x4
	.set L$set$17,LCFI10-LCFI9
	.long L$set$17
	.byte	0xdd
	.byte	0xde
	.byte	0xc
	.uleb128 0x1f
	.uleb128 0
	.align	3
LEFDE5:
	.ident	"GCC: (Homebrew GCC 13.3.0) 13.3.0"
	.subsections_via_symbols

    .text
    .global add_scanf

add_scanf:
    str fp, [sp, -8]!
    str x30, [sp, -8]!   
    mov fp, sp
    sub sp, sp, 32

    adrp x0, scanf_format_string
    add x0, x0, :lo12:scanf_format_string

    mov x1, sp
    add x2, x1, 8

    bl scanf

    ldr x0, [sp]
    ldr x1, [sp, 8]

    add x0, x0, x1

    add sp, sp, 32
    ldr x30, [sp], 8
    ldr fp, [sp], 8

    ret

    .section .rodata

scanf_format_string:
    .string "%lld %lld"

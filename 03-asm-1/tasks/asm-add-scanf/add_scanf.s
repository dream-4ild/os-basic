  .intel_syntax noprefix

  .text
  .global add_scanf

add_scanf:
  push rbp
  mov  rbp, rsp
  sub  rsp, 32

  lea rdi, [scanf_format_string]

  mov rsi, rsp
  mov rdx, rsi
  add rdx, 8

  call scanf

  mov rdi, [rsp]
  mov rsi, [rsp+8]

  add rdi, rsi
  mov rax, rdi




  mov rsp, rbp
  pop rbp
  ret

  .section .rodata

scanf_format_string:
  .string "%lld %lld"

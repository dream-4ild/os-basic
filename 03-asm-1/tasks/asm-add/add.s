  .intel_syntax noprefix

  .text
  .global add

add:
  add rdi, rsi

  mov rax, rdi

  ret

  .intel_syntax noprefix

  .text
  .global my_memcpy

my_memcpy:
  mov RCX, RDI

loop_8:
  cmp EDX, 8
  jl loop_1

  mov RAX, [RSI]
  mov [RDI], RAX

  add RSI, 8
  add RDI, 8

  sub EDX, 8
  jnz loop_8


loop_1:
  cmp EDX, 0
  je end

  mov AL, [RSI]
  mov [RDI], AL

  add RSI, 1
  add RDI, 1

  sub EDX, 1
  jnz loop_1

end:
  mov RAX, RCX
  ret

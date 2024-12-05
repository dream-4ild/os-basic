  .text
  .global longest_inc_subseq

longest_inc_subseq:
  mov x13, x0
  mov x12, x1
  mov x11, x2

  eor x10, x10, x10

loop:
  cmp x10, x11
  b.eq end1

  mov x0, x13
  mov x1, x12
  mov x2, x10
  mov x7, x10
  lsl x7, x7, 3
  ldr x3, [x0, x7]
  b find_max

ret_label:  
  mov x1, x10
  lsl x1, x1, 3
  add x1, x1, x12
  add x4, x4, 1

  str x4, [x1]

  add x10, x10, 1
  b loop


find_max:
  eor x4, x4, x4

loop1:
  cmp x2, 0
  b.eq ret_label

  ldr x6, [x0]
  cmp x3, x6
  b.le els

  ldr x6, [x1]
  cmp x6, x4
  b.le els

  mov x4, x6

els:
  sub x2, x2, 1
  add x1, x1, 8
  add x0, x0, 8

  b loop1
  

find_real_max:
  eor x9, x9, x9

loop2:
  cmp x2, 0
  b.eq end

  ldr x8, [x1]
  cmp x8, x9
  b.le else

  mov x9, x8

else:
  add x1, x1, 8
  sub x2, x2, 1
  b loop2


end1:
  mov x1, x12
  mov x2, x11

  b find_real_max

end:
  mov x0, x9
  ret

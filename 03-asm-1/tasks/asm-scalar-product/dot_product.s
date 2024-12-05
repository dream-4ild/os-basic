    .intel_syntax noprefix

    .text
    .global dot_product

dot_product:
    vxorps ymm0, ymm0, ymm0


loop_8:
    cmp rdi, 4
    jl extract_from_8

    vmovups xmm1, [rsi]
    vmovups xmm2, [rdx]
    
    vmulps xmm1, xmm1, xmm2
    vaddps xmm0, xmm0, xmm1
    
    add rsi, 16
    add rdx, 16

    sub rdi, 4

    jmp loop_8

extract_from_8:
    vextractf128 xmm1, ymm0, 1

    vaddps xmm0, xmm0, xmm1

    vhaddps xmm0, xmm0, xmm0
    vhaddps xmm0, xmm0, xmm0


loop_1:
    cmp rdi, 0
    je end

    movss xmm1, [rsi]
    movss xmm2, [rdx]
    
    mulss xmm1, xmm2
    addss xmm0, xmm1
    
    add rsi, 4
    add rdx, 4

    dec rdi

    jmp loop_1

end:
    ret

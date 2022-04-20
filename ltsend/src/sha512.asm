public sha512_compress

.code
sha512_compress:

movq   xmm0, r10
movq   xmm1, r11
movq   xmm2, r12
movq   xmm3, r13
movq   xmm4, r14
movq   xmm5, r15
movq   xmm6, rbx
sub    rsp, 80h
mov    r8, QWORD PTR [rsi]
mov    r9, QWORD PTR [rsi+8h]
mov    r10, QWORD PTR [rsi+10h]
mov    r11, QWORD PTR [rsi+18h]
mov    r12, QWORD PTR [rsi+20h]
mov    r13, QWORD PTR [rsi+28h]
mov    r14, QWORD PTR [rsi+30h]
mov    r15, QWORD PTR [rsi+38h]
mov    rbx, QWORD PTR [rdi]
bswap  rbx
mov    QWORD PTR [rsp], rbx
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r15, rbx
mov    rcx, r14
xor    rcx, r13
and    rcx, r12
xor    rcx, r14
add    r15, rax
mov    rax, 428a2f98d728ae22h

add    r15, rcx
add    r15, rax
add    r11, r15
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r10
add    r15, rax
mov    rax, r10
or     rax, r9
and    rcx, r9
and    rax, r8
or     rax, rcx
add    r15, rax
mov    rbx, QWORD PTR [rdi+8h]
bswap  rbx
mov    QWORD PTR [rsp+8h], rbx
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r14, rbx
mov    rcx, r13
xor    rcx, r12
and    rcx, r11
xor    rcx, r13
add    r14, rax
mov    rax, 7137449123ef65cdh

add    r14, rcx
add    r14, rax
add    r10, r14
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r9
add    r14, rax
mov    rax, r9
or     rax, r8
and    rcx, r8
and    rax, r15
or     rax, rcx
add    r14, rax
mov    rbx, QWORD PTR [rdi+10h]
bswap  rbx
mov    QWORD PTR [rsp+10h], rbx
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r13, rbx
mov    rcx, r12
xor    rcx, r11
and    rcx, r10
xor    rcx, r12
add    r13, rax
mov    rax, 0b5c0fbcfec4d3b2fh

add    r13, rcx
add    r13, rax
add    r9, r13
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r8
add    r13, rax
mov    rax, r8
or     rax, r15
and    rcx, r15
and    rax, r14
or     rax, rcx
add    r13, rax
mov    rbx, QWORD PTR [rdi+18h]
bswap  rbx
mov    QWORD PTR [rsp+18h], rbx
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r12, rbx
mov    rcx, r11
xor    rcx, r10
and    rcx, r9
xor    rcx, r11
add    r12, rax
mov    rax, 0e9b5dba58189dbbch

add    r12, rcx
add    r12, rax
add    r8, r12
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r15
add    r12, rax
mov    rax, r15
or     rax, r14
and    rcx, r14
and    rax, r13
or     rax, rcx
add    r12, rax
mov    rbx, QWORD PTR [rdi+20h]
bswap  rbx
mov    QWORD PTR [rsp+20h], rbx
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r11, rbx
mov    rcx, r10
xor    rcx, r9
and    rcx, r8
xor    rcx, r10
add    r11, rax
mov    rax, 3956c25bf348b538h

add    r11, rcx
add    r11, rax
add    r15, r11
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r14
add    r11, rax
mov    rax, r14
or     rax, r13
and    rcx, r13
and    rax, r12
or     rax, rcx
add    r11, rax
mov    rbx, QWORD PTR [rdi+28h]
bswap  rbx
mov    QWORD PTR [rsp+28h], rbx
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r10, rbx
mov    rcx, r9
xor    rcx, r8
and    rcx, r15
xor    rcx, r9
add    r10, rax
mov    rax, 59f111f1b605d019h

add    r10, rcx
add    r10, rax
add    r14, r10
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r13
add    r10, rax
mov    rax, r13
or     rax, r12
and    rcx, r12
and    rax, r11
or     rax, rcx
add    r10, rax
mov    rbx, QWORD PTR [rdi+30h]
bswap  rbx
mov    QWORD PTR [rsp+30h], rbx
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r9, rbx
mov    rcx, r8
xor    rcx, r15
and    rcx, r14
xor    rcx, r8
add    r9, rax
mov    rax, 923f82a4af194f9bh

add    r9, rcx
add    r9, rax
add    r13, r9
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r12
add    r9, rax
mov    rax, r12
or     rax, r11
and    rcx, r11
and    rax, r10
or     rax, rcx
add    r9, rax
mov    rbx, QWORD PTR [rdi+38h]
bswap  rbx
mov    QWORD PTR [rsp+38h], rbx
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r8, rbx
mov    rcx, r15
xor    rcx, r14
and    rcx, r13
xor    rcx, r15
add    r8, rax
mov    rax, 0ab1c5ed5da6d8118h

add    r8, rcx
add    r8, rax
add    r12, r8
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r11
add    r8, rax
mov    rax, r11
or     rax, r10
and    rcx, r10
and    rax, r9
or     rax, rcx
add    r8, rax
mov    rbx, QWORD PTR [rdi+40h]
bswap  rbx
mov    QWORD PTR [rsp+40h], rbx
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r15, rbx
mov    rcx, r14
xor    rcx, r13
and    rcx, r12
xor    rcx, r14
add    r15, rax
mov    rax, 0d807aa98a3030242h

add    r15, rcx
add    r15, rax
add    r11, r15
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r10
add    r15, rax
mov    rax, r10
or     rax, r9
and    rcx, r9
and    rax, r8
or     rax, rcx
add    r15, rax
mov    rbx, QWORD PTR [rdi+48h]
bswap  rbx
mov    QWORD PTR [rsp+48h], rbx
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r14, rbx
mov    rcx, r13
xor    rcx, r12
and    rcx, r11
xor    rcx, r13
add    r14, rax
mov    rax, 12835b0145706fbeh

add    r14, rcx
add    r14, rax
add    r10, r14
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r9
add    r14, rax
mov    rax, r9
or     rax, r8
and    rcx, r8
and    rax, r15
or     rax, rcx
add    r14, rax
mov    rbx, QWORD PTR [rdi+50h]
bswap  rbx
mov    QWORD PTR [rsp+50h], rbx
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r13, rbx
mov    rcx, r12
xor    rcx, r11
and    rcx, r10
xor    rcx, r12
add    r13, rax
mov    rax, 243185be4ee4b28ch

add    r13, rcx
add    r13, rax
add    r9, r13
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r8
add    r13, rax
mov    rax, r8
or     rax, r15
and    rcx, r15
and    rax, r14
or     rax, rcx
add    r13, rax
mov    rbx, QWORD PTR [rdi+58h]
bswap  rbx
mov    QWORD PTR [rsp+58h], rbx
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r12, rbx
mov    rcx, r11
xor    rcx, r10
and    rcx, r9
xor    rcx, r11
add    r12, rax
mov    rax, 550c7dc3d5ffb4e2h

add    r12, rcx
add    r12, rax
add    r8, r12
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r15
add    r12, rax
mov    rax, r15
or     rax, r14
and    rcx, r14
and    rax, r13
or     rax, rcx
add    r12, rax
mov    rbx, QWORD PTR [rdi+60h]
bswap  rbx
mov    QWORD PTR [rsp+60h], rbx
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r11, rbx
mov    rcx, r10
xor    rcx, r9
and    rcx, r8
xor    rcx, r10
add    r11, rax
mov    rax, 72be5d74f27b896fh

add    r11, rcx
add    r11, rax
add    r15, r11
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r14
add    r11, rax
mov    rax, r14
or     rax, r13
and    rcx, r13
and    rax, r12
or     rax, rcx
add    r11, rax
mov    rbx, QWORD PTR [rdi+68h]
bswap  rbx
mov    QWORD PTR [rsp+68h], rbx
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r10, rbx
mov    rcx, r9
xor    rcx, r8
and    rcx, r15
xor    rcx, r9
add    r10, rax
mov    rax, 80deb1fe3b1696b1h

add    r10, rcx
add    r10, rax
add    r14, r10
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r13
add    r10, rax
mov    rax, r13
or     rax, r12
and    rcx, r12
and    rax, r11
or     rax, rcx
add    r10, rax
mov    rbx, QWORD PTR [rdi+70h]
bswap  rbx
mov    QWORD PTR [rsp+70h], rbx
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r9, rbx
mov    rcx, r8
xor    rcx, r15
and    rcx, r14
xor    rcx, r8
add    r9, rax
mov    rax, 9bdc06a725c71235h

add    r9, rcx
add    r9, rax
add    r13, r9
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r12
add    r9, rax
mov    rax, r12
or     rax, r11
and    rcx, r11
and    rax, r10
or     rax, rcx
add    r9, rax
mov    rbx, QWORD PTR [rdi+78h]
bswap  rbx
mov    QWORD PTR [rsp+78h], rbx
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r8, rbx
mov    rcx, r15
xor    rcx, r14
and    rcx, r13
xor    rcx, r15
add    r8, rax
mov    rax, 0c19bf174cf692694h

add    r8, rcx
add    r8, rax
add    r12, r8
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r11
add    r8, rax
mov    rax, r11
or     rax, r10
and    rcx, r10
and    rax, r9
or     rax, rcx
add    r8, rax
mov    rax, QWORD PTR [rsp+8h]
mov    rbx, QWORD PTR [rsp]
add    rbx, QWORD PTR [rsp+48h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+70h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp], rbx
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r15, rbx
mov    rcx, r14
xor    rcx, r13
and    rcx, r12
xor    rcx, r14
add    r15, rax
mov    rax, 0e49b69c19ef14ad2h

add    r15, rcx
add    r15, rax
add    r11, r15
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r10
add    r15, rax
mov    rax, r10
or     rax, r9
and    rcx, r9
and    rax, r8
or     rax, rcx
add    r15, rax
mov    rax, QWORD PTR [rsp+10h]
mov    rbx, QWORD PTR [rsp+8h]
add    rbx, QWORD PTR [rsp+50h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+78h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+8h], rbx
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r14, rbx
mov    rcx, r13
xor    rcx, r12
and    rcx, r11
xor    rcx, r13
add    r14, rax
mov    rax, 0efbe4786384f25e3h

add    r14, rcx
add    r14, rax
add    r10, r14
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r9
add    r14, rax
mov    rax, r9
or     rax, r8
and    rcx, r8
and    rax, r15
or     rax, rcx
add    r14, rax
mov    rax, QWORD PTR [rsp+18h]
mov    rbx, QWORD PTR [rsp+10h]
add    rbx, QWORD PTR [rsp+58h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+10h], rbx
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r13, rbx
mov    rcx, r12
xor    rcx, r11
and    rcx, r10
xor    rcx, r12
add    r13, rax
mov    rax, 0fc19dc68b8cd5b5h

add    r13, rcx
add    r13, rax
add    r9, r13
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r8
add    r13, rax
mov    rax, r8
or     rax, r15
and    rcx, r15
and    rax, r14
or     rax, rcx
add    r13, rax
mov    rax, QWORD PTR [rsp+20h]
mov    rbx, QWORD PTR [rsp+18h]
add    rbx, QWORD PTR [rsp+60h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+8h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+18h], rbx
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r12, rbx
mov    rcx, r11
xor    rcx, r10
and    rcx, r9
xor    rcx, r11
add    r12, rax
mov    rax, 0240ca1cc77ac9c65h

add    r12, rcx
add    r12, rax
add    r8, r12
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r15
add    r12, rax
mov    rax, r15
or     rax, r14
and    rcx, r14
and    rax, r13
or     rax, rcx
add    r12, rax
mov    rax, QWORD PTR [rsp+28h]
mov    rbx, QWORD PTR [rsp+20h]
add    rbx, QWORD PTR [rsp+68h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+10h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+20h], rbx
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r11, rbx
mov    rcx, r10
xor    rcx, r9
and    rcx, r8
xor    rcx, r10
add    r11, rax
mov    rax, 2de92c6f592b0275h

add    r11, rcx
add    r11, rax
add    r15, r11
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r14
add    r11, rax
mov    rax, r14
or     rax, r13
and    rcx, r13
and    rax, r12
or     rax, rcx
add    r11, rax
mov    rax, QWORD PTR [rsp+30h]
mov    rbx, QWORD PTR [rsp+28h]
add    rbx, QWORD PTR [rsp+70h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+18h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+28h], rbx
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r10, rbx
mov    rcx, r9
xor    rcx, r8
and    rcx, r15
xor    rcx, r9
add    r10, rax
mov    rax, 4a7484aa6ea6e483h

add    r10, rcx
add    r10, rax
add    r14, r10
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r13
add    r10, rax
mov    rax, r13
or     rax, r12
and    rcx, r12
and    rax, r11
or     rax, rcx
add    r10, rax
mov    rax, QWORD PTR [rsp+38h]
mov    rbx, QWORD PTR [rsp+30h]
add    rbx, QWORD PTR [rsp+78h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+20h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+30h], rbx
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r9, rbx
mov    rcx, r8
xor    rcx, r15
and    rcx, r14
xor    rcx, r8
add    r9, rax
mov    rax, 5cb0a9dcbd41fbd4h

add    r9, rcx
add    r9, rax
add    r13, r9
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r12
add    r9, rax
mov    rax, r12
or     rax, r11
and    rcx, r11
and    rax, r10
or     rax, rcx
add    r9, rax
mov    rax, QWORD PTR [rsp+40h]
mov    rbx, QWORD PTR [rsp+38h]
add    rbx, QWORD PTR [rsp]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+28h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+38h], rbx
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r8, rbx
mov    rcx, r15
xor    rcx, r14
and    rcx, r13
xor    rcx, r15
add    r8, rax
mov    rax, 76f988da831153b5h

add    r8, rcx
add    r8, rax
add    r12, r8
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r11
add    r8, rax
mov    rax, r11
or     rax, r10
and    rcx, r10
and    rax, r9
or     rax, rcx
add    r8, rax
mov    rax, QWORD PTR [rsp+48h]
mov    rbx, QWORD PTR [rsp+40h]
add    rbx, QWORD PTR [rsp+8h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+30h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+40h], rbx
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r15, rbx
mov    rcx, r14
xor    rcx, r13
and    rcx, r12
xor    rcx, r14
add    r15, rax
mov    rax, 983e5152ee66dfabh

add    r15, rcx
add    r15, rax
add    r11, r15
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r10
add    r15, rax
mov    rax, r10
or     rax, r9
and    rcx, r9
and    rax, r8
or     rax, rcx
add    r15, rax
mov    rax, QWORD PTR [rsp+50h]
mov    rbx, QWORD PTR [rsp+48h]
add    rbx, QWORD PTR [rsp+10h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+38h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+48h], rbx
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r14, rbx
mov    rcx, r13
xor    rcx, r12
and    rcx, r11
xor    rcx, r13
add    r14, rax
mov    rax, 0a831c66d2db43210h

add    r14, rcx
add    r14, rax
add    r10, r14
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r9
add    r14, rax
mov    rax, r9
or     rax, r8
and    rcx, r8
and    rax, r15
or     rax, rcx
add    r14, rax
mov    rax, QWORD PTR [rsp+58h]
mov    rbx, QWORD PTR [rsp+50h]
add    rbx, QWORD PTR [rsp+18h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+40h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+50h], rbx
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r13, rbx
mov    rcx, r12
xor    rcx, r11
and    rcx, r10
xor    rcx, r12
add    r13, rax
mov    rax, 0b00327c898fb213fh

add    r13, rcx
add    r13, rax
add    r9, r13
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r8
add    r13, rax
mov    rax, r8
or     rax, r15
and    rcx, r15
and    rax, r14
or     rax, rcx
add    r13, rax
mov    rax, QWORD PTR [rsp+60h]
mov    rbx, QWORD PTR [rsp+58h]
add    rbx, QWORD PTR [rsp+20h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+48h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+58h], rbx
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r12, rbx
mov    rcx, r11
xor    rcx, r10
and    rcx, r9
xor    rcx, r11
add    r12, rax
mov    rax, 0bf597fc7beef0ee4h

add    r12, rcx
add    r12, rax
add    r8, r12
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r15
add    r12, rax
mov    rax, r15
or     rax, r14
and    rcx, r14
and    rax, r13
or     rax, rcx
add    r12, rax
mov    rax, QWORD PTR [rsp+68h]
mov    rbx, QWORD PTR [rsp+60h]
add    rbx, QWORD PTR [rsp+28h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+50h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+60h], rbx
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r11, rbx
mov    rcx, r10
xor    rcx, r9
and    rcx, r8
xor    rcx, r10
add    r11, rax
mov    rax, 0c6e00bf33da88fc2h

add    r11, rcx
add    r11, rax
add    r15, r11
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r14
add    r11, rax
mov    rax, r14
or     rax, r13
and    rcx, r13
and    rax, r12
or     rax, rcx
add    r11, rax
mov    rax, QWORD PTR [rsp+70h]
mov    rbx, QWORD PTR [rsp+68h]
add    rbx, QWORD PTR [rsp+30h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+58h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+68h], rbx
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r10, rbx
mov    rcx, r9
xor    rcx, r8
and    rcx, r15
xor    rcx, r9
add    r10, rax
mov    rax, 0d5a79147930aa725h

add    r10, rcx
add    r10, rax
add    r14, r10
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r13
add    r10, rax
mov    rax, r13
or     rax, r12
and    rcx, r12
and    rax, r11
or     rax, rcx
add    r10, rax
mov    rax, QWORD PTR [rsp+78h]
mov    rbx, QWORD PTR [rsp+70h]
add    rbx, QWORD PTR [rsp+38h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+60h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+70h], rbx
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r9, rbx
mov    rcx, r8
xor    rcx, r15
and    rcx, r14
xor    rcx, r8
add    r9, rax
mov    rax, 6ca6351e003826fh

add    r9, rcx
add    r9, rax
add    r13, r9
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r12
add    r9, rax
mov    rax, r12
or     rax, r11
and    rcx, r11
and    rax, r10
or     rax, rcx
add    r9, rax
mov    rax, QWORD PTR [rsp]
mov    rbx, QWORD PTR [rsp+78h]
add    rbx, QWORD PTR [rsp+40h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+68h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+78h], rbx
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r8, rbx
mov    rcx, r15
xor    rcx, r14
and    rcx, r13
xor    rcx, r15
add    r8, rax
mov    rax, 142929670a0e6e70h

add    r8, rcx
add    r8, rax
add    r12, r8
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r11
add    r8, rax
mov    rax, r11
or     rax, r10
and    rcx, r10
and    rax, r9
or     rax, rcx
add    r8, rax
mov    rax, QWORD PTR [rsp+8h]
mov    rbx, QWORD PTR [rsp]
add    rbx, QWORD PTR [rsp+48h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+70h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp], rbx
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r15, rbx
mov    rcx, r14
xor    rcx, r13
and    rcx, r12
xor    rcx, r14
add    r15, rax
mov    rax, 27b70a8546d22ffch

add    r15, rcx
add    r15, rax
add    r11, r15
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r10
add    r15, rax
mov    rax, r10
or     rax, r9
and    rcx, r9
and    rax, r8
or     rax, rcx
add    r15, rax
mov    rax, QWORD PTR [rsp+10h]
mov    rbx, QWORD PTR [rsp+8h]
add    rbx, QWORD PTR [rsp+50h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+78h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+8h], rbx
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r14, rbx
mov    rcx, r13
xor    rcx, r12
and    rcx, r11
xor    rcx, r13
add    r14, rax
mov    rax, 2e1b21385c26c926h

add    r14, rcx
add    r14, rax
add    r10, r14
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r9
add    r14, rax
mov    rax, r9
or     rax, r8
and    rcx, r8
and    rax, r15
or     rax, rcx
add    r14, rax
mov    rax, QWORD PTR [rsp+18h]
mov    rbx, QWORD PTR [rsp+10h]
add    rbx, QWORD PTR [rsp+58h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+10h], rbx
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r13, rbx
mov    rcx, r12
xor    rcx, r11
and    rcx, r10
xor    rcx, r12
add    r13, rax
mov    rax, 4d2c6dfc5ac42aedh

add    r13, rcx
add    r13, rax
add    r9, r13
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r8
add    r13, rax
mov    rax, r8
or     rax, r15
and    rcx, r15
and    rax, r14
or     rax, rcx
add    r13, rax
mov    rax, QWORD PTR [rsp+20h]
mov    rbx, QWORD PTR [rsp+18h]
add    rbx, QWORD PTR [rsp+60h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+8h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+18h], rbx
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r12, rbx
mov    rcx, r11
xor    rcx, r10
and    rcx, r9
xor    rcx, r11
add    r12, rax
mov    rax, 53380d139d95b3dfh

add    r12, rcx
add    r12, rax
add    r8, r12
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r15
add    r12, rax
mov    rax, r15
or     rax, r14
and    rcx, r14
and    rax, r13
or     rax, rcx
add    r12, rax
mov    rax, QWORD PTR [rsp+28h]
mov    rbx, QWORD PTR [rsp+20h]
add    rbx, QWORD PTR [rsp+68h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+10h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+20h], rbx
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r11, rbx
mov    rcx, r10
xor    rcx, r9
and    rcx, r8
xor    rcx, r10
add    r11, rax
mov    rax, 650a73548baf63deh

add    r11, rcx
add    r11, rax
add    r15, r11
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r14
add    r11, rax
mov    rax, r14
or     rax, r13
and    rcx, r13
and    rax, r12
or     rax, rcx
add    r11, rax
mov    rax, QWORD PTR [rsp+30h]
mov    rbx, QWORD PTR [rsp+28h]
add    rbx, QWORD PTR [rsp+70h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+18h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+28h], rbx
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r10, rbx
mov    rcx, r9
xor    rcx, r8
and    rcx, r15
xor    rcx, r9
add    r10, rax
mov    rax, 766a0abb3c77b2a8h

add    r10, rcx
add    r10, rax
add    r14, r10
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r13
add    r10, rax
mov    rax, r13
or     rax, r12
and    rcx, r12
and    rax, r11
or     rax, rcx
add    r10, rax
mov    rax, QWORD PTR [rsp+38h]
mov    rbx, QWORD PTR [rsp+30h]
add    rbx, QWORD PTR [rsp+78h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+20h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+30h], rbx
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r9, rbx
mov    rcx, r8
xor    rcx, r15
and    rcx, r14
xor    rcx, r8
add    r9, rax
mov    rax, 81c2c92e47edaee6h

add    r9, rcx
add    r9, rax
add    r13, r9
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r12
add    r9, rax
mov    rax, r12
or     rax, r11
and    rcx, r11
and    rax, r10
or     rax, rcx
add    r9, rax
mov    rax, QWORD PTR [rsp+40h]
mov    rbx, QWORD PTR [rsp+38h]
add    rbx, QWORD PTR [rsp]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+28h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+38h], rbx
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r8, rbx
mov    rcx, r15
xor    rcx, r14
and    rcx, r13
xor    rcx, r15
add    r8, rax
mov    rax, 92722c851482353bh

add    r8, rcx
add    r8, rax
add    r12, r8
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r11
add    r8, rax
mov    rax, r11
or     rax, r10
and    rcx, r10
and    rax, r9
or     rax, rcx
add    r8, rax
mov    rax, QWORD PTR [rsp+48h]
mov    rbx, QWORD PTR [rsp+40h]
add    rbx, QWORD PTR [rsp+8h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+30h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+40h], rbx
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r15, rbx
mov    rcx, r14
xor    rcx, r13
and    rcx, r12
xor    rcx, r14
add    r15, rax
mov    rax, 0a2bfe8a14cf10364h

add    r15, rcx
add    r15, rax
add    r11, r15
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r10
add    r15, rax
mov    rax, r10
or     rax, r9
and    rcx, r9
and    rax, r8
or     rax, rcx
add    r15, rax
mov    rax, QWORD PTR [rsp+50h]
mov    rbx, QWORD PTR [rsp+48h]
add    rbx, QWORD PTR [rsp+10h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+38h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+48h], rbx
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r14, rbx
mov    rcx, r13
xor    rcx, r12
and    rcx, r11
xor    rcx, r13
add    r14, rax
mov    rax, 0a81a664bbc423001h

add    r14, rcx
add    r14, rax
add    r10, r14
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r9
add    r14, rax
mov    rax, r9
or     rax, r8
and    rcx, r8
and    rax, r15
or     rax, rcx
add    r14, rax
mov    rax, QWORD PTR [rsp+58h]
mov    rbx, QWORD PTR [rsp+50h]
add    rbx, QWORD PTR [rsp+18h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+40h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+50h], rbx
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r13, rbx
mov    rcx, r12
xor    rcx, r11
and    rcx, r10
xor    rcx, r12
add    r13, rax
mov    rax, 0c24b8b70d0f89791h

add    r13, rcx
add    r13, rax
add    r9, r13
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r8
add    r13, rax
mov    rax, r8
or     rax, r15
and    rcx, r15
and    rax, r14
or     rax, rcx
add    r13, rax
mov    rax, QWORD PTR [rsp+60h]
mov    rbx, QWORD PTR [rsp+58h]
add    rbx, QWORD PTR [rsp+20h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+48h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+58h], rbx
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r12, rbx
mov    rcx, r11
xor    rcx, r10
and    rcx, r9
xor    rcx, r11
add    r12, rax
mov    rax, 0c76c51a30654be30h

add    r12, rcx
add    r12, rax
add    r8, r12
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r15
add    r12, rax
mov    rax, r15
or     rax, r14
and    rcx, r14
and    rax, r13
or     rax, rcx
add    r12, rax
mov    rax, QWORD PTR [rsp+68h]
mov    rbx, QWORD PTR [rsp+60h]
add    rbx, QWORD PTR [rsp+28h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+50h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+60h], rbx
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r11, rbx
mov    rcx, r10
xor    rcx, r9
and    rcx, r8
xor    rcx, r10
add    r11, rax
mov    rax, 0d192e819d6ef5218h

add    r11, rcx
add    r11, rax
add    r15, r11
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r14
add    r11, rax
mov    rax, r14
or     rax, r13
and    rcx, r13
and    rax, r12
or     rax, rcx
add    r11, rax
mov    rax, QWORD PTR [rsp+70h]
mov    rbx, QWORD PTR [rsp+68h]
add    rbx, QWORD PTR [rsp+30h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+58h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+68h], rbx
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r10, rbx
mov    rcx, r9
xor    rcx, r8
and    rcx, r15
xor    rcx, r9
add    r10, rax
mov    rax, 0d69906245565a910h

add    r10, rcx
add    r10, rax
add    r14, r10
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r13
add    r10, rax
mov    rax, r13
or     rax, r12
and    rcx, r12
and    rax, r11
or     rax, rcx
add    r10, rax
mov    rax, QWORD PTR [rsp+78h]
mov    rbx, QWORD PTR [rsp+70h]
add    rbx, QWORD PTR [rsp+38h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+60h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+70h], rbx
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r9, rbx
mov    rcx, r8
xor    rcx, r15
and    rcx, r14
xor    rcx, r8
add    r9, rax
mov    rax, 0f40e35855771202ah

add    r9, rcx
add    r9, rax
add    r13, r9
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r12
add    r9, rax
mov    rax, r12
or     rax, r11
and    rcx, r11
and    rax, r10
or     rax, rcx
add    r9, rax
mov    rax, QWORD PTR [rsp]
mov    rbx, QWORD PTR [rsp+78h]
add    rbx, QWORD PTR [rsp+40h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+68h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+78h], rbx
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r8, rbx
mov    rcx, r15
xor    rcx, r14
and    rcx, r13
xor    rcx, r15
add    r8, rax
mov    rax, 106aa07032bbd1b8h

add    r8, rcx
add    r8, rax
add    r12, r8
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r11
add    r8, rax
mov    rax, r11
or     rax, r10
and    rcx, r10
and    rax, r9
or     rax, rcx
add    r8, rax
mov    rax, QWORD PTR [rsp+8h]
mov    rbx, QWORD PTR [rsp]
add    rbx, QWORD PTR [rsp+48h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+70h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp], rbx
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r15, rbx
mov    rcx, r14
xor    rcx, r13
and    rcx, r12
xor    rcx, r14
add    r15, rax
mov    rax, 19a4c116b8d2d0c8h

add    r15, rcx
add    r15, rax
add    r11, r15
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r10
add    r15, rax
mov    rax, r10
or     rax, r9
and    rcx, r9
and    rax, r8
or     rax, rcx
add    r15, rax
mov    rax, QWORD PTR [rsp+10h]
mov    rbx, QWORD PTR [rsp+8h]
add    rbx, QWORD PTR [rsp+50h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+78h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+8h], rbx
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r14, rbx
mov    rcx, r13
xor    rcx, r12
and    rcx, r11
xor    rcx, r13
add    r14, rax
mov    rax, 1e376c085141ab53h

add    r14, rcx
add    r14, rax
add    r10, r14
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r9
add    r14, rax
mov    rax, r9
or     rax, r8
and    rcx, r8
and    rax, r15
or     rax, rcx
add    r14, rax
mov    rax, QWORD PTR [rsp+18h]
mov    rbx, QWORD PTR [rsp+10h]
add    rbx, QWORD PTR [rsp+58h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+10h], rbx
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r13, rbx
mov    rcx, r12
xor    rcx, r11
and    rcx, r10
xor    rcx, r12
add    r13, rax
mov    rax, 2748774cdf8eeb99h

add    r13, rcx
add    r13, rax
add    r9, r13
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r8
add    r13, rax
mov    rax, r8
or     rax, r15
and    rcx, r15
and    rax, r14
or     rax, rcx
add    r13, rax
mov    rax, QWORD PTR [rsp+20h]
mov    rbx, QWORD PTR [rsp+18h]
add    rbx, QWORD PTR [rsp+60h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+8h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+18h], rbx
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r12, rbx
mov    rcx, r11
xor    rcx, r10
and    rcx, r9
xor    rcx, r11
add    r12, rax
mov    rax, 34b0bcb5e19b48a8h

add    r12, rcx
add    r12, rax
add    r8, r12
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r15
add    r12, rax
mov    rax, r15
or     rax, r14
and    rcx, r14
and    rax, r13
or     rax, rcx
add    r12, rax
mov    rax, QWORD PTR [rsp+28h]
mov    rbx, QWORD PTR [rsp+20h]
add    rbx, QWORD PTR [rsp+68h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+10h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+20h], rbx
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r11, rbx
mov    rcx, r10
xor    rcx, r9
and    rcx, r8
xor    rcx, r10
add    r11, rax
mov    rax, 391c0cb3c5c95a63h

add    r11, rcx
add    r11, rax
add    r15, r11
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r14
add    r11, rax
mov    rax, r14
or     rax, r13
and    rcx, r13
and    rax, r12
or     rax, rcx
add    r11, rax
mov    rax, QWORD PTR [rsp+30h]
mov    rbx, QWORD PTR [rsp+28h]
add    rbx, QWORD PTR [rsp+70h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+18h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+28h], rbx
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r10, rbx
mov    rcx, r9
xor    rcx, r8
and    rcx, r15
xor    rcx, r9
add    r10, rax
mov    rax, 4ed8aa4ae3418acbh

add    r10, rcx
add    r10, rax
add    r14, r10
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r13
add    r10, rax
mov    rax, r13
or     rax, r12
and    rcx, r12
and    rax, r11
or     rax, rcx
add    r10, rax
mov    rax, QWORD PTR [rsp+38h]
mov    rbx, QWORD PTR [rsp+30h]
add    rbx, QWORD PTR [rsp+78h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+20h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+30h], rbx
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r9, rbx
mov    rcx, r8
xor    rcx, r15
and    rcx, r14
xor    rcx, r8
add    r9, rax
mov    rax, 5b9cca4f7763e373h

add    r9, rcx
add    r9, rax
add    r13, r9
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r12
add    r9, rax
mov    rax, r12
or     rax, r11
and    rcx, r11
and    rax, r10
or     rax, rcx
add    r9, rax
mov    rax, QWORD PTR [rsp+40h]
mov    rbx, QWORD PTR [rsp+38h]
add    rbx, QWORD PTR [rsp]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+28h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+38h], rbx
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r8, rbx
mov    rcx, r15
xor    rcx, r14
and    rcx, r13
xor    rcx, r15
add    r8, rax
mov    rax, 682e6ff3d6b2b8a3h

add    r8, rcx
add    r8, rax
add    r12, r8
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r11
add    r8, rax
mov    rax, r11
or     rax, r10
and    rcx, r10
and    rax, r9
or     rax, rcx
add    r8, rax
mov    rax, QWORD PTR [rsp+48h]
mov    rbx, QWORD PTR [rsp+40h]
add    rbx, QWORD PTR [rsp+8h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+30h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+40h], rbx
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r15, rbx
mov    rcx, r14
xor    rcx, r13
and    rcx, r12
xor    rcx, r14
add    r15, rax
mov    rax, 748f82ee5defb2fch

add    r15, rcx
add    r15, rax
add    r11, r15
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r10
add    r15, rax
mov    rax, r10
or     rax, r9
and    rcx, r9
and    rax, r8
or     rax, rcx
add    r15, rax
mov    rax, QWORD PTR [rsp+50h]
mov    rbx, QWORD PTR [rsp+48h]
add    rbx, QWORD PTR [rsp+10h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+38h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+48h], rbx
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r14, rbx
mov    rcx, r13
xor    rcx, r12
and    rcx, r11
xor    rcx, r13
add    r14, rax
mov    rax, 78a5636f43172f60h

add    r14, rcx
add    r14, rax
add    r10, r14
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r9
add    r14, rax
mov    rax, r9
or     rax, r8
and    rcx, r8
and    rax, r15
or     rax, rcx
add    r14, rax
mov    rax, QWORD PTR [rsp+58h]
mov    rbx, QWORD PTR [rsp+50h]
add    rbx, QWORD PTR [rsp+18h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+40h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+50h], rbx
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r13, rbx
mov    rcx, r12
xor    rcx, r11
and    rcx, r10
xor    rcx, r12
add    r13, rax
mov    rax, 84c87814a1f0ab72h

add    r13, rcx
add    r13, rax
add    r9, r13
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r8
add    r13, rax
mov    rax, r8
or     rax, r15
and    rcx, r15
and    rax, r14
or     rax, rcx
add    r13, rax
mov    rax, QWORD PTR [rsp+60h]
mov    rbx, QWORD PTR [rsp+58h]
add    rbx, QWORD PTR [rsp+20h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+48h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+58h], rbx
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r12, rbx
mov    rcx, r11
xor    rcx, r10
and    rcx, r9
xor    rcx, r11
add    r12, rax
mov    rax, 8cc702081a6439ech

add    r12, rcx
add    r12, rax
add    r8, r12
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r15
add    r12, rax
mov    rax, r15
or     rax, r14
and    rcx, r14
and    rax, r13
or     rax, rcx
add    r12, rax
mov    rax, QWORD PTR [rsp+68h]
mov    rbx, QWORD PTR [rsp+60h]
add    rbx, QWORD PTR [rsp+28h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+50h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+60h], rbx
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r11, rbx
mov    rcx, r10
xor    rcx, r9
and    rcx, r8
xor    rcx, r10
add    r11, rax
mov    rax, 90befffa23631e28h

add    r11, rcx
add    r11, rax
add    r15, r11
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r14
add    r11, rax
mov    rax, r14
or     rax, r13
and    rcx, r13
and    rax, r12
or     rax, rcx
add    r11, rax
mov    rax, QWORD PTR [rsp+70h]
mov    rbx, QWORD PTR [rsp+68h]
add    rbx, QWORD PTR [rsp+30h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+58h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+68h], rbx
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r10, rbx
mov    rcx, r9
xor    rcx, r8
and    rcx, r15
xor    rcx, r9
add    r10, rax
mov    rax, 0a4506cebde82bde9h

add    r10, rcx
add    r10, rax
add    r14, r10
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r13
add    r10, rax
mov    rax, r13
or     rax, r12
and    rcx, r12
and    rax, r11
or     rax, rcx
add    r10, rax
mov    rax, QWORD PTR [rsp+78h]
mov    rbx, QWORD PTR [rsp+70h]
add    rbx, QWORD PTR [rsp+38h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+60h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+70h], rbx
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r9, rbx
mov    rcx, r8
xor    rcx, r15
and    rcx, r14
xor    rcx, r8
add    r9, rax
mov    rax, 0bef9a3f7b2c67915h

add    r9, rcx
add    r9, rax
add    r13, r9
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r12
add    r9, rax
mov    rax, r12
or     rax, r11
and    rcx, r11
and    rax, r10
or     rax, rcx
add    r9, rax
mov    rax, QWORD PTR [rsp]
mov    rbx, QWORD PTR [rsp+78h]
add    rbx, QWORD PTR [rsp+40h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+68h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+78h], rbx
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r8, rbx
mov    rcx, r15
xor    rcx, r14
and    rcx, r13
xor    rcx, r15
add    r8, rax
mov    rax, 0c67178f2e372532bh

add    r8, rcx
add    r8, rax
add    r12, r8
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r11
add    r8, rax
mov    rax, r11
or     rax, r10
and    rcx, r10
and    rax, r9
or     rax, rcx
add    r8, rax
mov    rax, QWORD PTR [rsp+8h]
mov    rbx, QWORD PTR [rsp]
add    rbx, QWORD PTR [rsp+48h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+70h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp], rbx
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r15, rbx
mov    rcx, r14
xor    rcx, r13
and    rcx, r12
xor    rcx, r14
add    r15, rax
mov    rax, 0ca273eceea26619ch

add    r15, rcx
add    r15, rax
add    r11, r15
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r10
add    r15, rax
mov    rax, r10
or     rax, r9
and    rcx, r9
and    rax, r8
or     rax, rcx
add    r15, rax
mov    rax, QWORD PTR [rsp+10h]
mov    rbx, QWORD PTR [rsp+8h]
add    rbx, QWORD PTR [rsp+50h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+78h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+8h], rbx
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r14, rbx
mov    rcx, r13
xor    rcx, r12
and    rcx, r11
xor    rcx, r13
add    r14, rax
mov    rax, 0d186b8c721c0c207h

add    r14, rcx
add    r14, rax
add    r10, r14
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r9
add    r14, rax
mov    rax, r9
or     rax, r8
and    rcx, r8
and    rax, r15
or     rax, rcx
add    r14, rax
mov    rax, QWORD PTR [rsp+18h]
mov    rbx, QWORD PTR [rsp+10h]
add    rbx, QWORD PTR [rsp+58h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+10h], rbx
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r13, rbx
mov    rcx, r12
xor    rcx, r11
and    rcx, r10
xor    rcx, r12
add    r13, rax
mov    rax, 0eada7dd6cde0eb1eh

add    r13, rcx
add    r13, rax
add    r9, r13
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r8
add    r13, rax
mov    rax, r8
or     rax, r15
and    rcx, r15
and    rax, r14
or     rax, rcx
add    r13, rax
mov    rax, QWORD PTR [rsp+20h]
mov    rbx, QWORD PTR [rsp+18h]
add    rbx, QWORD PTR [rsp+60h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+8h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+18h], rbx
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r12, rbx
mov    rcx, r11
xor    rcx, r10
and    rcx, r9
xor    rcx, r11
add    r12, rax
mov    rax, 0f57d4f7fee6ed178h

add    r12, rcx
add    r12, rax
add    r8, r12
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r15
add    r12, rax
mov    rax, r15
or     rax, r14
and    rcx, r14
and    rax, r13
or     rax, rcx
add    r12, rax
mov    rax, QWORD PTR [rsp+28h]
mov    rbx, QWORD PTR [rsp+20h]
add    rbx, QWORD PTR [rsp+68h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+10h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+20h], rbx
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r11, rbx
mov    rcx, r10
xor    rcx, r9
and    rcx, r8
xor    rcx, r10
add    r11, rax
mov    rax, 6f067aa72176fbah

add    r11, rcx
add    r11, rax
add    r15, r11
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r14
add    r11, rax
mov    rax, r14
or     rax, r13
and    rcx, r13
and    rax, r12
or     rax, rcx
add    r11, rax
mov    rax, QWORD PTR [rsp+30h]
mov    rbx, QWORD PTR [rsp+28h]
add    rbx, QWORD PTR [rsp+70h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+18h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+28h], rbx
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r10, rbx
mov    rcx, r9
xor    rcx, r8
and    rcx, r15
xor    rcx, r9
add    r10, rax
mov    rax, 0a637dc5a2c898a6h

add    r10, rcx
add    r10, rax
add    r14, r10
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r13
add    r10, rax
mov    rax, r13
or     rax, r12
and    rcx, r12
and    rax, r11
or     rax, rcx
add    r10, rax
mov    rax, QWORD PTR [rsp+38h]
mov    rbx, QWORD PTR [rsp+30h]
add    rbx, QWORD PTR [rsp+78h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+20h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+30h], rbx
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r9, rbx
mov    rcx, r8
xor    rcx, r15
and    rcx, r14
xor    rcx, r8
add    r9, rax
mov    rax, 113f9804bef90daeh

add    r9, rcx
add    r9, rax
add    r13, r9
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r12
add    r9, rax
mov    rax, r12
or     rax, r11
and    rcx, r11
and    rax, r10
or     rax, rcx
add    r9, rax
mov    rax, QWORD PTR [rsp+40h]
mov    rbx, QWORD PTR [rsp+38h]
add    rbx, QWORD PTR [rsp]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+28h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+38h], rbx
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r8, rbx
mov    rcx, r15
xor    rcx, r14
and    rcx, r13
xor    rcx, r15
add    r8, rax
mov    rax, 1b710b35131c471bh

add    r8, rcx
add    r8, rax
add    r12, r8
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r11
add    r8, rax
mov    rax, r11
or     rax, r10
and    rcx, r10
and    rax, r9
or     rax, rcx
add    r8, rax
mov    rax, QWORD PTR [rsp+48h]
mov    rbx, QWORD PTR [rsp+40h]
add    rbx, QWORD PTR [rsp+8h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+30h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+40h], rbx
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r15, rbx
mov    rcx, r14
xor    rcx, r13
and    rcx, r12
xor    rcx, r14
add    r15, rax
mov    rax, 28db77f523047d84h

add    r15, rcx
add    r15, rax
add    r11, r15
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r10
add    r15, rax
mov    rax, r10
or     rax, r9
and    rcx, r9
and    rax, r8
or     rax, rcx
add    r15, rax
mov    rax, QWORD PTR [rsp+50h]
mov    rbx, QWORD PTR [rsp+48h]
add    rbx, QWORD PTR [rsp+10h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+38h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+48h], rbx
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r14, rbx
mov    rcx, r13
xor    rcx, r12
and    rcx, r11
xor    rcx, r13
add    r14, rax
mov    rax, 32caab7b40c72493h

add    r14, rcx
add    r14, rax
add    r10, r14
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r9
add    r14, rax
mov    rax, r9
or     rax, r8
and    rcx, r8
and    rax, r15
or     rax, rcx
add    r14, rax
mov    rax, QWORD PTR [rsp+58h]
mov    rbx, QWORD PTR [rsp+50h]
add    rbx, QWORD PTR [rsp+18h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+40h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+50h], rbx
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r13, rbx
mov    rcx, r12
xor    rcx, r11
and    rcx, r10
xor    rcx, r12
add    r13, rax
mov    rax, 3c9ebe0a15c9bebch

add    r13, rcx
add    r13, rax
add    r9, r13
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r8
add    r13, rax
mov    rax, r8
or     rax, r15
and    rcx, r15
and    rax, r14
or     rax, rcx
add    r13, rax
mov    rax, QWORD PTR [rsp+60h]
mov    rbx, QWORD PTR [rsp+58h]
add    rbx, QWORD PTR [rsp+20h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+48h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+58h], rbx
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r12, rbx
mov    rcx, r11
xor    rcx, r10
and    rcx, r9
xor    rcx, r11
add    r12, rax
mov    rax, 431d67c49c100d4ch

add    r12, rcx
add    r12, rax
add    r8, r12
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r15
add    r12, rax
mov    rax, r15
or     rax, r14
and    rcx, r14
and    rax, r13
or     rax, rcx
add    r12, rax
mov    rax, QWORD PTR [rsp+68h]
mov    rbx, QWORD PTR [rsp+60h]
add    rbx, QWORD PTR [rsp+28h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+50h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+60h], rbx
mov    rcx, r8
mov    rdx, r8
mov    rax, r8
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r11, rbx
mov    rcx, r10
xor    rcx, r9
and    rcx, r8
xor    rcx, r10
add    r11, rax
mov    rax, 4cc5d4becb3e42b6h

add    r11, rcx
add    r11, rax
add    r15, r11
mov    rcx, r12
mov    rdx, r12
mov    rax, r12
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r14
add    r11, rax
mov    rax, r14
or     rax, r13
and    rcx, r13
and    rax, r12
or     rax, rcx
add    r11, rax
mov    rax, QWORD PTR [rsp+70h]
mov    rbx, QWORD PTR [rsp+68h]
add    rbx, QWORD PTR [rsp+30h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+58h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+68h], rbx
mov    rcx, r15
mov    rdx, r15
mov    rax, r15
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r10, rbx
mov    rcx, r9
xor    rcx, r8
and    rcx, r15
xor    rcx, r9
add    r10, rax
mov    rax, 597f299cfc657e2ah

add    r10, rcx
add    r10, rax
add    r14, r10
mov    rcx, r11
mov    rdx, r11
mov    rax, r11
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r13
add    r10, rax
mov    rax, r13
or     rax, r12
and    rcx, r12
and    rax, r11
or     rax, rcx
add    r10, rax
mov    rax, QWORD PTR [rsp+78h]
mov    rbx, QWORD PTR [rsp+70h]
add    rbx, QWORD PTR [rsp+38h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+60h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+70h], rbx
mov    rcx, r14
mov    rdx, r14
mov    rax, r14
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r9, rbx
mov    rcx, r8
xor    rcx, r15
and    rcx, r14
xor    rcx, r8
add    r9, rax
mov    rax, 5fcb6fab3ad6faech

add    r9, rcx
add    r9, rax
add    r13, r9
mov    rcx, r10
mov    rdx, r10
mov    rax, r10
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r12
add    r9, rax
mov    rax, r12
or     rax, r11
and    rcx, r11
and    rax, r10
or     rax, rcx
add    r9, rax
mov    rax, QWORD PTR [rsp]
mov    rbx, QWORD PTR [rsp+78h]
add    rbx, QWORD PTR [rsp+40h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 8h
shr    rdx, 7h
ror    rax, 1
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    rax, QWORD PTR [rsp+68h]
mov    rcx, rax
mov    rdx, rax
ror    rcx, 3dh
shr    rdx, 6h
ror    rax, 13h
xor    rcx, rdx
xor    rax, rcx
add    rbx, rax
mov    QWORD PTR [rsp+78h], rbx
mov    rcx, r13
mov    rdx, r13
mov    rax, r13
ror    rcx, 12h
ror    rdx, 29h
ror    rax, 0eh
xor    rcx, rdx
xor    rax, rcx
add    r8, rbx
mov    rcx, r15
xor    rcx, r14
and    rcx, r13
xor    rcx, r15
add    r8, rax
mov    rax, 6c44198c4a475817h

add    r8, rcx
add    r8, rax
add    r12, r8
mov    rcx, r9
mov    rdx, r9
mov    rax, r9
ror    rcx, 27h
ror    rdx, 22h
ror    rax, 1ch
xor    rcx, rdx
xor    rax, rcx
mov    rcx, r11
add    r8, rax
mov    rax, r11
or     rax, r10
and    rcx, r10
and    rax, r9
or     rax, rcx
add    r8, rax
add    QWORD PTR [rsi], r8
add    QWORD PTR [rsi+8h], r9
add    QWORD PTR [rsi+10h], r10
add    QWORD PTR [rsi+18h], r11
add    QWORD PTR [rsi+20h], r12
add    QWORD PTR [rsi+28h], r13
add    QWORD PTR [rsi+30h], r14
add    QWORD PTR [rsi+38h], r15
movq   r10, xmm0
movq   r11, xmm1
movq   r12, xmm2
movq   r13, xmm3
movq   r14, xmm4
movq   r15, xmm5
movq   rbx, xmm6
add    rsp, 80h
ret

END

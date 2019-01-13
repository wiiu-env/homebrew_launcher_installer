#define BAT_SETUP_HOOK_ADDR         0xFFF1D624
# not all of those NOP address are required for every firmware
# mainly these should stop the kernel from removing our IBAT4 and DBAT5
#define BAT_SET_NOP_ADDR_1          0xFFF06B6C
#define BAT_SET_NOP_ADDR_2          0xFFF06BF8
#define BAT_SET_NOP_ADDR_3          0xFFF003C8
#define BAT_SET_NOP_ADDR_4          0xFFF003CC
#define BAT_SET_NOP_ADDR_5          0xFFF1D70C
#define BAT_SET_NOP_ADDR_6          0xFFF1D728
#define BAT_SET_NOP_ADDR_7          0xFFF1D82C

#define BAT_SET_NOP_ADDR_8          0xFFEE11C4
#define BAT_SET_NOP_ADDR_9          0xFFEE11C8

#define BAT_SETUP_HOOK_ENTRY        0x00800000


#define BAT4U_VAL                   0x008000FF
#define BAT4L_VAL                   0x30800012


#define SET_R4_TO_ADDR(addr)        \
    lis r3, addr@h              ;   \
    ori r3, r3, addr@l          ;   \
    stw r4, 0(r3)               ;   \
    dcbf 0, r3                  ;   \
    icbi 0, r3                  ;

     .globl Syscall_0x36
Syscall_0x36:
    li r0, 0x3600
    sc
    blr
    
    
.global SCKernelCopyData
SCKernelCopyData:
	// Disable data address translation
	mfmsr %r6
	li %r7, 0x10
	andc %r6, %r6, %r7
	mtmsr %r6
	
	// Copy data
	addi %r3, %r3, -1
	addi %r4, %r4, -1
	mtctr %r5
SCKernelCopyData_loop:
	lbzu %r5, 1(%r4)
	stbu %r5, 1(%r3)
	bdnz SCKernelCopyData_loop
	
	// Enable data address translation
	ori %r6, %r6, 0x10
	mtmsr %r6
blr

.global SC_0x25_KernelCopyData
SC_0x25_KernelCopyData:
	li %r0, 0x2500
	sc
blr

    .globl KernelPatches
KernelPatches:
    # store the old DBAT0
    mfdbatu r5, 0
    mfdbatl r6, 0

    # memory barrier
    eieio
    isync

    # setup DBAT0 for access to kernel code memory
    lis r3, 0xFFF0
    ori r3, r3, 0x0002
    mtdbatu 0, r3
    lis r3, 0xFFF0
    ori r3, r3, 0x0032
    mtdbatl 0, r3

    # memory barrier
    eieio
    isync

    # SaveAndResetDataBATs_And_SRs hook setup, but could be any BAT function though
    # just chosen because its simple
    lis r3, BAT_SETUP_HOOK_ADDR@h
    ori r3, r3, BAT_SETUP_HOOK_ADDR@l

    # make the kernel setup our section in IBAT4 and
    # jump to our function to restore the replaced instructions
    lis r4, 0x3ce0      				#   lis r7, BAT4L_VAL@h
    ori r4, r4, BAT4L_VAL@h
    stw r4, 0x00(r3)
    lis r4, 0x60e7      				#   ori r7, r7, BAT4L_VAL@l
    ori r4, r4, BAT4L_VAL@l
    stw r4, 0x04(r3)
    lis r4, 0x7cf1      				#   mtspr 561, r7
    ori r4, r4, 0x8ba6
    stw r4, 0x08(r3)
    lis r4, 0x3ce0      				#   lis r7, BAT4U_VAL@h
    ori r4, r4, BAT4U_VAL@h
    stw r4, 0x0C(r3)
    lis r4, 0x60e7      				#   ori r7, r7, BAT4U_VAL@l
    ori r4, r4, BAT4U_VAL@l
    stw r4, 0x10(r3)
    lis r4, 0x7cf0      				#   mtspr 560, r7
    ori r4, r4, 0x8ba6
    stw r4, 0x14(r3)
    lis r4, 0x7c00      				#   eieio
    ori r4, r4, 0x06ac
    stw r4, 0x18(r3)
    lis r4, 0x4c00      				#   isync
    ori r4, r4, 0x012c
    stw r4, 0x1C(r3)
    lis r4, 0x7ce8     				    #   mflr r7
    ori r4, r4, 0x02a6
    stw r4, 0x20(r3)
    lis r4, (BAT_SETUP_HOOK_ENTRY | 0x48000003)@h      #   bla BAT_SETUP_HOOK_ENTRY
    ori r4, r4, (BAT_SETUP_HOOK_ENTRY | 0x48000003)@l
    stw r4, 0x24(r3)

    # flush and invalidate the replaced instructions
    lis r3, (BAT_SETUP_HOOK_ADDR & ~31)@h
    ori r3, r3, (BAT_SETUP_HOOK_ADDR & ~31)@l
    dcbf 0, r3
    icbi 0, r3
    lis r3, ((BAT_SETUP_HOOK_ADDR + 0x20) & ~31)@h
    ori r3, r3, ((BAT_SETUP_HOOK_ADDR + 0x20) & ~31)@l
    dcbf 0, r3
    icbi 0, r3
    sync

    # setup IBAT4 for core 1 at this position (not really required but wont hurt)
    # IBATL 4
    lis r3, BAT4L_VAL@h
    ori r3, r3, BAT4L_VAL@l
    mtspr 561, r3

    # IBATU 4
    lis r3, BAT4U_VAL@h
    ori r3, r3, BAT4U_VAL@l
    mtspr 560, r3

    # memory barrier
    eieio
    isync

    # write "nop" to some positions
    lis r4, 0x6000
    # nop on IBATU 4 and DBAT 5 set/reset
#ifdef BAT_SET_NOP_ADDR_1
    SET_R4_TO_ADDR(BAT_SET_NOP_ADDR_1)
#endif
#ifdef BAT_SET_NOP_ADDR_2
    SET_R4_TO_ADDR(BAT_SET_NOP_ADDR_2)
#endif
#ifdef BAT_SET_NOP_ADDR_3
    SET_R4_TO_ADDR(BAT_SET_NOP_ADDR_3)
#endif
#ifdef BAT_SET_NOP_ADDR_4
    SET_R4_TO_ADDR(BAT_SET_NOP_ADDR_4)
#endif
#ifdef BAT_SET_NOP_ADDR_5
    SET_R4_TO_ADDR(BAT_SET_NOP_ADDR_5)
#endif
#ifdef BAT_SET_NOP_ADDR_6
    SET_R4_TO_ADDR(BAT_SET_NOP_ADDR_6)
#endif
#ifdef BAT_SET_NOP_ADDR_7
    SET_R4_TO_ADDR(BAT_SET_NOP_ADDR_7)
#endif

#if (defined(BAT_SET_NOP_ADDR_8) && defined(BAT_SET_NOP_ADDR_9))
    # memory barrier
    eieio
    isync

    # setup DBAT0 for access to kernel code memory
    lis r3, 0xFFEE
    ori r3, r3, 0x0002
    mtdbatu 0, r3
    lis r3, 0xFFEE
    ori r3, r3, 0x0032
    mtdbatl 0, r3

    # memory barrier
    eieio
    isync

    # write "nop" to some positions
    lis r4, 0x6000
    SET_R4_TO_ADDR(BAT_SET_NOP_ADDR_8)
    SET_R4_TO_ADDR(BAT_SET_NOP_ADDR_9)
#endif

    # memory barrier
    eieio
    isync

    # restore DBAT 0 and return from interrupt
    mtdbatu 0, r5
    mtdbatl 0, r6

    # memory barrier
    eieio
    isync

    blr


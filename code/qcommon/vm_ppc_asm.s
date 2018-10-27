	.section ".text"
	.align 2
	.sdreg	r2
	.align	4
	.globl _AsmCall

_AsmCall:
	#pop off the destination instruction
	lwz	r12,0(r4)	# RG_TOP, 0(RG_OPSTACK)
	addi	r4,r4,-4	# RG_OPSTACK, RG_OPSTACK, -4 \n"

	# see if it is a system trap
	cmpwi	r12,0		# RG_TOP, 0 \n"
	bc	12,0, .systemTrap

	# calling another VM function, so lookup in instructionPointers
	slwi	r12,r12,2	# RG_TOP,RG_TOP,2

	# FIXME: range check
	lwzx	r12, r8, r12	# RG_TOP, RG_INSTRUCTIONS(RG_TOP)	
	mtctr	r12		# RG_TOP

	bcctr	20,0		# when it hits a leave, it will branch to the current link register

	# calling a system trap

.systemTrap:
	# convert to positive system call number
	subfic	r12,r12,-1

	# save all our registers, including the current link register
	mflr	r13		# RG_SECOND	  copy off our link register
	addi	r1,r1,-92	# required 24 bytes of linkage
				# , 32 bytes of parameter, plus our saves
	stw	r3,56(r1)	# RG_STACK, -36(REAL_STACK)
	stw	r4,60(r1)	# RG_OPSTACK, 4(RG_REAL_STACK)
	stw	r5,64(r1)	# RG_MEMBASE, 8(RG_REAL_STACK)
	stw	r6,68(r1)	# RG_MEMMASK, 12(RG_REAL_STACK)
	stw	r7,72(r1)	# RG_ASMCALL, 16(RG_REAL_STACK)
	stw	r8,76(r1)	# RG_INSTRUCTIONS, 20(RG_REAL_STACK)
	stw	r9,80(r1)	# RG_NUM_INSTRUCTIONS, 24(RG_REAL_STACK)
	stw	r10,84(r1)	# RG_VM, 28(RG_REAL_STACK)
	stw	r13,88(r1)	# RG_SECOND, 32(RG_REAL_STACK)	// link register

	# save the vm stack position to allow recursive VM entry
	addi	r13,r3,-4	# RG_TOP, RG_STACK, -4
	stw	r13,0(r10)	# RG_TOP, VM_OFFSET_PROGRAM_STACK(RG_VM)

	# save the system call number as the 0th parameter
	add	r3,r3,r5	# r3,  RG_STACK, RG_MEMBASE	// r3 is the first parameter to vm->systemCalls
	stwu	r12,4(r3)	# RG_TOP, 4(r3)

	# make the system call with the address of all the VM parms as a parameter
	# vm->systemCalls( &parms )
	lwz	r12,4(r10)	# RG_TOP, VM_OFFSET_SYSTEM_CALL(RG_VM)
	mtctr	r12		# RG_TOP
	bcctrl	20,0

	mr	r12,r3		# RG_TOP, r3

	# pop our saved registers
	lwz	r3,56(r1)	# RG_STACK, 0(RG_REAL_STACK)
	lwz	r4,60(r1)	# RG_OPSTACK, 4(RG_REAL_STACK)
	lwz	r5,64(r1)	# RG_MEMBASE, 8(RG_REAL_STACK)
	lwz	r6,68(r1)	# RG_MEMMASK, 12(RG_REAL_STACK)
	lwz	r7,72(r1)	# RG_ASMCALL, 16(RG_REAL_STACK)
	lwz	r8,76(r1)	# RG_INSTRUCTIONS, 20(RG_REAL_STACK)
	lwz	r9,80(r1)	# RG_NUM_INSTRUCTIONS, 24(RG_REAL_STACK)
	lwz	r10,84(r1)	# RG_VM, 28(RG_REAL_STACK)
	lwz	r13,88(r1)	# RG_SECOND, 32(RG_REAL_STACK)
	addi	r1,r1,92	# RG_REAL_STACK, RG_REAL_STACK, 36

	# restore the old link register
	mtlr	r13		# RG_SECOND

	# save off the return value
	stwu	r12,4(r4)	# RG_TOP, 0(RG_OPSTACK)
	blr

	.type	_AsmCall,@function
	.size	_AsmCall,$-_AsmCall


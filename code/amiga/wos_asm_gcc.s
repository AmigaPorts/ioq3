
	# Checks done for mirrors in R_SetupProjectionZ
	.text
	.align	2
	.global SGN
	.type	SGN, @function
SGN:
	lis	%r9,zero@ha
	lfs	%f5,zero@l(%r9)
	fcmpu	%cr0,%f1,%f5	# if (a == 0.f)
	bne	%cr0,.notzero	# return 0.f;
	#mr	%f1,%f5
	blr

.notzero:

	lis	%r10,one@ha
	lfs	%f2,one@l(%r10)
	lis	%r11,negone@ha
	lfs	%f3,negone@l(%r11)
	fsel	%f1,%f1,%f2,%f3	# return __fsel(a, 1.0f, -1.0f);
	blr

	.size	SGN, .-SGN

######
	#.tocd
	.align	2
	.section	.rodata
	
	.align	2
zero:
	.long	0x00000000

	.align	2
one:
	.long	0x3f800000

	.align	2
negone:
	.long	0xbf800000


	


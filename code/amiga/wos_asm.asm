
	.text
	.align	2
	.sdreg	r2
	.align	4
	.global _rint

_rint:
	#lfs	f13,two23(r2)
	lfd	f13,two23(r2)   # should be double ??
	fabs	f0,f1
	fsubs	f12,f13,f13	# generate 0.0 
	fcmpu	cr7,f0,f13	# if (fabs(x) > TWO23)
	fcmpu	cr6,f1,f12	# if (x > 0.0) 
	bnllr-	cr7
	bng-	cr6, .lessthanzero

	fadds	f1,f1,f13	# x+= TWO23 
	fsubs	f1,f1,f13	# x-= TWO23
	fabs	f1,f1		# if (x == 0.0)
	blr			# x = 0.0; 

.lessthanzero:
	bnllr-	cr6		# if (x < 0.0) 
	fsubs	f1,f1,f13	# x -= TWO23 
	fadds	f1,f1,f13	# x += TWO23 
	fnabs	f1,f1		# if (x == 0.0) 
	blr			# x = -0.0;

	.type	_rint,@function
	.size	_rint,$-_rint


# Checks done for mirrors in R_SetupProjectionZ
	.text
	.align	2
	.sdreg	r2
	.align	4
	.global _SGN

_SGN:
	lfs	f5,zero(r2)
	fcmpu	cr0,f1,f5	# if (a == 0.f)
	bne	cr0,.notzero	# return 0.f;
	#mr	f1,f5
	blr

.notzero:

	lfs	f2,one(r2)
	lfs	f3,negone(r2)
	fsel	f1,f1,f2,f3	# return __fsel(a, 1.0f, -1.0f);
	blr

	.type	_SGN,@function
	.size	_SGN,$-_SGN

######
	.tocd
	.align	2
	.section	.rodata
#	.align	2
	.align  3

two23:
	#.long   0x4B000000
	.long   0x41600000, 0x00000000 	# should be double ?? (8388608.0)

	.align	2
zero:
	.long	0x00000000

	.align	2
one:
	.long	0x3f800000

	.align	2
negone:
	.long	0xbf800000


	


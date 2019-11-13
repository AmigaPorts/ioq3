
	.text
	.align	2
	.sdreg	r2
	.align	4
	.global _fround

# Actually this is roundf

_fround:

	lfs	f13,two23(r2)
	fabs	f0,f1
	fsubs	f12,f13,f13	# generate 0.0 
	fcmpu	cr7,f0,f13	# if (fabs(x) > TWO23)
	#mffs	%f11		# save fpu rounding mode and inexact state
	fcmpu	cr6,f1,f12	# if (x > 0.0) 
	bnl-	cr7,.nan
	#mtfsfi	7,1		# set roundind mode toward 0

	lfs	f10,halfone(r2)

	ble-	cr6,.lessthanzero
	fadds	f1,f1,f10	# x+= 0.5
	fadds	f1,f1,f13	# x+= TWO23
	fsubs	f1,f1,f13	# x-= TWO23
	fabs	f1,f1		# if (x == 0.0) x = 0.0; 

	#mtfsf	0xff,%f11	# restore previous rounding mode
	blr

.lessthanzero:
	fsubs	f9,f1,f10	# x+= 0.5
	bge-	cr6,.L9		# if (x < 0.0) 
	fsubs	f1,f9,f13	# x -= TWO23 
	fadds	f1,f1,f13	# x += TWO23 
	fnabs	f1,f1		# if (x == 0.0)

.L9:	#mtfsf	0xff,%f11	# restore previous rounding mode
	blr			# x = -0.0;

#ensure sNaN input is converted to qNan
.nan:
	fcmpu	cr7,f1,f1
	beqlr	cr7
	fadds	f1,f1,f1
	blr

	.type	_fround,@function
	.size	_fround,$-_fround


# Checks done for mirrors in R_SetupProjectionZ
	.text
	.align	2
	.sdreg	r2
	.align	4
	.global _SGN

_SGN:
	#lfs	f5,zero(r2)
	fsubs	f4,f5,f5
	fcmpu	cr0,f1,f4	# if (a == 0.f)
	bne	cr0,.notzero	# return 0.f;
	#mr	f1,f4
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
	.align	2
two23:
	.long   0x4b000000

	.align	2
halfone:
	.long	0x3f000000

	.align	2
one:
	.long	0x3f800000

	.align	2
negone:
	.long	0xbf800000


	


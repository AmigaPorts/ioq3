
	.text
	.align	2
	.global fround
	.type 	fround, @function

# Actually this is roundf

fround:
	lis	%r9,two23@ha
	lfs	%f13,two23@l(%r9)

	fabs	%f0,%f1
	fsubs	%f12,%f13,%f13	# generate 0.0 
	fcmpu	%cr7,%f0,%f13	# if (fabs(x) > TWO23)
	#mffs	%f11		# save fpu rounding mode and inexact state

	fcmpu	%cr6,%f1,%f12	# if (x > 0.0) 
	bnl-	%cr7,.nan
	#mtfsfi	7,1		# set roundind mode toward 0

	lfs	%f10,halfone@l(%r9)

	ble-	%cr6,.L8
	fadds	%f1,%f1,%f10	# x+= 0.5
	fadds	%f1,%f1,%f13	# x+= TWO23
	fsubs	%f1,%f1,%f13	# x-= TWO23
	fabs	%f1,%f1		# if (x == 0.0) x = 0.0

	#mtfsf	0xff,%f11	# restore previous rounding mode
	blr

.L8:
	fsubs	%f9,%f1,%f10	# x+= 0.5
	bge-	%cr6,.L9	# if (x < 0.0) 
	fsubs	%f1,%f9,%f13	# x -= TWO23 
	fadds	%f1,%f1,%f13	# x += TWO23 
	fnabs	%f1,%f1		# if (x == 0.0) x = -0.0

.L9:	#mtfsf	0xff,%f11	# restore previous rounding mode
	blr

#ensure sNaN input is converted to qNan
.nan:
	fcmpu	%cr7,%f1,%f1
	beqlr	%cr7
	fadds	%f1,%f1,%f1
	blr

	.size	fround, .-fround

.align	2
	.section	.rodata
	
	.align	2
two23:
	.long   0x4b000000

	.align	2
halfone:
	.long   0x3f000000


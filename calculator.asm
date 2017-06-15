.globl main

.data
	op_string:	.asciiz		"\nSelect operation (+, -, *, or /): "
	num1_string:	.asciiz		"\nFirst number: "
	num2_string:	.asciiz		"\nSecond number: "
	ans_string:	.asciiz		"\nanswer = "
	
	str_buffer:	.space		4
	
	plus:		.byte		'+'
	minus:		.byte		'-'
	star:		.byte		'*'
	slash:		.byte		'/'
.text
main:
	li	$v0, 4			# Load print string into $v0
	la	$a0, op_string		# Load 'op_string' address into $a0
	syscall				# Print string
	
	li	$v0, 8			# Load 'read string' into $v0
	la	$a0, str_buffer		# Load address of operator string buffer into $a0
	li	$a1, 4			# Load num chars into $a1
	syscall				# Load user operator string
	lb	$t0, ($a0)		# Load character into $t0
	
	li	$v0, 4			# Load print string into $v0
	la	$a0, num1_string	# Load 'num1_string' address into $a0
	syscall
	
	li	$v0, 5			# Load 'read int' into $v0
	syscall
	move	$t1, $v0		# Move 'int1' into $t1
	
	li	$v0, 4			# Load 'print string' into $v0
	la	$a0, num2_string	# Load 'num2_string' address into $a0
	syscall
	
	li	$v0, 5			# Load 'read int' into $v0
	syscall
	move	$t2, $v0		# Move 'int2' into $t2
	
	lb	$t4, plus		# Load '+' byte into $t4
	beq	$t0, $t4, addition	# Go to 'addittion' subroutine
	lb	$t4, minus		# Load '-' byte into $t4
	beq	$t0, $t4, subtraction	# Go to 'subtraction' subroutine
	lb	$t4, star		# Load '*' byte into $t4
	beq	$t0, $t4, multiply	# Go to 'multiply' subroutine
	lb	$t4, slash		# Load '/' byte into $t4
	beq	$t0, $t4, division	# Go to 'division' subroutine
	
addition:
	add	$t0, $t1, $t2		# Add values in $t1 and $t2
	j	continue
	
subtraction:
	sub	$t0, $t1, $t2		# Subtract $t1 - $t2
	j	continue
	
multiply:
	mul	$t0, $t1, $t2		# Multiply $t1 x $t2
	j	continue
	
division:
	div	$t0, $t1, $t2		# Divide $t1 / $t2
	j	continue
	
continue:
	li	$v0, 4			# Load 'print string' into $v0
	la	$a0, ans_string		# Load 'ans_string' address into $a0
	syscall
	
	li	$v0, 1			# Load 'print int' into $v0
	move	$a0, $t0		# Move result $t0 into $a0
	syscall
	
	
	li	$v0, 10			# Load 'terminate' into $v0
	syscall
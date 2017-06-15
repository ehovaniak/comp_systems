.globl		main

.data
	fib_num_str:	.asciiz		"\nFibonacci sequence to compute: "
	answer_str:	.asciiz		"\nAnswer = "
	
.text
main:
	li	$v0, 4			# Load print string into $v0
	la	$a0, fib_num_str	# Load 'fib_num_str' address into $a0
	syscall				# Print string
	
	li	$v0, 5			# Load 'read int' into $v0
	syscall
	move	$a0, $v0		# Move 'int1' into $t1
	
	jal	fib			# Begin Fibonacci
	move	$t0, $v0		# Return Fibonacci result
	
	li	$v0, 4			# Load 'print string' into $v0
	la	$a0, answer_str		# Load 'answer_str' address into $a0
	syscall				# Print string
	
	li	$v0, 1			# Load 'print int' into $v0
	move	$a0, $t0		# Load Fibonacci result into $a0
	syscall
	
	li	$v0, 10
	syscall
	
fib:
	subi	$sp, $sp, 12		# Push space for stack frame
	sw	$ra, 0($sp)		# store return address for this call
	sw	$s0, 4($sp)		# Store $t0
	sw	$s1, 8($sp)		# Store $t1
	#sw	$t2, 12($sp)		# Store $t2
	
	move	$s0, $a0		# Move the argument value into preserved register $s0
	
	#move	$t1, 1
	beq	$s0, $zero, seq0	# Check if argument is sequence 0
	beq	$s0, 1, seq1		# Check if argument is sequence 1
	
	subi	$a0, $s0, 1		# Decriment the Fib sequence counter
	jal 	fib			# Recursive call
	
	move	$s1, $v0		# result of Fib(n-1)
	
	subi	$a0, $s0, 2		# Devriment the Fib sequence counter	
	jal	fib			# Recursive call
	
	add	$v0, $v0, $s1		# Add the two recursive calls
	
	
return:
	lw	$ra, 0($sp)		# Load the previous call's return address
	lw	$s0, 4($sp)		# Load the stored value $s0
	lw	$s1, 8($sp)		# Load the stored value #s2
	addi	$sp, $sp, 12		# Incriment the stack pointer by 12 bytes
	jr	$ra			# Jump back up the call stack
	
seq1:					# Case when reached sequence 1
	li	$v0, 1
	j	return
seq0:	li	$v0, 0			# Case when reached sequence 1
	j	return

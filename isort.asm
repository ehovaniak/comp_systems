.globl main

.data
hello: 		.asciiz		"Good morning"
len_str:	.asciiz		"How many ints?\n"
array_str:	.asciiz		"Enter that many ints, one per line\n"
orig_str:	.asciiz		"Your array:\n"
sorted_str:	.asciiz		"Sorted array:\n"
new_line:	.asciiz		"\n"
space:		.asciiz		" "

.text
main:
	# $t0 = n
	# $t1 = xs
	# $t2 = ii
	# $t3 = tmp
	# $t4 = ys

	li $v0, 4
	la $a0, hello
	syscall
	
	li $v0, 4
	la $a0, new_line
	syscall
	
	li $v0, 4
	la $a0, len_str
	syscall
	
	li $v0, 5	#read length of array int
	syscall
	move $t0, $v0	#move 'n' into $t0
	
	li $v0, 4
	la $a0, array_str	#print request for array elements
	syscall
	
	move $a0, $t0		#move 'n' into $a0 for 'read_ints'
	jal read_ints		#call 'read_ints' to load array
	
	move $t1, $v0		# move 'ys' address to $t4
	
	move $a0, $t1
	move $a1, $t0
	
	jal isort
	
	move $t4, $v0
	
	li $v0, 4
	la $a0, orig_str
	syscall
	
	move $a0, $t1
	move $a1, $t0
	jal print_ints
	
	li $v0, 4
	la $a0, new_line
	syscall
	
	li $v0, 4
	la $a0, sorted_str
	syscall
	
	move $a0, $t4
	move $a1, $t0
	jal print_ints
	
	li $v0, 4
	la $a0, new_line
	syscall
	
	li $v0, 10
	syscall

read_ints:
	# y = $t3
	# add_off = $t5
	# word_size = $t6
	
	addi $sp, $sp, -8	# push room on stack for '$sp' and 'n'
	sw $ra, 0($sp)		# push function return addres for 'jal'
	sw $a0, 4($sp)		# push orig 'n' onto stack
	
	move $t0, $a0		# copy 'n' from input arg
	
	li $t3, 4		# size of a word
	mul $t3, $t0, $t3	# size of needed memory for input array
	
	li $v0, 9
	move $a0, $t3		# load num bytes required
	syscall			# allocate 'n' * 4 bytes on heap
	move $t4, $v0		# store address of input array 'ys' into '$t4'
	move $s0, $t4
	
	#move $t3, $zero		# initialize 'y' to zero for defensive programming
	
	move $t3, $t4
	
	move $t2, $zero		# initialize 'ii' loop control to 0
input_loop:
	li $v0, 5
	syscall
	#move $t3, $v0		# move entry to 'y'
	
	#mul $t5, $t2, $t6	# calculate addr_off
	#add $t5, $t4, $t5	# determine next address in array 'ys'
	
	sw $v0, 0($t3)		# store the loaded element
	
	addi $t2, $t2, 1	# incriment loop controll 'ii'
	addi $t3, $t3, 4	# incriment array address
	blt $t2, $t0, input_loop	# if control < 'n', run again
	
	move $v0, $t4		# place array address in function return
	
	lw $ra, 0($sp)		# load function return address
	lw $t0, 4($sp)		# load 'n'
	addi $sp, $sp, 8	# pop the stack
	jr $ra
	
print_ints:
	# addr_off = $t3
	# word_size = $t6
	addi $sp, $sp, -12	# push room on stack for '$sp', 'n', and 'xs'
	sw $ra, 0($sp)		# push function return address for 'jal'
	sw $a0, 4($sp)		# push pointer address to array
	sw $a1, 8($sp)		# push 'n'
	
	move $t1, $a0		# move array address to local register
	move $t0, $a1		# move array length 'n'
	
	move $t2, $zero		# initialize loop counter
	
	#move $t3, $t1		# initialize address offset
	#li $t5, 4		# store word size
	
	
output_loop:
	#beq $t2, $t0, return_print_ints		# loop check
	
	
	li $v0, 1
	lw $a0, 0($t1)	# load int
	syscall
	
	li $v0, 4
	la $a0, space
	syscall
	
	addi $t2, $t2, 1	# incriment counter
	addi $t1, $t1, 4	# increase address offset by 4 bytes
	blt $t2, $t0, output_loop	# loop check
	
	li $v0, 4
	la $a0, new_line
	syscall
	
	lw $ra, 0($sp)
	lw $t1, 4($sp)
	lw $t0, 8($sp)
	addi $sp, $sp, 12
	jr $ra
	
insert:
	addi $sp, $sp, -16
	sw $ra, 0($sp)
	sw $a0, 4($sp)		# array address
	sw $a1, 8($sp)		# push 'n'
	sw $a2, 12($sp)		# push 'xx' num to insert
	
	move $t1, $a0		#xs
	move $t0, $a1		#nn
	move $t6, $a2		#xx
	
	move $t5, $zero		#yy
	move $t2, $zero		#ii
	
	move $t3, $t1
	
insert_loop:
	bge $t2, $t0, break_loop
	
	li $t3, 4
	mul $t3, $t2, $t3
	add $t3, $t1, $t3
	lw $t5, 0($t3)
	
	bge $t6, $t5, continue_insert_loop
	
	sw $t6, 0($t3)
	move $t6, $t5
	
continue_insert_loop:
	addi $t2, $t2, 1
	j insert_loop
	#li $t3, 4
	#mul $t3, $t2, $t3
	#add $t3, $t1, $t3
	
	#blt $t2, $t0, insert_loop
	
break_loop:
	li $t3, 4
	mul $t3, $t2, $t3
	add $t3, $t1, $t3
	sw $t6, 0($t3)
	
	lw $ra, 0($sp)
	lw $t4, 4($sp)
	lw $t2, 8($sp)
	lw $t6, 12($sp)
	addi $sp, $sp, 16
	jr $ra
	
isort:
	addi $sp, $sp, -12
	sw $ra, 0($sp)
	sw $a0, 4($sp)
	sw $a1, 8($sp)
	
	move $t1, $a0		# move address 'xs' to local temp
	move $t0, $a1		# move 'n' to local temp
	
	li $t3, 4		# size of a word
	mul $t3, $t3, $t0	# 'n' bytes required for 'ys'
	
	li $v0, 9
	move $a0, $t3		# load num bytes required
	syscall			# allocate 'n' * 4 bytes on heap
	move $t4, $v0		# get address of 'ys'
	
	move $t2, $zero
	
	move $s1, $t1
	move $s0, $t0
	
	
isort_loop:
	#move $s0, $t0
	#move $s1, $t1

	move $a0, $t4
	
	move $s2, $t2
	move $a1, $t2

	li $t3, 4
	mul $t3, $t2, $t3
	add $t3, $t1, $t3
	lw $a2, 0($t3)
	
	jal insert
	
	move $t0, $s0
	move $t1, $s1
	
	move $t2, $s2
	addi $t2, $t2, 1
	blt $t2, $t0, isort_loop
	
	move $v0, $t4
	
	lw $ra, 0($sp)
	lw $t1, 4($sp)
	lw $t0, 8($sp)
	addi $sp, $sp, 12
	jr $ra

addi x1, x0, 1
addi x2, x0, 8
sub x3, x2,x2
blt x2, x1, 8
add x2, x1, x0
blt x3, x1, 8
addi x2, x0, 2
sw x2, 0, x0
addi x4, x0, 0x20
jalr x3, x4, 12
addi x3, x0, 8
sw x3, 4, x0
lw x10, 4, x0
lw x11, 0, x0
	lw	1,0,five
	lw	2,1,2
start	add	1,1,2
	beq	0,1,done
	j	start
done	halt
start	.fill 	5
five    .fill   5
neg1	.fill 	2
stAddr	.fill	start
main    lui 1,5     #r1 = 5
        lui 2,f     #r2 = f
        jalr 6,2    #f()
        halt        #exit

f       add 3,0,0   #r3 = 0
        lw 4,0,n    #r4 = n
loop    sw 3,3,arr  
        addi 3,3,1  #i++
        slt 5,3,4
        beq 5,0,ret  
        j loop
ret     jalr 2,6

arr .space  5
n   .fill   5

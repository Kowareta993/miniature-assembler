main    lw 1,0,m     #r1 = m
        lw 2,0,n     #r2 = n
        or  3,1,2
        nand 3,3,3
        ori 4,2,10
        slti 5,4,-3
        halt        #exit

n .fill 1
m .fill 2
PUB start(address) 
  cognew(@copy_start, address)



DAT
    

    
   
            ' Read in the variables from the hub
copy_start  mov     addr, par
            mov     addr2, addr
            add     addr2, #4
            
        'At this point the addresses are now loaded.   Repeatedly copy the first variable into the second
copy_loop   rdlong  theValue, addr
            wrlong  theValue, addr2
            jmp     #copy_loop                  
    

addr                res     1          
addr2               res     1 
theValue            res     1
    fit
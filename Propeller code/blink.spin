 '' This program blinks the requested LED(s) at 8 Hz
 
  _CLKMODE = xtal1 + pll16x
  _XINFREQ = 5_000_000


PUB start (address)                                     'start the PWM loop into a cog
    cognew(@blink, address) 


DAT
        org     0
blink   
        rdlong  whichPin, par 
        'mov     whichPin, #17
        mov     pinMask, #1
        shl     pinMask, whichPin

        
                  
        mov     dira, pinMask   ' Make the appropriate pin be an output     
                          
        rdlong  Delay,  #0        ' read clkfreq variable                                
        shr     Delay, #4
        mov     Time,   cnt       
        add     Time,   #9        
                                  
loop   waitcnt Time,   Delay     
                                      
        xor     outa, pinMask
        jmp     #loop           
                                                
addr            res     1            
Delay           res     1
Time            res     1
whichPin        res     1
pinMask         res     1
fit
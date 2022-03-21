 '' This program does PWM on four pins starting at startPin.  
 '' It loads a PWM value from HUB RAM once per cycle
 '' The PWM values can change up to 1/4 the PWM frequency.  
 '' This program uses a linear feedback shift register to randomize 
 '' the timeslots so that there isn't a constant whine from motors
 '' being driven by this routine.
 ''
 '' To use this code, Put the following in the main program:
 ''
 ''VAR     
 ''   long    PWMs[4]
 ''   long    PWMstartPin
 ''
 ''PUB main
 ''   pwm.start(@PWMs)
 ''   
 '' After that, you can just modify the PWMs variables to change the output duty cycles.

 
  _CLKMODE = xtal1 + pll16x
  _XINFREQ = 5_000_000


PUB start (address)                                     'start the PWM loop into a cog
    cognew(@pwm_asm, address) 


DAT
            org     0
pwm_asm 
            ' Get the base address for the HUB RAM  
            mov     hubAddress, par 
        
            ' Read the PWM values
            mov     cogAddress, #PWMs
            movd    pwm1, cogAddress
            nop                     ' Need one instruction between a movd and the instruction that it modifies           
pwm1        rdlong  0-0, hubAddress

            add     hubAddress, #4
            add     cogAddress, #1
            movd    pwm2, cogAddress
            nop
pwm2        rdlong  0-0, hubAddress

            add     hubAddress, #4
            add     cogAddress, #1
            movd    pwm3, cogAddress
            nop
pwm3        rdlong  0-0, hubAddress

            add     hubAddress, #4
            add     cogAddress, #1
            movd    pwm4, cogAddress
            nop
pwm4        rdlong  0-0, hubAddress
            


            mov         PWMcycle, #255
        
            ' Now read which pin to start things on 
            add     hubAddress, #4
            rdlong  pin, hubAddress
            
            
            
            ' Make the appropriate pins be outputs
            mov     temp, #15  ' This will now be a series of 4 1's starting bit 0
            shl     temp, pin ' This was hard-coded as #12
            mov     dira, temp
            
            ' setup the pin variables.
            mov     temp, #1
            shl     temp, pin
            mov     pin0, temp
            shl     temp, #1
            mov     pin1, temp
            shl     temp, #1
            mov     pin2, temp
            shl     temp, #1
            mov     pin3, temp
            
              
        
   
                                                                      
            mov     loadCounter, #0                              
            mov     cogAddress, #PWMs
            mov     hubAddress, par
            mov     whichPWM, #0
                                
                           
            rdlong  delay,  #0          ' read clkfreq variable
            
            ' The delays below are accurate if the timeslots aren't randomized.
            ' 
            shr     delay,  #19         ' Makes the PWM be ~2000 Hz  ~470Hz if randomized.
            'shr     delay,  #18         ' Makes the PWM be ~1000 Hz.  
            'shr     delay,  #17         ' Makes the PWM be ~500 Hz.  

            mov     time,   cnt       
            add     time,   #100          '  Adding this makes it so that it doesn't have to wrap around 
                                        '  the counter at the beginning before starting the PWM  
                                        '  
            mov     lfsr,   #1          ' seed the linear feedback shift register for random numbers.
                

loop


            ' This section randomizes the time slot's length to spread the frequency of the PWM 
            ' so that it doesn't have a high pitch whine
            mov         alteredDelay, delay
            
            ' Adjust the lfsr to generate a pseudorandom number
            and     lfsr, lfsrMask      wc,nr  ' The carry bit will be set if the result has an odd number of high bits
            shl     lfsr, #1
    if_c    add     lfsr, #1
            
            ' Now adjust the delay time based on the lfsr
            mov     lfsrTemp, lfsr
            shr     lfsrTemp, #23
            add     alteredDelay, lfsrTemp 
                     
            waitcnt   time,   alteredDelay  
            

            
            
            'waitcnt   time,   delay  
              
            djnz      PWMcycle,#PWM01           

            ' If it gets to here then it has just finished a PWM cycle
            mov       PWMcycle,#255     ' Reset the counter
             
             
            ' Load one PWM variable per cycle.
            movd   selfmod01, cogAddress
            add    cogAddress, #1 
            nop
selfmod01   rdlong 0-0, hubAddress

            ' Move the address pointers 
                 
            add     hubAddress, #4
            add     whichPWM, #1
            cmp     whichPWM, #4    wz
    if_z    mov     whichPWM, #0
    if_z    mov     hubAddress, par
    if_z    mov     cogAddress, #PWMs       
              
              
              
                                  
PWM01
            mov     outPins, #0
            cmp     PWMs, PWMcycle      wc
    if_nc    add     outPins, pin0
            cmp     PWMs+1, PWMcycle      wc
    if_nc    add     outPins, pin1
            cmp     PWMs+2, PWMcycle      wc
    if_nc    add     outPins, pin2
            cmp     PWMs+3, PWMcycle      wc
    if_nc    add     outPins, pin3
                     
            mov     outa, outPins                           

            jmp     #loop   
            
            
lfsrMask1       long    %00000000000000000000000000000001
lfsrMask2       long    %00000000000000000000000000000010
lfsrMask3       long    %00000000000000000000000000000100
lfsrMask4       long    %00000000001000000000000000000000 
lfsrMask        long    %00000000001000000000000000000111 
lfsr            res     1
lfsrTemp        res     1               
outPins         res     1                                              
hubAddress      res     1            
delay           res     1
alteredDelay    res     1
time            res     1
PWMcycle        res     1
PWMs            res     4
cogAddress      res     1
loadCounter     res     1
whichPWM        res     1
pin             res     1
temp            res     1
pin0            res     1
pin1            res     1
pin2            res     1
pin3            res     1


fit
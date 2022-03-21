{ This file contains a positional PID algorithm 
    TODO:  There will be an issue when it wraps around.  Fix that.
    The main loop runs at 20.5 kHz.  It should maybe be slowed down to make the I term make more sense. 
}


PUB start(address) 
  cognew(@pid_start, address)

' 177 longs currently


DAT
    

{   This is a dual motor control position-based PID algorithm.  
    Pass the address of the first requested position to this algorithm to get things started.

    long    requestedPosition0
    long    requestedPosition1
    long    Pterm
    long    Iterm
    long    Dterm
    long    numberOfEncoders
    long    encoderBasePin
    long    encoderPosition0
    long    encoderPosition1
    long    PWMs[4]
}
    
pid_start    
            ' Read in the variables from the hub
            mov     addr, par
            mov     requestedPositionBaseAddress, addr
            add     addr, #8
            mov     PIDbaseAddress, addr
            add     addr, #20 ' skip over the PID values, numberOfEncoders and encoderBasePin variables
            mov     encoderPositionBaseAddress, addr
            add     addr, #8
            mov     PWMbaseAddress, addr
    
    

    
            mov     dira, #1
    
    ' OK all of the variable addresses have been loaded
    ' Start the main loop where it repeatedly reads the PID terms, requested positions and the encoders
    ' then calculates the response 
     
    ' Read the PID terms (both motors)
            mov     addr, PIDbaseAddress
            rdlong  P, addr
            add     addr, #4
            rdlong  I, addr
            add     addr, #4
            rdlong  D, addr
     
            mov     waitcntTimeVar, pauseDuration 
            add     waitcntTimeVar, cnt
            
            
PIDloopStart
            mov     outa, #0
            waitcnt waitcntTimeVar, pauseDuration
            mov     outa, #1
            

            
    
    ' Read the requested positions (both motors)
            mov     addr, requestedPositionBaseAddress
            rdlong  requestedPosition0, addr
            add     addr, #4
            rdlong  requestedPosition1, addr
            
            
            
            
            
            
            
    
    ' Read the encoder positions (both motors)
            mov     addr, encoderPositionBaseAddress
            rdlong  encoderPosition0, addr
            add     addr, #4
            rdlong  encoderPosition1, addr
            
       
    ' ------- MOTOR A -------
    
    ' shuffle the proportionalErrors arrays (unrolling the loop is the fastest way to shuffle these)       
            mov     proportionalErrorsA+7, proportionalErrorsA+6
            mov     proportionalErrorsA+6, proportionalErrorsA+5
            mov     proportionalErrorsA+5, proportionalErrorsA+4
            mov     proportionalErrorsA+4, proportionalErrorsA+3
            mov     proportionalErrorsA+3, proportionalErrorsA+2
            mov     proportionalErrorsA+2, proportionalErrorsA+1
            mov     proportionalErrorsA+2, proportionalErrorsA+0
            

    ' Calculate the new error value and put it into the proportional Error array
            mov     PIDtemp, requestedPosition0    
            sub     PIDtemp, encoderPosition0
            mov     proportionalErrorsA, PIDtemp

      
      
    ' Calculate the P value (Working)
            mov     multiplyA, P
            mov     multiplyB, proportionalErrorsA
            call    #signedMultiply32x32
            mov     PIDvalue, returnVal
        
            
            
    ' Calculate the I value (unrolling the loop is the fastest way to sum these)
            mov     PIDtemp, proportionalErrorsA
            adds    PIDtemp, proportionalErrorsA+1
            adds    PIDtemp, proportionalErrorsA+2
            adds    PIDtemp, proportionalErrorsA+3
            adds    PIDtemp, proportionalErrorsA+4
            adds    PIDtemp, proportionalErrorsA+5
            adds    PIDtemp, proportionalErrorsA+6
            adds    PIDtemp, proportionalErrorsA+7
            
            
            
            'PIDtemp will now hold the sum of the errors
            mov     multiplyA, I
            mov     multiplyB, PIDtemp
            call    #signedMultiply32x32              ' This could be slightly faster by not using PIDtemp
            adds    PIDvalue, returnVal
            
            
            
    
    ' Calculate the D value
            mov     PIDtemp, proportionalErrorsA + 1
            subs    PIDtemp, proportionalErrorsA
            mov     multiplyA, D
            mov     multiplyB, PIDtemp
            call    #signedMultiply32x32
            adds    PIDvalue, returnVal
            
           

        
    ' Now use the resulting value to update the PWM registers.  
            cmps    PIDvalue, #0        wc
            ' Scale the results
            abs     PIDvalue, PIDvalue
            shr     PIDvalue, #4        ' divide by 16.  
            mov     addr, pwmBaseAddress
            
            mov     PIDtemp, #0  
   
   if_c     jmp     #setPWMA                   'PIDvalue < 0
                                             
            wrlong  PIDvalue, addr
            add     addr, #4
            wrlong  PIDtemp, addr
            
            jmp     #PWMAdone          
    
setPWMA        
            wrlong  PIDtemp, addr
            add     addr, #4
            wrlong  PIDvalue,addr
             
PWMAdone
            
            
            
    ' ------- MOTOR B -------
    
    ' shuffle the proportionalErrors arrays (unrolling the loop is the fastest way to shuffle these)       
            mov     proportionalErrorsB+7, proportionalErrorsB+6
            mov     proportionalErrorsB+6, proportionalErrorsB+5
            mov     proportionalErrorsB+5, proportionalErrorsB+4
            mov     proportionalErrorsB+4, proportionalErrorsB+3
            mov     proportionalErrorsB+3, proportionalErrorsB+2
            mov     proportionalErrorsB+2, proportionalErrorsB+1
            mov     proportionalErrorsB+2, proportionalErrorsB+0
            

    ' Calculate the new error value and put it into the proportional Error array
            mov     PIDtemp, requestedPosition1    
            sub     PIDtemp, encoderPosition1
            mov     proportionalErrorsB, PIDtemp
            
          ' mov     PIDvalue, proportionalErrorsB   ' Just for testing
      
      
    ' Calculate the P value (Working)
            mov     multiplyA, P
            mov     multiplyB, proportionalErrorsB
            call    #signedMultiply32x32
            mov     PIDvalue, returnVal
        
            
            
    ' Calculate the I value (unrolling the loop is the fastest way to sum these)
            mov     PIDtemp, proportionalErrorsB
            adds    PIDtemp, proportionalErrorsB+1
            adds    PIDtemp, proportionalErrorsB+2
            adds    PIDtemp, proportionalErrorsB+3
            adds    PIDtemp, proportionalErrorsB+4
            adds    PIDtemp, proportionalErrorsB+5
            adds    PIDtemp, proportionalErrorsB+6
            adds    PIDtemp, proportionalErrorsB+7
            
            
            
            'PIDtemp will now hold the sum of the errors
            mov     multiplyA, I
            mov     multiplyB, PIDtemp
            call    #signedMultiply32x32              ' This could be slightly faster by not using PIDtemp
            adds    PIDvalue, returnVal
            
            
            
    
    ' Calculate the D value
            mov     PIDtemp, proportionalErrorsB + 1
            subs    PIDtemp, proportionalErrorsB
            mov     multiplyA, D
            mov     multiplyB, PIDtemp
            call    #signedMultiply32x32
            adds    PIDvalue, returnVal
            
           

    
    
    ' Now use the resulting value to update the PWM registers.  
            cmps    PIDvalue, #0        wc
            ' Scale the results
            abs     PIDvalue, PIDvalue
            shr     PIDvalue, #4        ' divide by 16.  
            mov     addr, pwmBaseAddress
            add     addr, #8        ' offset for the second motor
            
            mov     PIDtemp, #0  
   
   if_c     jmp     #setPWMB                   'PIDvalue < 0
                                             
            wrlong  PIDvalue, addr
            add     addr, #4
            wrlong  PIDtemp, addr
            
            jmp     #PWMBdone          
    
setPWMB        
            wrlong  PIDtemp, addr
            add     addr, #4
            wrlong  PIDvalue,addr
             
PWMBdone    
            jmp     #PIDloopStart
    
    
    
    
    
'---------------------------------------------------------------------------------------------
            
signedMultiply32x32
{ Multiplies two signed 32-bit values and returns a signed 64-bit result.
  To use, load multiplyA and multiplyB with the values to be multiplied and call this function.
  The return value will be in highLong and returnVal (low long)
}
        ' make values positive, multiplySign holds sign of result 
            abs     multiplyA, multiplyA WC          
            muxc    multiplySign,#1                  
            abs     multiplyB, multiplyB WC, WZ      
    if_c    xor     multiplySign,#1                  


        ' Setup for 32 bits of multiply loop
            mov     highLong,#0                  
            mov     multiplyCount,#32                 
            shr     multiplyA,#1 WC               

 
__L0001 ' Main multiply loop 
        ' This is adding to highLong if the carry bit was high then shifting everything to the right by one bit.  
        ' This loop takes 160 instructions                                                   
    if_c    add     highLong,multiplyB WC          
            rcr     highLong,#1 WC               
            rcr     multiplyA,#1 WC               
            djnz    multiplyCount,#__L0001 
                       
        ' Check if result should be negative
            test    multiplySign,#1 WZ               
    if_nz   neg     highLong,highLong             
    if_nz   neg     multiplyA,multiplyA WZ          
    if_nz   sub     highLong,#1                  

            mov     returnVal,multiplyA               ' Store lower 32-bit of result, highLong = upper 32-bits of result

            'mov     returnVal, multiplyB
signedMultiply32x32_ret    ret
    
           

pauseDuration       long    80000000/1024   ' This makes it run at 128 Hz. 
proportionalErrorsA long    0[8]   ' last 8 errors for two encoders
proportionalErrorsB long    0[8]    
PIDtemp             res     1
multiplyA           res     1
multiplyB           res     1
multiplySign        res     1
multiplyCount       res     1
highLong            res     1
returnVal           res     1
count               res     1
addr                res     1            
requestedPositionBaseAddress res     1

requestedPosition0  res     1
requestedPosition1  res     1

encoderPositionBaseAddress    res     1
encoderPosition0    res     1
encoderPosition1    res     1
PWMbaseAddress      res     1
PIDbaseAddress      res     1
P                   res     1   ' These PID variables hold the scale factors.
I                   res     1
D                   res     1
PIDvalue            res     1   ' This is the sum of the errors that were multiplied with the P, I, & D values.  

proportionalError   res     1
integralError       res     1
differentialError   res     1

waitcntTimeVar      res     1


fit
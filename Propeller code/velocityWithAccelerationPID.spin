{ This file contains a velocity PID algorithm that incorporates acceleration

    The function of this program, in short is that you can put whatever velocity that you want into the 
    requestedVelocity variables but it won't jump immediately to that velocity, instead, it will ramp up
    or down to that velocity based on the given acceleration.  
    
    To adjust the velocity, first the direction that it needs to accelerate must be found.  Then, the 
    acceleration will be added to an acceleration accumulator.  A copy of the accumulator will be divided 
    by the number of iterations of this algorithm per second.  The resulting value will be added to or 
    subtracted from the actual velocity based on the direction that it needs to change.  One final check
    will ensure that the actual velocity didn't pass the requested velocity due to a high acceleration 
    rate. 

    TODO:  There will be an issue when it wraps around.  Fix that.
    TODO:  Will this code have an issue going into negative numbers?  (I think no because one motor is)
    The way that this program works is that it has a positional PID loop that has an additional 
    bit of code that (potentially) adjusts the position every time the loop runs.   It is doing something
    like dithering or bresenham's line drawing algorithm where there is an accumulator that spreads the 
    error over multiple timeslots.  Every frame, the velocity is added to the accumulator.  The 
    accumulator's high 22 bits are shifted right by 10 and that value is added or subtracted from the 
    position.  The high 22 bits are cleared in the accumulator before the next loop.  I am shifting by 10
    because this code runs 1024 times a second.  This allows the requested velocities to be in ticks per
    second.  
    
    
    velocity is in ticks per second
    velocity is in ticks per second^2
}


PUB start(address) 
  cognew(@pid_start, address)

' 177 longs currently


DAT
    

{   This is a dual motor control position-based PID algorithm.  
    Pass the address of the acceleration variable to this algorithm to get things started.

    long    acceleration
    long    requestedVelocity0
    long    requestedVelocity1
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
            mov     accelerationAddress, addr
            add     addr,#4
            mov     requestedVelocityBaseAddress, addr
            add     addr, #8
            mov     PIDbaseAddress, addr
            add     addr, #20 ' skip over the PID values, numberOfEncoders and encoderBasePin variables
            mov     encoderPositionBaseAddress, addr
            add     addr, #8
            mov     PWMbaseAddress, addr
    
            mov     requestedVelocity0, #0
            mov     requestedVelocity1, #0
            mov     requestedPosition0, #0
            mov     requestedPosition1, #0    
            mov     accumulator0, #0
            mov     accumulator1, #0

    
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
            
            
            rdlong  acceleration, accelerationAddress
     
            mov     waitcntTimeVar, pauseDuration 
            add     waitcntTimeVar, cnt
            
            
PIDloopStart
            'mov     outa, #0
            waitcnt waitcntTimeVar, pauseDuration
            'mov     outa, #1
            
       
    
    ' Read the requested velocities (both motors)
            mov     addr, requestedVelocityBaseAddress
            rdlong  requestedVelocity0, addr
            add     addr, #4
            rdlong  requestedVelocity1, addr
            
    
    
    ' Do some initial work for acceleration
            add     accelAccumulator, acceleration
            mov     accelTemp, accelAccumulator
            shr     accelTemp, #10
            and     accelAccumulator, accumulatorMask
            
    ' Does it need to accelerate and if so, in what direction?
    ' 
    ' Motor 0:
            cmps    requestedVelocity0, currentVelocity0    wc,wz
            ' if z == 1 then they are equal
            ' if c == 1 then currentVelocity0 > requestedVelocity0
            ' if c == 0 then currentVelocity0 <= requestedVelocity0
     if_z   jmp     #motor0accelDone
     if_c   jmp     #motor0A ' if currentVelocity0 > requestedVelocity0
            '  currentVelocity0 <= requestedVelocity0
            adds    currentVelocity0, accelTemp
            cmps    requestedVelocity0, currentVelocity0    wc
     if_c   mov     currentVelocity0, requestedVelocity0  
            jmp     #motor0accelDone                  
motor0A 
            subs    currentVelocity0, accelTemp  
            cmps    requestedVelocity0, currentVelocity0    wc
     if_nc  mov     currentVelocity0, requestedVelocity0 
                
motor0accelDone


    ' Motor 1:
            cmps    requestedVelocity1, currentVelocity1    wc,wz
            ' if z == 1 then they are equal
            ' if c == 1 then currentVelocity1 > requestedVelocity1
            ' if c == 0 then currentVelocity1 <= requestedVelocity1
     if_z   jmp     #motor1accelDone
     if_c   jmp     #motor1A ' if currentVelocity1 > requestedVelocity1
            '  currentVelocity1 <= requestedVelocity1
            adds    currentVelocity1, accelTemp
            cmps    requestedVelocity1, currentVelocity1    wc
     if_c   mov     currentVelocity1, requestedVelocity1  
            jmp     #motor1accelDone                  
motor1A 
            subs    currentVelocity1, accelTemp  
            cmps    requestedVelocity1, currentVelocity1    wc
     if_nc  mov     currentVelocity1, requestedVelocity1 


motor1accelDone

    
    
    ' Move the positions based on the requested velocities
            abs     PIDtemp2, currentVelocity0      wc
            add     accumulator0, PIDtemp2
            mov     PIDtemp, accumulator0
            and     accumulator0, accumulatorMask   ' clear all but the low 10 bits
            shr     PIDtemp, #10
    if_c    add     requestedPosition0, PIDtemp
    if_nc   sub     requestedPosition0, PIDtemp     
                     
            
            abs     PIDtemp2, currentVelocity1      wc
            add     accumulator1, PIDtemp2
            mov     PIDtemp, accumulator1
            and     accumulator1, accumulatorMask   ' clear all but the low 10 bits
            shr     PIDtemp, #10
    if_c    add     requestedPosition1, PIDtemp
    if_nc   sub     requestedPosition1, PIDtemp     
                     

            
            
    
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
            ' First add a deadband to the value so that it isn't making noise if it is just sitting there.
            
    
            cmps    PIDvalue, #0        wc
            ' Scale the results
            abs     PIDvalue, PIDvalue
            shr     PIDvalue, #4        ' divide by 16.  
            mov     addr, pwmBaseAddress
            
            ' TODO: Put in a scale factor here so that it linearizes the PWM values.  Currently a value of 64
            ' sometimes doesn't start the motor.  It would also be nice to add a deadband to the PWM so that 
            ' if it is below a certain threshold it will just set the PWM value to be zero.  
            ' 
    
            
            mov     PIDtemp, #0  
   
   if_c     jmp     #setPWMA                   'PIDvalue < 0
            
            ' Add a deadband in to keep it quiet when it is just sitting there 
            cmp     PIDvalue, #32       wc
    if_c    mov     PIDvalue, #0
            
                                             
            wrlong  PIDvalue, addr              ' the PWM value
            add     addr, #4
            wrlong  PIDtemp, addr               ' zero for the other PWM pin for this motor
            
            jmp     #PWMAdone          
    
setPWMA     
            ' Add a deadband in to keep it quiet when it is just sitting there 
            cmp     PIDvalue, #32       wc
    if_c    mov     PIDvalue, #0
       
            wrlong  PIDtemp, addr               ' zero
            add     addr, #4    
            wrlong  PIDvalue,addr               ' The PWM value
             
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
                                               '
            ' Add a deadband in to keep it quiet when it is just sitting there 
            cmp     PIDvalue, #32       wc
    if_c    mov     PIDvalue, #0
                                        
            wrlong  PIDvalue, addr
            add     addr, #4
            wrlong  PIDtemp, addr
            
            jmp     #PWMBdone          
    
setPWMB     
            ' Add a deadband in to keep it quiet when it is just sitting there 
            cmp     PIDvalue, #32       wc
    if_c    mov     PIDvalue, #0
          
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
requestedVelocity0  long    0
requestedVelocity1  long    0
direction0          long    0
direction1          long    0
'accumulatorMask is the low 10 bits.    
accumulatorMask     long    %00000000000000000000001111111111
currentVelocity0    long    0
currentVelocity1    long    0


accelerationAddress res     1
acceleration        res     1
accelAccumulator    res     1   ' There is only one acceleration variable so only one accumulator as well.
accelTemp           res     1


accumulator0        res     1
accumulator1        res     1
PIDtemp             res     1
PIDtemp2            res     1
multiplyA           res     1
multiplyB           res     1
multiplySign        res     1
multiplyCount       res     1
highLong            res     1
returnVal           res     1
count               res     1
addr                res     1            
requestedVelocityBaseAddress res     1

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
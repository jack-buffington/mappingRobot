{
encoder.spin
Written by Jack Buffington Dec 2020
Reads one or more encoders and stores them in HUB RAM.
The encoders must all be attached to consecutive pins.
The initial encoder positions are loaded at the beginning but thereafter are kept in 
COG RAM and copied out to HUB at each cycle.  There is no read, modify, write going on
so you can't modify the counter after the fact from a HUB program.  To do that, you would 
need to kill the cog and then restart it after changing the encoder value in HUB RAM.

This code is optimized for ease of adaptability for different numbers of encoders It could 
be written to be faster.  See comments at the bottom of the code for more info.

To use, call start with the address of the first variable in this list:
long    numberOfEncoders
long    basePin             This is the first encoder pin. All others will be consecutively 
                            higher pins.
long    encoder0position
long    encoder1position   (optional) 
long    encoder2position   (optional) 
... 

In main program:
    long    numberOfEncoders
    long    encoderBasePin
    long    encoderPosition
    long    encoderPosition2
    
    
    numberOfEncoders := 2      ' I want two encoders
    encoderBasePin := 16       ' Encoder 1 attached to pins 16 & 17
                               ' Encoder 2 attached to pins 18 & 19
    encoderPosition := 8       ' Initial encoder value
    ENCODER.start(@numberOfEncoders) 
}


PUB start(address) 

  cognew(@encoderStart, address)

DAT
            org
encoderStart
    ' Read in all of the variables.
            mov     HUBaddress, par
            rdlong  numberOfEncoders, HUBaddress
            add     HUBaddress, #4              ' Should be pointing at basePin
            rdlong  basePin, HUBaddress
            mov     count, numberOfEncoders
            mov     COGaddress, #positions
                                 
            mov     dira, #0                    ' Make all pins be inputs

            mov     baseHubAddress, HUBaddress
            add     baseHubAddress, #4          ' Should be pointing at position0
            
            
            
            
            
            
encoder01   ' This loop loads the initial encoder positions         
            movd    encoder02, COGaddress
            add     COGaddress, #1
            add     HUBaddress, #4
encoder02   rdlong  0-0, HUBaddress
            djnz    count, #encoder01
           






                  
encoder03   ' This outer loop runs forever and sets things up for the inner loop           
            ' First set things up for going through the encoders
            mov     count, numberOfEncoders
            mov     encoderBitmask, #3
            shl     encoderBitmask, basePin
            mov     shiftOffset, basePin
            mov     hubAddress, baseHubAddress
            
            mov     lastEncodersAddress, #lastEncoders
            mov     positionsAddress, #positions
            
            
            
            
            
            
            
            ' This is the inner loop that iterates through all encoders
encoder04   ' Now adjust the encoder positions if necessary
            ' Read the encoder
            movs    encoder05, lastEncodersAddress  ' This is the pins from last time
            movs    encoder06, positionsAddress     ' This is the position from last time
            mov     currentEncoder, ina
            and     currentEncoder, encoderBitmask
            shr     currentEncoder, shiftOffset     ' The current encoder pins should now be 
                                                    ' right aligned in currentEncoder
            ' Get things into lastEncoder and encoderPosition in prep for adjustEncoder.  
            ' currentEncoder was already taken care of above
encoder05   mov     lastEncoder, 0-0                ' Get the last encoder value
            movd    encoder07, lastEncodersAddress
encoder06   mov     encoderPosition, 0-0            ' Get the last encoder position
            ' Copy currentEncoder into the lastEncoders array for next time.
encoder07   mov     0-0, currentEncoder             ' Copy the current encoder into the last encoder array       
            
            
            'Now that currentEncoder, lastEncoder, and encoderPosition are ready, call adjustEncoder
            call    #adjustEncoder
            ' At this point, encoderPosition will hold the adjusted position.
            
            
            movd    encoder08, positionsAddress    ' prep to store the new position in COG RAM

            wrlong  encoderPosition, HUBaddress     ' Write the adjusted position to HUB RAM
             
encoder08   mov     0-0, encoderPosition            ' Copy the adjusted position into the positions array 
            
            ' Get things ready for next time
            add     lastEncodersAddress, #1
            add     positionsAddress, #1
            add     shiftOffset, #2
            shl     encoderBitmask, #2
            add     hubAddress, #4
            djnz    count, #encoder04
            jmp     #encoder03                      ' If it gets to here then it is done
                                                    ' iterating through the encoders






adjustEncoder
    ' This function takes encoderPosition, lastEncoder, and currentEncoder as inputs
    ' It then xors currentEncoder and lastEncoder and uses that to adjust
    ' encoderPosition.  It then returns and encoderPosition is copied to where it 
    ' really needs to go.
    ' lastEncoder should have their values in the least significant bits of the long.
    ' All other bits in lastEncoder and currentEncoder should be zeros.
    ' 
    ' This routine isn't the fastest possible way to program this but it allows for 
    ' simpler code.  The fastest way would be to use computed jumps as shown in 
    ' encoderLoopup.  In that function, the last encoder values and current encoder
    ' values would be concatenated, multiplied by 2, and used to adjust the return value

                mov     encoderTemp, lastEncoder
                xor     encoderTemp, currentEncoder     wz
                
    ' If xoring of the two encoder values results in 0 or 3 then there should be no change
    ' 00: It didn't move
    ' 11: It moved too fast and it doesn't know which way to go  
        if_nz   cmp     encoderTemp, #3                 wz    
        if_z    jmp     #adjustEncoder_ret
        
    ' It is going up or down so let's figure out which way it is going.  To do that, I'll
    ' shift the currentEncoder left by 1 and xor them again.  If bit #1 is 1 then it is 
    ' going up, otherwise it is going down.
                shl     currentEncoder, #1
                xor     lastEncoder, currentEncoder
                and     lastEncoder, #2                 wz
      if_z      sub     encoderPosition, #1
      if_nz     add     encoderPosition, #1            
        
adjustEncoder_ret   ret







              
                                               
encoder1        long      	0 ' If it needs to go up and down then 2,147,483,648 will put it right at the center.  
encoder2        long      	0 

' The next three variables are used by adjustEncoder.    
encoderPosition res             1
lastEncoder     res             1
currentEncoder  res             1

numberOfEncoders    res         1
encoderTemp     res             1
count           res             1
HUBaddress      res             1
COGaddress      res             1
basePin         res             1
positions       res             16  ' These are the actual positon of the encoders
lastEncoders    res             16  ' These are the (right-aligned) encoder pins from last time.
encoderBitmask  res             1
shiftOffset     res             1
temp            res             1
baseHUBaddress  res             1   ' This is where the encoder positions start
lastEncodersAddress res         1
positionsAddress    res         1


{
' This is a faster way of doing an encoder but you have to manually code for every encoder.  The code 
' above is more flexible.  If you need ultimate speed though, do it like below.  This is what I did 
' for Matchtrax.  I created a variable from the current encoder pins and previous encoder pins and then 
' did a computed jump to this routine.  Maybe it could be even faster if it did a lookup from a 
' table of pre-computed values and then added that value?  I'm not sure.  
' 
encoderLookup nop                               'case 0
              jmp       #encoder1Return
              add       encoder1,#1                 'case 1
              jmp       #encoder1Return
              sub       encoder1,#1                 'case 2
              jmp       #encoder1Return
              nop                                   'case 3
              jmp       #encoder1Return
              sub       encoder1,#1                 'case 4
              jmp       #encoder1Return
              nop                                   'case 5
              jmp       #encoder1Return
              nop                                   'case 6
              jmp       #encoder1Return  
              add       encoder1,#1                 'case 7
              jmp       #encoder1Return
              add       encoder1,#1                 'case 8
              jmp       #encoder1Return
              nop                                   'case 9
              jmp       #encoder1Return
              nop                                   'case 10
              jmp       #encoder1Return
              sub       encoder1,#1                 'case 11
              jmp       #encoder1Return
              nop                                   'case 12
              jmp       #encoder1Return
              sub       encoder1,#1                 'case 13
              jmp       #encoder1Return
              add       encoder1,#1                 'case 14
              jmp       #encoder1Return
              nop                                   'case 15
encoderLookup_ret		ret
}
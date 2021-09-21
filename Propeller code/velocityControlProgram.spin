{ This code connects the two motors so that if you turn one of them, the other will match its position
}

CON
  _CLKMODE = XTAL1 + PLL16X
  _XINFREQ = 5_000_000
  
OBJ
    PWM         : "PWM.spin"
    SERIAL      : "quadSerialPorts.spin"
    ENCODER     : "encoder.spin"
    PID         : "velocityPID.spin"

    
VAR
    
                                 
                                 
    long    spinRequestedVelocity0  ' Encoder ticks per high-level timeslot
    long    spinRequestedVelocity1
    long    Pterm
    long    Iterm
    long    Dterm
    long    numberOfEncoders
    long    encoderBasePin
    long    spinEncoderPosition0
    long    spinEncoderPosition1
    long    PWMs[4]
    long    PWMstartPin
    
    
    
    long    lastEncoder0
    long    lastEncoder1
    long    mainTemp




    
PUB launchStuff

    start_uarts
    serial.str(0,string("Starting up...",13))

    numberOfEncoders := 2      ' I want two encoders
    encoderBasePin := 4       ' Encoder 1 attached to pins 4 & 5
                               ' Encoder 2 attached to pins 6 & 7
    spinEncoderPosition0 := 0       ' Initial encoder value
    ENCODER.start(@numberOfEncoders) 
    serial.str(0,string("Done  setting up the encoder",13))
    serial.str(0,string("clkfreq: "))
    serial.sendLongToSerial(0, clkfreq)
    
    PWMstartPin := 12
    
    PWMs[0] := 32
    PWMs[1] := 0
    PWMs[2] := 16
    PWMs[3] := 0

    Pterm := 10      
    Iterm := 6
    Dterm := 30
   
    pwm.start(@pwms)
    
    
    spinRequestedVelocity0 := 0
    spinRequestedVelocity1 := 0
    
    lastEncoder0 := 0
    lastEncoder1 := 0
    
    
    PID.start(@spinRequestedVelocity0)
    { 
    spinRequestedVelocity0 := 400
    spinRequestedVelocity1 := -400  
     
    repeat 
    } 
     
    repeat 1000
        spinRequestedVelocity0 -= 4
        spinRequestedVelocity1 += 4   
        waitcnt(cnt + clkfreq / 200)
      
     

    repeat    
        repeat 2000
            spinRequestedVelocity0 += 4
            spinRequestedVelocity1 -= 4   
            waitcnt(cnt + clkfreq / 200)
     
        waitcnt(cnt + clkfreq * 1)
        
        repeat 2000
            spinRequestedVelocity0 -= 4
            spinRequestedVelocity1 += 4   
            waitcnt(cnt + clkfreq / 200)
        
        waitcnt(cnt + clkfreq * 1)
        
        
        
    
        
{        
    repeat    
        repeat 800
            spinRequestedVelocity0 -= 1
            spinRequestedVelocity1 += 1   
            waitcnt(cnt + clkfreq / 100)   
            
        waitcnt(cnt + clkfreq * 2)
             
        repeat 800
            spinRequestedVelocity0 += 1
            spinRequestedVelocity1 -= 1   
            waitcnt(cnt + clkfreq / 100)    
    
 }   
    




PUB start_uarts
  SERIAL.init                    
  'AddPort      (port,rxpin,txpin,ctspin,rtspin,rtsthreshold,mode,baudrate)
  SERIAL.AddPort(0,   10,   11,   -1,    -1,    0,           0,   115200)
  'SERIAL.AddPort(1,   12,   13,   -1,    -1,    0,           0,   115200)
  'SERIAL.AddPort(2,   14,   15,   -1,    -1,    0,           0,   115200)
  SERIAL.Start
  ' Give it some time to start up
  
  waitcnt(cnt + clkfreq/10)





{DAT

PWMs              long        0[4]}
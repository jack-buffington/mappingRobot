{ This code connects the two motors so that if you turn one of them, the other will match its position
}

CON
  _CLKMODE = xtal1 + pll16x
  '_CLKMODE = RCFAST
  _XINFREQ = 5_000_000
  'LED   =   1 << 21
  '_KILL =   1 << 24
  LED = 21
  _KILL = 24
  
  
OBJ
    'PWM         : "PWM.spin"
    'SERIAL      : "quadSerialPorts.spin"
    'ENCODER     : "encoder.spin"
    
    'PID         : "velocityWithAccelerationPID.spin"

    
VAR
    
                                 
    long    spinAcceleration                             
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

    ' The first thing this program needs to do is ensure that its power stays on.   To do this, 
    ' The processor should set pin 24 high to make the power chip not just turn off again.
    
    ' First though, make that pin be an output
    DIRA[_KILL] := 1
    OUTA[_KILL] := 1
    
    'Good.  Now it should be permanently on.  Let's blink a LED to indicate success.   
    DIRA[LED] := 0
    
    repeat
        OUTA[LED] := 1
        waitcnt(cnt + clkfreq / 2)
        OUTA[LED] := 0
        waitcnt(cnt + clkfreq / 2)
        

    'start_uarts
    'serial.str(0,string("Starting up...",13))

    'numberOfEncoders := 2      ' I want two encoders
    'encoderBasePin := 4       ' Encoder 1 attached to pins 4 & 5
                               ' Encoder 2 attached to pins 6 & 7
    'spinEncoderPosition0 := 0       ' Initial encoder value
    'ENCODER.start(@numberOfEncoders) 
    'serial.str(0,string("Done  setting up the encoder",13))
    'serial.str(0,string("clkfreq: "))
    'serial.sendLongToSerial(0, clkfreq)
    
    'PWMstartPin := 12
    
    'PWMs[0] := 32
    'PWMs[1] := 0
    'PWMs[2] := 16
    'PWMs[3] := 0

    'Pterm := 10      
    'Iterm := 6
    'Dterm := 30
   
    'pwm.start(@pwms)
    
    
    'spinRequestedVelocity0 := 0
    'spinRequestedVelocity1 := 0
    
    'lastEncoder0 := 0
    'lastEncoder1 := 0
    
    
    'PID.start(@spinAcceleration)

     
    'spinAcceleration := 500
    'serial.str(0,string("Starting the velocity loop.",13)) 
    'repeat
    '    serial.str(0,string("A",13))
    '    spinRequestedVelocity0 := -3000
    '    spinRequestedVelocity1 := 500   
    '    waitcnt(cnt + clkfreq * 12)
    '    
    '    serial.str(0,string("B",13))
    '    spinRequestedVelocity0 := 3000
    '    spinRequestedVelocity1 := -500   
    '    waitcnt(cnt + clkfreq * 12)
     


        
        
    





'PUB start_uarts
'  SERIAL.init                    
'  'AddPort      (port,rxpin,txpin,ctspin,rtspin,rtsthreshold,mode,baudrate)
'  SERIAL.AddPort(0,   10,   11,   -1,    -1,    0,           0,   115200)
'  'SERIAL.AddPort(1,   12,   13,   -1,    -1,    0,           0,   115200)
'  'SERIAL.AddPort(2,   14,   15,   -1,    -1,    0,           0,   115200)
'  SERIAL.Start
'  ' Give it some time to start up
  
'  waitcnt(cnt + clkfreq/10)






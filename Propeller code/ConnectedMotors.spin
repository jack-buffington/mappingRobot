{ This code connects the two motors so that if you turn one of them, the other will match its position
}

CON
  _CLKMODE = XTAL1 + PLL16X
  _XINFREQ = 5_000_000
  
OBJ
    PWM         : "PWM.spin"
    SERIAL      : "quadSerialPorts.spin"
    ENCODER     : "encoder.spin"
    PID         : "positionalPID.spin"
    COPY        : "ASMcopyVariable.spin"
    
    
VAR                             
    long    spinRequestedPosition0
    long    spinRequestedPosition1
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
    
    
    PWMstartPin := 12
    
    PWMs[0] := 32
    PWMs[1] := 0
    PWMs[2] := 16
    PWMs[3] := 0

    Pterm := 28      '30
    Iterm := 5      '2
    Dterm := 28      '16
   
    pwm.start(@pwms)
    
    
    spinRequestedPosition0 := 0
    spinRequestedPosition1 := 0
    
    lastEncoder0 := 0
    lastEncoder1 := 0
    
    
    PID.start(@spinRequestedPosition0)
     

    
    repeat

        ' These two lines cause the two motors to lock together and move as if
        ' they are connected with a chain so that moving one will move the other 
        ' and vice versa.  It feels like there is a lot of drag though.
        'spinRequestedPosition0 := spinEncoderPosition1
        'spinRequestedPosition1 := spinEncoderPosition0
        
        ' Predict where the motor will be the next time and command it to go there.
        ' This is much easier to turn and feels like there is a bit of inertial after you let go.  
        ' It is similar to if the motor wasn't powered at all but with twice the inertia.
        mainTemp := spinEncoderPosition1
        spinRequestedPosition0 := ((mainTemp - lastEncoder1)*2) + mainTemp
        lastEncoder1 := mainTemp
        
        mainTemp := spinEncoderPosition0
        spinRequestedPosition1 := ((mainTemp - lastEncoder0)*2) + mainTemp
        lastEncoder0 := mainTemp 
                                       
        waitcnt(cnt + clkfreq / 100)
    
    




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
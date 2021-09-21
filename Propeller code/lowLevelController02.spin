CON
  _CLKMODE = xtal1 + pll16x
  _XINFREQ = 5_000_000

  'MY_LED_PIN = 18 
  
  
  LED =  21
  POWER_ON = 24 ' Normally high.  Set low to turn off the system power. 
  BUTTON = 26 ' 0 when button is pushed
  SPEAKER = 0
  PROCESSOR_ENABLE = 2
  MOTOR_ENABLE = 1
  PWM0PIN = 17
  PWM1PIN = 18
  PWM2PIN = 19
  PWM3PIN = 20
  CPU_RX = 23
  CPU_TX = 22
  CPU_POWER_SIGNAL = 9



OBJ
    'Motor control stuff
    PWM         : "PWM.spin"
    SERIAL      : "quadSerialPorts.spin"
    ENCODER     : "encoder.spin"
    PID         : "velocityWithAccelerationPID.spin"
    ADC         : "sigmaDelta.spin"   
    I2C        : "I2C PASM driver v1.8od.spin"
    OLED       : "SSD1306_fast.spin"                ' This one doesn't take a COG
    

    

VAR
    '=================================================================================================
    ' This group of variables is used by the PWM, encoder, and PID programs and must be kept together.                        
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
    long    PWM0
    long    PWM1
    long    PWM2
    long    PWM3
    long    PWMstartPin
    '=================================================================================================
    
    long    stillWorking
    long    waitTime
    long    temp
    long    temp1
    long    temp2
    long    temp3
    long    temp4
    long    batteryValue
    long    bytesInMessage
    long    messageType
    long    messageBuffer[32]
    long    bufferPos
    long    checksum
    long    lastButtonState
    long    buttonPushes
    long    lastPushTime
    long    countUpdated


PUB main
  OUTA[POWER_ON] := 1 ' Make sure the power stays on  
  DIRA[POWER_ON] := 1 ' Make the pin be an output

  DIRA[LED] := 1 ' Set the LED pin to an output
  DIRA[SPEAKER] := 1
  DIRA[BUTTON] := 0 ' this is an input
  DIRA[MOTOR_ENABLE] := 1
  DIRA[PROCESSOR_ENABLE] := 1
  DIRA[CPU_POWER_SIGNAL] := 0 ' Make it an input

  
  
  OUTA[MOTOR_ENABLE] := 1 'Turn on the motor power
  OUTA[PROCESSOR_ENABLE] := 1 'Turn on the CPU
  
 
  start_uarts
  serial.str(0,string("Starting up...",13))
  
  
  I2C.start(28, 29, 400000)
  oled.start($3C) ' The I2C start command is tucked into here

  oled.displayOn
  serial.str(0,string("Display is now on",13))
    
  oled.setPageMode
  oled.setTextRowCol(0,0)
  oled.putString(string(" Mapping robot  "))
  oled.setTextRowCol(1,0)
  oled.putString(string(" Waiting for CPU"))
  oled.setTextRowCol(2,0)
  oled.putString(string("                "))
  oled.setTextRowCol(3,0)
  oled.putString(string("                "))
  

  ADC.Start_ADC(@batteryValue)

  playHappyBeeps
  
  
  ' Set up stuff for PWM
  PWMstartPin := 17
  pwm.start(@PWM0)
  
  ' Set up stuff for the encoders
  numberOfEncoders := 2     ' I want two encoders
  encoderBasePin := 5       ' Encoder 1 attached to pins 5 & 6  Encoder 2 attached to pins 7 & 8
  spinEncoderPosition0 := 0     ' Initial encoder value
  spinEncoderPosition1 := 0                           
  ENCODER.start(@numberOfEncoders) 
  
  
  ' Start up the velocity controller
  Pterm := 10      
  Iterm := 6
  Dterm := 30
  spinRequestedVelocity0 := 0
  spinRequestedVelocity1 := 0
  spinAcceleration := 5000
  PID.start(@spinAcceleration)
  
  stillWorking := 1
  
  lastButtonState := 0
  buttonPushes := 0
  
  countUpdated := 0
  lastPushTime := CNT  
  
  repeat
      
    if (lastButtonState == 0) & (INA[BUTTON] == 1)
        lastButtonState := 1
        
    elseif (lastButtonState == 1) & (INA[BUTTON] == 0)
        lastButtonState := 0
        buttonPushes := buttonPushes + 1    
        lastPushTime := CNT
        countUpdated := 0
        
    ' The code above is correctly changing buttonPushes
    
    

    if (CNT > (lastPushTime + CLKFREQ)) & (countUpdated <> 1)
        oled.setTextRowCol(3,14)
        oled.putInt(buttonPushes, 1, 1)
        buttonPushes := 0
        countUpdated := 1
        
        
        
    
    
    temp := Serial.nonblockingRX(1) 
    
    if temp == 85  ' 0x55
        ' Then it has found the first character.
        bytesInMessage := serial.rx(1)
        bufferPos := 0
        checksum := 0
        repeat bytesInMessage
            temp := serial.rx(1)
            checksum ^= temp
            messageBuffer[bufferPos] := temp
            bufferPos += 1
        
        'At this point the message is fully received.   Check the checksum.
        if checksum == 0
            case messageBuffer[0]
            
                0:  ' #########################
                    ' #### Display Message ####
                    ' #########################
                    
                    temp1 := messageBuffer[1] ' row
                    temp2 := messageBuffer[2] ' column
                    oled.setTextRowCol(temp1, temp2)
                    
                    bufferPos := 3
                    repeat bytesInMessage - 4 ' -2 because of message type, row, column, and checksum 
                        oled.putchar(messageBuffer[bufferPos])
                        bufferPos += 1
                
                1:  ' ###############
                    ' #### Beeps ####
                    ' ###############
                    oled.setTextRowCol(1,0)
                    oled.putString(string("Beeps           "))
                    case messageBuffer[1]
                        0: 'Happy beeps    
                            'oled.setTextRowCol(2,0)
                            'oled.putString(string("Happy           "))
                            playHappyBeeps
                        1: 'Sad beeps      
                            'oled.setTextRowCol(2,0)
                            'oled.putString(string("Sad             "))
                            playSadBeeps
                            
                3:  ' ######################
                    ' #### Drive motors ####
                    ' ######################
                    
                    oled.setTextRowCol(1,0)
                    oled.putString(string("Drive motors    "))
                    temp1 := messageBuffer[1]
                    temp2 := messageBuffer[2]
                    temp3 := messageBuffer[3]
                    temp4 := messageBuffer[4]
                    spinRequestedVelocity0 := (temp4 << 24) + (temp3<< 16) + (temp2 << 8) + temp1
                    temp1 := messageBuffer[5]
                    temp2 := messageBuffer[6]
                    temp3 := messageBuffer[7]
                    temp4 := messageBuffer[8]
                    spinRequestedVelocity1 := -((temp4 << 24) + (temp3 << 16) + (temp2 << 8) + temp1)
                    'oled.setTextRowCol(2,0)
                    'oled.putInt(spinRequestedVelocity0, 8, 1)
                    'oled.setTextRowCol(3,0)
                    'oled.putInt(spinRequestedVelocity1, 8, 1)
                
                5:  ' ###########################
                    ' #### CPU shutting down ####
                    ' ###########################
                    
                    ' This is called just before shutting down.  The MCU will then watch the CPU_POWER_SIGNAL
                    ' to see when it goes low.  Once it does then the CPU is fully off and it can shut down.
                    
                    
                    repeat while INA[CPU_POWER_SIGNAL] == 1
                        oled.setTextRowCol(1,0)
                        oled.putString(string(" Shutting down  "))
                        waitcnt(CNT + CLKFREQ/4)
                        
                    OUTA[POWER_ON] := 0     ' Shut the system down
  
                
                6:  ' #################################
                    ' #### Request battery voltage ####
                    ' #################################
                    ' 
                    'oled.setTextRowCol(1,0)
                    'oled.putString(string("Request Voltage "))
                    
                    temp1 := readBattery
                    
                    ' Build the message that will be sent back to the CPU
                    messageBuffer[0] := 85
                    messageBuffer[1] := 6
                    messageBuffer[2] := 2
                    messageBuffer[3] := temp1 & 255 
                    messageBuffer[4] := (temp1 >>= 8) & 255
                    messageBuffer[5] := (temp1 >>= 16) & 255
                    messageBuffer[6] := (temp1 >>= 24) & 255
                    
                    'oled.setTextRowCol(2,0)
                    'oled.putInt(messageBuffer[3],3,1)
                    'oled.putString(string(" "))
                    'oled.putInt(messageBuffer[4],3,1)
                    'oled.putString(string(" "))
                    'oled.putInt(messageBuffer[5],3,1)
                    'oled.putString(string(" "))
                    'oled.putInt(messageBuffer[6],3,1)
                    
                    
                    temp2 := 0
                    repeat temp1 from 2 to 6
                        temp2 ^= messageBuffer[temp1]
                        
                    messageBuffer[7] := temp2 ' This is the checksum
                    
                    ' Send the message
                    repeat temp from 0 to 7
                        serial.tx(1,messageBuffer[temp])
                        
                7:  ' ###################
                    ' #### CPU Ready ####
                    ' ###################
                    
                    oled.setTextRowCol(1,0)
                    oled.putString(string("    CPU Ready   "))
  
  
                  


PUB start_uarts
  SERIAL.init                    
  'AddPort      (port,rxpin,txpin,ctspin,rtspin,rtsthreshold,mode,baudrate)
  SERIAL.AddPort(0,   31,   30,   -1,    -1,    0,           0,   115200)
  SERIAL.AddPort(1, CPU_RX, CPU_TX,   -1,    -1,    0,           0,   115200)

  SERIAL.Start
  ' Give it some time to start up
  
  waitcnt(cnt + clkfreq/10)
  
PUB playHappyBeeps
    beep(1000,250)
    beep(1200,250)
    
PUB playSadBeeps
    beep(300,500)
    beep(200,750)
        
        
PUB beep(frequency, durationInMillis)
    temp := CLKFREQ / 1000
    frequency *= 2
    waitTime := CNT + temp * durationInMillis
    repeat while waitTime > CNT
        OUTA[speaker] := 1
        waitcnt(CLKFREQ/frequency + CNT)
        OUTA[speaker] := 0
        waitcnt(CLKFREQ/frequency + CNT)




PUB readBattery : voltage 
    ' In order to read the battery, the ADC cog must have been started.   
    ' 
    ' Likely use case:
    ' oled.setTextRowCol(3,4)
    ' temp := readBattery
    ' oled.putFloat(temp, 2)
    ' oled.putString(string(" V"))
    ' The formula for the battery voltage is (ADC - 8096) / 841.66
     voltage := ((batteryValue - 8096) * 1000) / 842
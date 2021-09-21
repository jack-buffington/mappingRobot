' The difference between this version and version 2 is that this version
' doesn't block when it is looking for serial data so other things can happen
' simultaneously.

CON
    _CLKMODE = xtal1 + pll16x
    _XINFREQ = 5_000_000
    
    BATTERY_DEAD = 14000    ' This is the voltage where my Dewalt drill shuts off
    BATTERY_LOW = 14300     ' This is the voltage where it tells the CPU to shut down
    BATTERY_FULL = 20000
    
    
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
    CPU_POWER_SIGNAL = 9    ' high when power is on.



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
    long    bufferPos
    long    checksum
    long    lastButtonState
    
    long    buttonPushes
    long    lastPushTime
    long    countUpdated
    
    long    serialRXstate
    long    serialMessageLength
    long    serialBytesReceived
    long    serialBuffer[30]
    long    serialChecksum

    long    shutdownMessageHasBeenSent



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
  
  shutdownMessageHasBeenSent := 0
 
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
  
  
  serialRXstate := 0
  
  'waitcnt(CNT + CLKFREQ * 13) ' delay while the boot messages are coming out of the serial port
  
  
  
  'This is the main event loop
  repeat 
    monitorButton
    lookForSerialMessage
    monitorVoltage
    lookForCPUshutdown
        
    
    
    
  
  
                  


PUB start_uarts
  SERIAL.init                    
  'AddPort      (port,rxpin,  txpin,    ctspin,rtspin,rtsthreshold,mode,baudrate)
  SERIAL.AddPort(0,   31,     30,       -1,    -1,    0,           0,   115200)
  SERIAL.AddPort(1,   CPU_RX, CPU_TX,   -1,    -1,    0,           0,   115200)

  SERIAL.Start
  ' Give it some time to start up
  
  waitcnt(cnt + clkfreq/10)
  
  
  
PUB playHappyBeeps
    beep(1000,250)
    beep(1200,250)
    
    
    
PUB playSadBeeps
    beep(300,500)
    beep(200,750)
    
        
        
PUB beep(frequency, durationInMillis) | halfPeriod, cycleCount, totalCycles
    ' Beeps at the requested frequency for the requested duration.  
    ' KNOWN ISSUE:  This hogs the processor while beeping so button presses can be missed.     
        
    ' Because beeping could fail if I just keep toggling until CNT has passed a value (in the event of a 
    ' rollover) I am implementing this so that it figures out how many toggles it should do and then 
    ' does that many.   This way, it will never simply not beep.  
    
    halfPeriod := CLKFREQ / (frequency * 2)
    
    ' # of toggles = (durationInMillis/1000 * frequency) * 2
    totalCycles := (durationInMillis * frequency) / 1000
    
    cycleCount := 0
    repeat while cycleCount < totalCycles
        OUTA[speaker] := 1
        waitcnt(halfPeriod + CNT)
        OUTA[speaker] := 0
        waitcnt(halfPeriod + CNT)
        cycleCount := cycleCount + 1





PUB monitorButton
    ' Checks the button state and counts how many times it has been pushed in the past second
    ' Once the button hasn't been pushed in the past second then it sends the count to the CPU
    
    if (lastButtonState == 0) & (INA[BUTTON] == 1)
        lastButtonState := 1
        
    elseif (lastButtonState == 1) & (INA[BUTTON] == 0)
        lastButtonState := 0
        buttonPushes := buttonPushes + 1    
        lastPushTime := CNT
        countUpdated := 0
        
    ' The code above is correctly changing buttonPushes
    
    

    if (CNT > (lastPushTime + CLKFREQ)) & (countUpdated <> 1)
        'oled.setTextRowCol(3,14)
        'oled.putInt(buttonPushes, 1, 1)
        
        
        '0x55 <bytes in message> 0x01 <number of button presses> <checksum>
        serialBuffer[0] := 85
        serialBuffer[1] := 3
        serialBuffer[2] := 1
        serialBuffer[3] := buttonPushes
        
        ' Compute the checksum
        temp2 := 0
        repeat temp1 from 2 to 3
            temp2 ^= serialBuffer[temp1]
        
        serialBuffer[4] := temp2
        
        ' Send the message
        repeat temp from 0 to 4
            serial.tx(1,serialBuffer[temp])   
            
        ' Reset the count
        buttonPushes := 0
        countUpdated := 1




PUB monitorVoltage | voltage
    voltage := ((batteryValue - 8096) * 1000) / 842 ' battery voltage * 1000
    if voltage < BATTERY_DEAD
        OUTA[POWER_ON] := 0     ' Shut the power off with no questions asked
    if voltage < BATTERY_LOW
        ' Tell the CPU to shut down 
        if shutdownMessageHasBeenSent == 0
            sendShutdownMessage
            shutdownMessageHasBeenSent := 1
        
        
        
PUB sendShutdownMessage
    serialBuffer[0] := 85
    serialBuffer[1] := 2
    serialBuffer[2] := 0
    serialBuffer[3] := 0
    
    ' Send the message
    repeat temp from 0 to 3
        serial.tx(1,serialBuffer[temp])   

PUB readBattery : voltage 
    ' In order to read the battery, the ADC cog must have been started.   
    ' The value returned is voltage * 1000
    ' 
    ' Likely use case:
    ' oled.setTextRowCol(3,4)
    ' temp := readBattery
    ' oled.putFloat(temp, 2)
    ' oled.putString(string(" V"))
    ' The formula for the battery voltage is (ADC - 8096) / 841.66
     voltage := ((batteryValue - 8096) * 1000) / 842
     
     
     
PUB lookForCPUshutdown
    ' This checks to see if the CPU has shut down by watching a GPIO pin.  If it has then it shuts 
    ' off all power to the system.
    if INA[CPU_POWER_SIGNAL] == 0
        OUTA[POWER_ON] := 0
     
     
     
     
     
     
PUB lookForSerialMessage | theByte
    ' This function is a non-blocking function that looks for messages coming into the serial port.
    
    theByte := serial.rxcheck(1)
    if theByte == -1  ' If there was nothing in the RX buffer
        return
    
    
    ' Below here is executed if there was a byte in the buffer   
    ' 
    ' echo the byte to the PC for debugging purposes
    'serial.dec(0,theByte)  
    'serial.tx(0,13)  
        
      
    
    case serialRXstate
    
        0:  ' ### START BYTE ###
            if theByte == 171 ' then it is 0xAA so move to the next state
                serialRXstate := 1
                'oled.setTextRowCol(3,0)
                'oled.putString(string("S               ")) ' Clear the line.
                serial.str(0,string("S "))
            
        1:   ' ### BYTES IN MESSAGE ###
            serialMessageLength := theByte
            serialRXstate := 2
            serialBytesReceived := 0
            serialChecksum := 0
            serial.dec(0, serialMessageLength)
            serial.str(0,string(" ",13))
            'oled.setTextRowCol(3,1)
            'oled.putInt(serialMessageLength, 2, 2)
        
        2:   ' ### RECEIVE MESSAGE AND COMPUTER CHECKSUM SIMULTANEOUSLY ###
            serialBuffer[serialBytesReceived] := theByte
            serialBytesReceived := serialBytesReceived + 1
            serialChecksum ^= theByte
            'oled.putString(string("B"))
            serial.str(0,string("*"))
            
            if serialBytesReceived == serialMessageLength
                

            
                if serialChecksum == 0
                    ' Then the checksum worked out correctly.  Go ahead and parse the message
                    'oled.putString(string("C"))
                    'oled.putInt(serialBuffer[0],1,1)
                    '
                    
                    serial.str(0,string(" Passed Checksum",13))
                    
                    case serialBuffer[0]
                
                        0:  ' #########################
                            ' #### Display Message ####
                            ' #########################
                            
                            
                            temp1 := serialBuffer[1] ' row
                            temp2 := serialBuffer[2] ' column
                            
                            serial.str(0,string("Row: "))                           
                            serial.dec(0,temp1)
                            serial.str(0,string(" Column: "))                        
                            serial.dec(0,temp2)   
                            serial.str(0,string(" Bytes in message: "))                        
                            serial.dec(0,serialMessageLength)   
                            serial.str(0,string(" ", 13))
                                                   
                            oled.setTextRowCol(temp1, temp2)
                            
                            bufferPos := 3
                            repeat serialMessageLength - 4 ' -4 because of message type, row, column, and checksum 
                                oled.putchar(serialBuffer[bufferPos])
                                bufferPos += 1
                        
                        1:  ' ###############
                            ' #### Beeps ####
                            ' ###############
                            'oled.setTextRowCol(1,0)
                            'oled.putString(string("Beeps           "))
                            case serialBuffer[1]
                                0: playHappyBeeps
                                1: playSadBeeps
                                    
                        3:  ' ######################
                            ' #### Drive motors ####
                            ' ######################
                            
                            'oled.setTextRowCol(1,0)
                            'oled.putString(string("Drive motors    "))
                            temp1 := serialBuffer[1]
                            temp2 := serialBuffer[2]
                            temp3 := serialBuffer[3]
                            temp4 := serialBuffer[4]
                            spinRequestedVelocity0 := (temp4 << 24) + (temp3<< 16) + (temp2 << 8) + temp1
                            temp1 := serialBuffer[5]
                            temp2 := serialBuffer[6]
                            temp3 := serialBuffer[7]
                            temp4 := serialBuffer[8]
                            spinRequestedVelocity1 := -((temp4 << 24) + (temp3 << 16) + (temp2 << 8) + temp1)
          
                        
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
                            serialBuffer[0] := 85
                            serialBuffer[1] := 6
                            serialBuffer[2] := 2
                            serialBuffer[3] := temp1 & 255 
                            serialBuffer[4] := (temp1 >>= 8) & 255
                            serialBuffer[5] := (temp1 >>= 16) & 255
                            serialBuffer[6] := (temp1 >>= 24) & 255
        
                            
                            temp2 := 0
                            repeat temp1 from 2 to 6
                                temp2 ^= serialBuffer[temp1]
                                
                            serialBuffer[7] := temp2 ' This is the checksum
                            
                            ' Send the message
                            repeat temp from 0 to 7
                                serial.tx(1,serialBuffer[temp])
                                
                        7:  ' ###################
                            ' #### CPU Ready ####
                            ' ###################
                            
                            oled.setTextRowCol(1,0)
                            oled.putString(string("    CPU Ready   "))
                            
                            
                            
                ' Get ready for next time.
                serialRXstate := 0
                serialMessageLength := 0
                serialBytesReceived := 0
                
        
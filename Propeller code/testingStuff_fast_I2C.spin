CON
  _CLKMODE = XTAL1 + PLL16X
  _XINFREQ = 5_000_000
  
OBJ
    'PWM         : "PWM.spin"
    'BLINK       : "blink.spin"
    'SIG_DELT    : "sigmaDelta.spin"
    SERIAL      : "quadSerialPorts.spin"
    'ENCODER     : "encoder.spin"
    i2cfast     : "I2C PASM driver v1.8od.spin"
    OLED        : "SSD1306_fast.spin"
    
    
VAR
long    whichPin
long    LEDvalues[4]   
long    potentiometer
long    temp
long    time

long    numberOfEncoders
long    encoderBasePin
long    encoderPosition
long    encoderPosition2
long    encoderPosition3
byte    characterBuffer[16]
long    myStringPtr
long    A

byte    buff[16]

    
PUB launchStuff
''-------- This Block is what runs at startup ----------

    start_uarts
    SERIAL.tx(0,13)                ' the basic i/o methods are part of fullDuplexSerial4port

    i2cfast.start(20,21,1000000)    

    serial.str(0,string("Starting up the SSD1306 ...",13))
    
    oled.start($3C) ' The I2C start command is tucked into here

    oled.displayOn
    serial.str(0,string("Display is now on",13))

    oled.setPageMode
    oled.setTextRowCol(0,0)
    oled.putString(string("This is the"))
    oled.setTextRowCol(1,0)
    oled.putString(string("first display"))
    oled.setTextRowCol(2,0)
    oled.putString(string("################"))
    oled.setTextRowCol(3,0)
    oled.putString(string("################"))
    serial.str(0,string("Done putting string",13))
    
    waitcnt(cnt + clkfreq * 5)
    
    oled.clearDisplay 
    serial.str(0,string("Done clearing the screen",13))





    ' Second screen:  
    oled.start128x64($3D)
    oled.displayOn
    oled.setPageMode
    oled.clearDisplay
    
    
    waitcnt(cnt + clkFreq / 2)
    oled.setTextRowCol(0,0)
    oled.putString(string("================"))  ' equal signs because I may have code wrong
    oled.setTextRowCol(1,0)                     ' for 128x64.  Only every other row is showing
    oled.putString(string("================"))  ' on these two lines.
    oled.setTextRowCol(2,0)
    oled.putString(string("Line 2    Line 2"))
    oled.setTextRowCol(3,0)
    oled.putString(string("Line 3    Line 3"))
    oled.setTextRowCol(4,0)
    oled.putString(string("Line 4    Line 4"))
    oled.setTextRowCol(5,0)
    oled.putString(string("Line 5    Line 5"))
    oled.setTextRowCol(6,0)
    oled.putString(string("Line 6    Line 6"))
    oled.setTextRowCol(7,0)
    oled.putString(string("Line 7    Line 7"))
    
    serial.str(0,string("Done putting string",13))
    
    waitcnt(cnt + clkfreq * 5)
    oled.clearDisplay 
  
        

PUB start_uarts
  SERIAL.init                    
  'AddPort      (port,rxpin,txpin,ctspin,rtspin,rtsthreshold,mode,baudrate)
  SERIAL.AddPort(0,   10,   11,   -1,    -1,    0,           0,   115200)
  SERIAL.AddPort(1,   12,   13,   -1,    -1,    0,           0,   115200)
  SERIAL.AddPort(2,   14,   15,   -1,    -1,    0,           0,   115200)
  SERIAL.Start
  ' Give it some time to start up
  time := cnt
  waitcnt(time += clkfreq/10)


CON
  _CLKMODE = xtal1 + pll16x
  _XINFREQ = 5_000_000

  'MY_LED_PIN = 18 
  
  
  LED =  21
  POWER_ON = 24 ' Normally high.  Set low to turn off the system power. 
  BUTTON = 26 ' pulsed low by the LTC2954 when a button press happens.
  SPEAKER = 0
  PROCESSOR_ENABLE = 2
  MOTOR_ENABLE = 1

PUB main
  OUTA[POWER_ON] := 1 ' Make sure the power stays on  
  DIRA[POWER_ON] := 1 ' Make the pin be an output

  DIRA[LED] := 1 ' Set the LED pin to an output
  DIRA[SPEAKER] := 1
  DIRA[BUTTON] := 0 ' this is an input
  DIRA[MOTOR_ENABLE] := 1
  DIRA[PROCESSOR_ENABLE] := 1
  
  
  OUTA[MOTOR_ENABLE] := 1 'Turn on the motor power
  OUTA[PROCESSOR_ENABLE] := 1 'Turn on the CPU



  repeat ' Repeat forever
    OUTA[LED] := 1  ' Turn the LED on
    'OUTA[SPEAKER] := 1
    waitcnt(cnt + clkfreq/1000) ' Wait 1 second
    OUTA[LED] := 0  ' Turn the LED off
    'OUTA[SPEAKER] := 0
    waitcnt(cnt + clkfreq/1000) ' Wait 1 second
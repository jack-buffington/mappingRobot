VAR
    long    oledAddress  
    byte    buff[16]
   
        
              
OBJ
    i2cfast     : "I2C PASM driver v1.8od.spin"
    font        : "font.5x8.spin"  
        
    
PUB start(address) 
    ' This must be called before anything else
    oledAddress := address
    
    'i2cfast.start(20,21,900000) '  ######################################## I have to put this here
    
    ' The Adafruit Arduino driver does this at startup:
    buff[0] := $AE
    buff[1] := $D5
    buff[2] := $80
    buff[3] := $A8
    i2cfast.writeBytes(address, $00, @buff, 4)
    
    
    waitcnt(cnt + 10000)
    i2cfast.writeByte(address, $00, $1F)
    
    waitcnt(cnt + 10000)
    buff[0] := $D3
    buff[1] := $00
    buff[2] := $40
    buff[3] := $8D
    i2cfast.writeBytes(address, $00, @buff, 4)
    
    
    waitcnt(cnt + 10000)
    i2cfast.writeByte(address, $00, $14)
    
    
    waitcnt(cnt + 10000)  
    buff[0] := $20
    buff[1] := $00
    buff[2] := $A1
    buff[3] := $C8
    i2cfast.writeBytes(address, $00, @buff, 4)  
      
    waitcnt(cnt + 10000)
    i2cfast.writeByte(address, $00, $DA)
    
    waitcnt(cnt + 10000) 
    i2cfast.writeByte(address, $00, $02)
    
    waitcnt(cnt + 10000)
    i2cfast.writeByte(address, $00, $81)
    
    waitcnt(cnt + 10000)
    i2cfast.writeByte(address, $00, $8F) 
    
    waitcnt(cnt + 10000)
    i2cfast.writeByte(address, $00, $D9)  
    
    waitcnt(cnt + 10000)
    i2cfast.writeByte(address, $00, $F1)  
    
    
    
    
    

PUB start128x64(address) 
    ' This must be called before anything else
    oledAddress := address

    i2cfast.writeByte(address, $80, $AE)    ' Display off       
                                                            
    i2cfast.writeByte(address, $80, $D4)    ' Set display clock divide ratio / osc frequency
    i2cfast.writeByte(address, $80, $80)    ' display clock divide ratio / osc frequency
                            
    i2cfast.writeByte(address, $80, $A8)    ' Set Multiplex ratio
    i2cfast.writeByte(address, $80, $3F)    ' Multiplex ratio for 128x64 (64 - 1)                                          
                                               
    i2cfast.writeByte(address, $80, $D3)    ' Set display offset
    i2cfast.writeByte(address, $80, $00)    ' Display offset                                      

    i2cfast.writeByte(address, $80, $40)    ' Set display start line

    i2cfast.writeByte(address, $80, $8D)    ' Set Charge pump 
    i2cfast.writeByte(address, $80, $14)    ' charge pump internal                                   
                                                                               
                                            ' This section deals with flipping of the screen
    i2cfast.writeByte(address, $80, $3F)    ' Set segment remap A1: normal A0: flipped L/R
    i2cfast.writeByte(address, $80, $3F)    ' Set Com Output scan direction C8: Normal C0: Flipped U/D

    i2cfast.writeByte(address, $80, $DA)    ' Set COM hardware configuration
    i2cfast.writeByte(address, $80, $12)    ' COM hardware configuration
    
    i2cfast.writeByte(address, $80, $81)    ' Set Contrast
    i2cfast.writeByte(address, $80, $CF)    ' Contrast

    i2cfast.writeByte(address, $80, $D9)    ' Set Pre charge period
    i2cfast.writeByte(address, $80, $F1)    ' Pre charge period internal

    i2cfast.writeByte(address, $80, $DB)    ' Set VCOMH Dselect level
    i2cfast.writeByte(address, $80, $40)    ' VCOMH Dselect level

    i2cfast.writeByte(address, $80, $A4)    ' Set all pixels off
    i2cfast.writeByte(address, $80, $A6)    ' Set display not inverted
    i2cfast.writeByte(address, $80, $AF)    ' Set display on
                                              

PUB displayOff
    i2cfast.writeByte(oledAddress, $80, $AE)
    
PUB entireDisplayOn 
    i2cfast.writeByte(oledAddress, $80, $A5) 
    
PUB displayOn
    i2cfast.writeByte(oledAddress, $80, $AF)
    
PUB setDisplayNormal
    i2cfast.writeByte(oledAddress, $80, $A6)

PUB setDisplayInverted
    i2cfast.writeByte(oledAddress, $80, $A7)
    
PUB setPageMode
    ' Used for text.  This mode orients the bytes coming in vertically and progresses 
    ' horizontally along a row. When it reaches the end of a row, it jumps back to the
    ' beginning of the same row.
    i2cfast.writeByte(oledAddress, $80, $20)
    i2cfast.writeByte(oledAddress, $80, $02)
    
PUB setHorizontalMode
    ' Used for graphics. This is very similar to page mode except when it reaches the
    ' end of a row, it jumps to the first column of the next row.
    i2cfast.writeByte(oledAddress, $80, $20)
    i2cfast.writeByte(oledAddress, $80, $00)
    
 
PUB setVerticalMode
    ' Used for graphics.  Each byte is placed in eight pixels within a single column.  
    ' After each write, the row is incremented.  When a write happens on the bottom row, 
    ' the column is incremented and it jumps back to the top row. 
    i2cfast.writeByte(oledAddress, $80, $20)
    i2cfast.writeByte(oledAddress, $80, $01)
    
PUB setTextRowCol(row,col) | temp
    ' row and col start at 0 and are in 8x8 blocks. Position will be specified by 
    ' row (0-7) for Y and by column (0-15) for X
    ' You will generally use this when in page mode or horizontal mode. 
    ' There are 4 or 8 rows and 16 columns
    row += $B0    
    i2cfast.writeByte(oledAddress, $80, row)    'Set the row                                           
                
                                               
    col *= 8
    temp := col >> 4   ' This is the high nibble
    col &= 15         ' This is the low nibble    
    i2cfast.writeByte(oledAddress, $80, col)    'Set the low nibble of the column                                              
    temp += 16         'Set bit 4   
    i2cfast.writeByte(oledAddress, $80, temp)    'Set the high nibble of the column           



PUB setBrightness(theBrightness)
    'theBrightness is in the range of 0-255
    i2cfast.writeByte(oledAddress, $80, $81)
    i2cfast.writeByte(oledAddress, $80, theBrightness)


PUB putChar(theChar) | temp
    theChar *= 8    ' Now offset from start of table
    theChar += font.BaseAddr ' Now an actual address in the table

    repeat 8
        i2cfast.writeByte(oledAddress, $40, byte[theChar])
        theChar += 1



PUB putString(theString) | numChars, p
    numChars := strsize(theString)

    repeat p from 0 to numChars - 1
        putChar(byte[theString++])



PUB clearDisplay | row, col
    repeat row from 0 to 7
        repeat col from 0 to 15
            setTextRowCol(row,col)
            putChar(" ")
            
            


PUB putFloat(theFloatAsLong, W) | intPart, fractionalPart, temp
    'theFloatAsLong is the float * 1000 stored in an int.
    'Prints XXX.YYYY where XXX is W whole digits and YYYY is 4 fractional digits
    intPart := theFloatAsLong / 1000
    fractionalPart := theFloatAsLong - (intPart * 1000)
    
    ' print out the int part
    putInt(intPart, W, 1)
    putChar(".")
    putInt(fractionalPart, 3, 2)     



PUB putInt(value, digits, flag) | i
    ' Displays an int with the specified number of digits.
    ' flag is 1: spaces instead of leading zeros
    '         2: leading zeros


  digits := 1 #> digits <# 10
  if value < 0
    -value
    'tx(port,"-")
    'i2cfast.writeByte(oledAddress, $40, byte["-"])
    putChar("-")

  i := 1_000_000_000
  if flag & 3
    if digits < 10                                      ' less than 10 digits?
      repeat (10 - digits)                              '   yes, adjust divisor
        i /= 10

  repeat digits
    if value => i
      'tx(port,value / i + "0")
      'i2cfast.writeByte(oledAddress, $40, value / i + "0")
      putChar(value / i + "0")
      value //= i
      result~~
    elseif (i == 1) OR result OR (flag & 2)
      'tx(port,"0")
      'i2cfast.writeByte(oledAddress, $40,  "0")
      putChar("0")
    elseif flag & 1
      'tx(port," ")
      'i2cfast.writeByte(oledAddress, $40, " ")
      putChar(" ")
    i /= 10

        
       
    'TODO
 
PUB plotPixel(X,Y, color)
    ' color: 0 = black, 1 = white
    'TODO


PUB drawLine(X1,X2,Y1,Y2,color)
    ' color: 0 = black, 1 = white
    'TODO

PUB fillRegion(X1,X2,Y1,Y2,color)
    ' color: 0 = black, 1 = white
    'TODO

PUB updateScreen
    ' Sends all of the data in the buffer to the screen
    'TODO

PUB drawCharacter(X,Y,theCharacter, color)
    ' This is for when doing graphics too
    ' color: 0 = black, 1 = white
    ' theCharacter: and ascii code
    'TODO

PUB drawString(X,Y,theString, color)
    ' This is for when doing graphics too
    ' color: 0 = black, 1 = white
    'TODO   
   
   
   
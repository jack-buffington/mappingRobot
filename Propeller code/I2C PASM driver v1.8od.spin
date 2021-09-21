{{┌──────────────────────────────────────────┐
  │ I2C open-drain PASM driver          1.8od│      This routine requires the use of pull-up resistors on the SCL and SDA lines
  │ Author: Chris Gadd                       │      - Does not work with the EEPROM on the demo board, which only has a pull-up on SDA
  │ Copyright (c) 2014 Chris Gadd            │      - This version supports clock stretching - limits the bitrate to just over 900Kbps
  │ See end of file for terms of use.        │         Commenting out stretch checks allows operation up to 1Mbps
  └──────────────────────────────────────────┘

  PUB methods:
    start(SCL,SDA,BITRATE)                                            ' Start the I2C driver 
    stop
    write(DeviceID,RegisterAddress,SourceAddress,Number,Endian,Size)  ' Master method for all writes
     writeByte   (DeviceID,RegisterAddress,Data)                      '  Write a single byte
     writeWordL             "                                         '  Write a single little-endian word
     writeWordB             "                                         '  Write a single big-endian word
     writeLongL             "                                         '  Write a single little-endinal long
     writeLongB             "                                         '  write a single big-endian long
     writeBytes  (DeviceID,RegisterAddress,SourceAddress,Number)      '  Write many bytes
     writeWordsL            "                                         '  Write many little-endian words
     writeWordsB            "                                         '  Write many big-endian words
     writeLongsL            "                                         '  Write many little-endian longs
     writeLongsB            "                                         '  Write many big-endian longs
    read(DeviceID,RegisterAddress,DestAddress,Number,Endian,Size)     ' Master method for all reads
     readByte    (DeviceID,RegisterAddress)                           '  Read a single byte
     readWordL              "                                         '  Read a single little-endian word 
     readWordB              "                                         '  Read a single big-endian word    
     readLongL              "                                         '  Read a single little-endian long
     readLongB              "                                         '  Read a single big-endian long
     readBytes   (DeviceID,RegisterAddress,DestAddress,Number)        '  Read many bytes                         
     readWordsL             "                                         '  Read many little-endian words           
     readWordsB             "                                         '  Read many big-endian words              
     readLongsL             "                                         '  Read many little-endian longs           
     readLongsB             "                                         '  Read many big-endian longs              

    arbitrary(Out_address,Out_count,In_address,In_count)              ' General purpose method for edge-cases - bytes are sent exactly as they're entered into the out_buffer
     readNext(DeviceID)                                               '  Sends the deviceID with the read bit set, and returns 1 byte
     command(DeviceID,command)                                        '  Pressure sensors take single byte instructions
     
    This routine performs ACK polling to determine when a device is ready.
    Routine will abort a transmission if no ACK is received within 10ms of polling - prevents I2C routine from stalling if a device becomes disconnected
    No other ACK testing is performed                                                                           ┌──────────────────────────────────────────┐
    All methods except single reads return true if the operation was successful, false if no response           │ if not I2C.writeByte(EEPROM,$0123,$01)   │
     -single reads return the requested value                                                                   │   FDS.str(string("EEPROM not present"))  │
                                                                                                                └──────────────────────────────────────────┘
    This routine automatically uses two bytes when addressing an EEPROM.  The EEPROM is the only device, so far discovered, that uses two-byte addresses.
                 
'----------------------------------------------------------------------------------------------------------------------
  This object uses a four step count for every bit sent.
    T0 - Put bit to be sent on the SDA pin if writing, float if reading data or Ack/NAK
    T1 - Float SCL pin (already floating on start)
    T2 - Sample SDA pin if reading data or Ack/NAK, or set/release SDA pin for start/stop 
    T3 - Pull SCL pin low (except on stop)

        ┌─Start─┬─Bit 1─┬─Bit 0─┬─Ack(r)┬─Start─┬─Read──┬─Ack(t)┬─Read──┬──NAK──┬─Stop──┐         
    SCL          
    SDA ─────────────────────  
        0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3    
                                                                                        
                                    sample          sample          sample                 
}}                                                                                                                                                
CON
' Device codes
' Requires the un-shifted 7-bit device address.  The driver shifts the address and appends the read / write bit

  EEPROM = %101_0000            ' Device code for 24LC256 EEPROM with all chip select pins tied to ground
  RTC    = %110_1000            ' Device code for DS1307 real time clock
  ACC    = %001_1101            ' Device code for MMA7455L 3-axis accelerometer
  GYRO   = %110_1001            ' Device code for L3G4200D gyroscope (SDO to Vdd)
  ALT    = %111_0110            ' Device code for MS5607 altimeter (CS floating)

'Jump table offsets  
  _WRITE        = 1
  _READ         = 2
  _ARBITRARY    = 3

  _LITTLE = 0
  _BIG    = 1
  _BYTE   = 1
  _WORD   = 2
  _LONG   = 4
                                    
VAR


PUB start(scl_pin, sda_pin, bitrate) | par_ptr, okay

  stop
  |< scl_pin
  |< sda_pin
  bitrate := clkfreq / (bitrate * 4)
  par_ptr := @parameters[0]  
  okay := cog := cognew(@entry, @scl_pin) + 1
  waitcnt(clkfreq / 1000 + cnt)
  return okay

PUB stop

  if cog
    cogstop(cog~ - 1)

PUB write(deviceID,registerAddress,dataAddress,number,endian,type)              '' Write any number of big or little-endian bytes, words, or longs

  parameters[1] := deviceID << 1
  parameters[2] := registerAddress
  parameters[3] := dataAddress
  parameters[4] := number
  parameters[5] := endian
  parameters[6] := type
  parameters[0] := _WRITE                                                       ' setting parameters[0] initiates the operation - must be set last
  wait_for_ready
  return parameters[0]                                                          ' Returns $FF if successful / $00 if no response from device (device not found)
    
PUB writeByte(deviceID,registerAddress,data)                                    '' Write a single byte from an immediate value
  return write(deviceID,registerAddress,data,1,_LITTLE,_BYTE)
  
PUB writeBytes(deviceID,registerAddress,dataAddress,number)                     '' Write many bytes from an array
  return write(deviceID,registerAddress,dataAddress,number,_LITTLE,_BYTE)

PUB writeWordL(deviceID,registerAddress,data)                                   '' Write a single little-endian word      
  return write(deviceID,registerAddress,data,1,_LITTLE,_WORD)                                                                                
                                                                                                                          
PUB writeWordB(deviceID,registerAddress,data)                                   '' Write a single big-endian word         
  return write(deviceID,registerAddress,data,1,_BIG,_WORD)                                                                                
                                                                                                                          
PUB writeWordsL(deviceID,registerAddress,dataAddress,number)                    '' Write many little-endian words         
  return write(deviceID,registerAddress,dataAddress,number,_LITTLE,_WORD)                                                                                
                                                                                                                          
PUB writeWordsB(deviceID,registerAddress,dataAddress,number)                    '' Write many big-endian words            
  return write(deviceID,registerAddress,dataAddress,number,_BIG,_WORD)                                                                                
                                                                                                                          
PUB writeLongL(deviceID,registerAddress,data)                                   '' Write a single little-endian long      
  return write(deviceID,registerAddress,data,1,_LITTLE,_LONG)                                                                                
                                                                                                                          
PUB writeLongB(deviceID,registerAddress,data)                                   '' Write a single big-endian long         
  return write(deviceID,registerAddress,data,1,_BIG,_LONG)                                                                                
                                                                                                                          
PUB writeLongsL(deviceID,registerAddress,dataAddress,number)                    '' Write many little-endian longs         
  return write(deviceID,registerAddress,dataAddress,number,_LITTLE,_LONG)                                                                                
                                                                                                                          
PUB writeLongsB(deviceID,registerAddress,dataAddress,number)                    '' Write many big-endian longs            
  return write(deviceID,registerAddress,dataAddress,number,_BIG,_LONG)                                                                                

PUB read(deviceID,registerAddress,dataAddress,number,endian,type)               '' Read any number of big or little-endian bytes, words, or longs

  parameters[1] := deviceID << 1
  parameters[2] := registerAddress
  parameters[3] := dataAddress
  parameters[4] := number
  parameters[5] := endian
  parameters[6] := type
  parameters[0] := _READ
  wait_for_ready
  return parameters[3]

PUB readByte(deviceID,registerAddress)                                          '' Read a single byte and return an immediate result
  return read(deviceID,registerAddress,0,1,_LITTLE,_BYTE)
  
PUB readBytes(deviceID,registerAddress,dataAddress,number)                      '' Read many bytes and store in an array
  read(deviceID,registerAddress,dataAddress,number,_LITTLE,_BYTE)
  return parameters[0]                                                          '  Returns $FF if successful / $00 if no response from device (device not found)

PUB readWordL(deviceID,registerAddress)                                         '' Read a single little-endian word
  return read(deviceID,registerAddress,0,1,_LITTLE,_WORD)

PUB readWordB(deviceID,registerAddress)                                         '' Read a single big-endian word
  return read(deviceID,registerAddress,0,1,_BIG,_WORD)

PUB readWordsL(deviceID,registerAddress,dataAddress,number)                     '' Read many little-endian words
  read(deviceID,registerAddress,dataAddress,number,_LITTLE,_WORD)
  return parameters[0]
  
PUB readWordsB(deviceID,registerAddress,dataAddress,number)                     '' Read many big-endian words
  read(deviceID,registerAddress,dataAddress,number,_BIG,_WORD)
  return parameters[0]
  
PUB readLongL(deviceID,registerAddress)                                         '' Read a single little-endian long
  return read(deviceID,registerAddress,0,1,_LITTLE,_LONG)

PUB readLongB(deviceID,registerAddress)                                         '' Read a single big-endian long
  return read(deviceID,registerAddress,0,1,_BIG,_LONG)

PUB readLongsL(deviceID,registerAddress,dataAddress,number)                     '' Read many little-endian longs
  read(deviceID,registerAddress,dataAddress,number,_LITTLE,_LONG)
  return parameters[0]
  
PUB readLongsB(deviceID,registerAddress,dataAddress,number)                     '' Read many big-endian longs
  read(deviceID,registerAddress,dataAddress,number,_BIG,_LONG)
  return parameters[0]
  
PUB arbitrary(Out_address,Out_count,In_address,In_count)                        '' Sends bytes exactly as they're entered into the out_address array 
  parameters[2] := Out_address                                                  '  If sending a device address, it must be shifted to 8 bits and a read/write bit appended
  parameters[3] := In_address                                                   '  Arbitrary method does not perform ack polling
  parameters[4] := (Out_count & $FF) << 8 | (In_count & $FF)
  parameters[0] := _ARBITRARY
  wait_for_ready
  return parameters[0]

PUB command(deviceID,comm) | temp                                               '' Write the deviceID and a single command byte.  Specifically used in pressures sensors
  temp := comm << 8 | deviceID << 1
  return arbitrary(@temp,2,0,0)

PUB readNext(deviceID) : temp                                                   '' Read the next byte.  Sends the Device ID with read bit set
  temp := deviceID << 1 | 1
  arbitrary(@temp,1,@temp,1)

PRI wait_for_ready                                                              '' returns true when ready, false if no response in 10ms
  repeat
    if parameters[0] == $FF
      return true
    if parameters[0] == $00 'or cnt - t > 0                                  
      return false

DAT                     org
entry
                        mov       t1,par
                        rdlong    scl_mask,t1
                        add       t1,#4
                        rdlong    sda_mask,t1
                        add       t1,#4
                        rdlong    bit_delay,t1
                        add       t1,#4
                        rdlong    t1,t1

                        movd      :loop,#command_address
                        mov       loop_counter,#7
:loop                   mov       0-0,t1
                        add       t1,#4
                        add       :loop,d1
                        djnz      loop_counter,#:loop
                        
                        test      SDA_mask,ina                wz
          if_nz         jmp       #main
                        mov       delay_target,bit_delay
                        add       delay_target,cnt
:twiddle
                        waitcnt   delay_target,bit_delay
                        waitcnt   delay_target,bit_delay
                        xor       dira,SCL_mask
                        test      SDA_mask,ina                wz
          if_z          jmp       #:twiddle
'----------------------------------------------------------------------------------------------------------------------
main
                        andn      dira,SCL_mask               
                        rdbyte    command_byte,command_address wz               ' Loop until command is set by a Spin routine
          if_z          jmp       #main
                        cmp       command_byte,#$FF           wz
          if_e          jmp       #main
                        rdlong    device_byte,device_address
                        mov       t1,command_byte  
                        add       t1,#:jump_table                               ' Use value in Command_byte (1-9) to 
:jump_table             jmp       t1                                            '  determine which routine to jump to
                        jmp       #send
                        jmp       #receive
                        jmp       #edge_case
'......................................................................................................................
send
                        rdlong    count,count_address
                        rdlong    size,size_address
                        rdlong    order,order_address                           
                        cmp       count,#1                    wz                
          if_e          mov       t2,data_address                               ' Send an immediate value if writing a single long, word, or byte
          if_ne         rdlong    t2,data_address                               ' Otherwise, the values are retrieved from an array                  
                        call      #Send_start         
:count_loop                                                                     
                        mov       loop_counter,size
                        cmp       size,#_WORD                 wc,wz              
          if_a          rdlong    I2C_data,t2                                   ' Read the data with the proper alignment for page writes
          if_e          rdword    I2C_data,t2
          if_b          rdbyte    I2C_data,t2
                        cmp       order,#_LITTLE              wz
          if_e          jmp       #:size_loop                                   ' sending a little endian doesn't require any preparation
                        mov       t1,size                                       ' for big-endian: rotate right (size - 1) x 8
                        sub       t1,#1                                         '  Rotate longs right by 24   
                        shl       t1,#3                                         '  Rotate words right by 8    
                        ror       I2C_data,t1                                   '  Do not rotate bytes        
:size_loop
                        call      #I2C_write
                        cmp       order,#_BIG                 wz
          if_e          rol       I2C_data,#8                                   ' rotate left 8 if sending big endians
          if_ne         ror       I2C_data,#8                                   ' rotate right 8 if sending little endians                        
                        djnz      loop_counter,#:size_loop
                        add       t2,size
                        djnz      count,#:count_loop
                        call      #I2C_stop
                        jmp       #main                        
'......................................................................................................................
receive
                        rdlong    count,count_address                           ' Number of bytes, words, or longs to read
                        rdlong    size,size_address                             ' Long(4), Word(2), or Byte(1)
                        cmp       size,#_BYTE                 wz
          if_ne         rdlong    order,order_address                           ' Big-endian(1) or little-endian(0)
          if_e          mov       order,#_BIG                                   '  bytes must be processed as big-endian
                        cmp       count,#1                    wz
          if_e          mov       t2,data_address                               ' Return an immediate result if reading a single long, word, or byte
          if_ne         rdlong    t2,data_address                               ' Otherwise, the results are stored in an array
                        call      #send_start                                   ' Send a start bit, device ID, and address
                        call      #I2C_start                                    ' Send a restart
                        mov       I2C_data,device_byte                          ' Send the device ID with read bit set
                        or        I2C_data,#1
                        call      #I2C_write
:count_loop
                        mov       loop_counter,size                             ' loop 4, 2, or 1 for long, word, or byte
:size_loop              
                        call      #I2C_read                                     
                        cmp       order,#_LITTLE              wz                ' Reorder bytes if data is little-endian 
          if_e          rol       I2C_data,#16
                        sub       loop_counter,#1             wz                
          if_nz         call      #I2C_ack                                      ' Send an ack and read another byte for words and longs
          if_nz         jmp       #:size_loop
                        cmp       size,#1                     wz                
          if_e          wrbyte    I2C_data,t2                                   
          if_e          jmp       #:lower                                       
                        cmp       order,#_BIG                 wc               
                        cmp       size,#_WORD                 wz
          if_c_and_z    ror       I2C_data,#8                                   ' rotate little-endian words right 8
          if_z          wrword    I2C_data,t2
          if_z          jmp       #:lower                                       
          if_c          rol       I2C_data,#8                                   ' rotate little-endian longs left 8
                        wrlong    I2C_data,t2                                   
:lower
                        add       t2,size
                        sub       count,#1                    wz
          if_nz         call      #I2C_ack
          if_nz         jmp       #:count_loop
                        call      #I2C_nak
                        call      #I2C_stop
                        jmp       #main
'......................................................................................................................
edge_case
                        rdlong    loop_counter,count_address
                        shr       loop_counter,#8             wz
          if_z          jmp       #:arb_read
                        rdlong    t2,register_address
                        mov       timeout,_10ms
                        add       timeout,cnt
:ack_poll               
                        mov       t1,timeout
                        sub       t1,cnt
                        cmps      t1,#0                       wc
          if_c          jmp       #no_response
                        call      #I2C_start
                        rdbyte    I2C_data,t2
                        call      #I2C_write
          if_c          jmp       #:ack_poll
                        sub       loop_counter,#1             wz
          if_z          jmp       #:arb_read
                        add       t2,#1
:arb_write_loop                       
                        rdbyte    I2C_data,t2
                        call      #I2C_write
                        add       t2,#1
          if_nc         djnz      loop_counter,#:arb_write_loop                 ' Repeat until all bytes are sent, or stop if NAK
:arb_read
                        rdlong    loop_counter,count_address
                        and       loop_counter,#$FF           wz
          if_z          jmp       #:arb_end
                        rdlong    t2,data_address
:arb_read_loop
                        call      #I2C_read                                     ' Read a byte
                        wrbyte    I2C_data,t2                                   '  and either store in @parameter[3] or in an array
                        add       t2,#1                                         '  increment the array address
                        sub       loop_counter,#1             wz
          if_nz         call      #I2C_ack                                      ' Send an ack if reading more bytes
          if_nz         jmp       #:arb_read_loop
                        call      #I2C_nak                                      ' Otherwise send a NAK
:arb_end
                        call      #I2C_stop                              
                        jmp       #main
'======================================================================================================================
send_start
                        mov       timeout,_10ms                                 ' Prepare a 10ms timeout (prevents routine from hanging if a device
                        add       timeout,cnt                                   '  becomes disconnected or unresponsive)
:loop
                        mov       t1,timeout                                    ' Check if 10ms has elapsed
                        sub       t1,cnt                                        '  Abort if it has
                        cmps      t1,#0                       wc
          if_c          jmp       #No_response
                        mov       I2C_data,device_byte                          ' Send the start bit and device ID
                        call      #I2C_start                                    '  Device will respond with Ack or NAK if ready/not ready
                        call      #I2C_write                                      
          if_c          jmp       #:loop                                        ' Loop until device is ready (C is set if NAK)
                        mov       t1,device_byte                                ' Determine if device code is for EEPROM (%101_0xxx)
                        and       t1,#%1111_0000                                ' Clear chip select bits
                        cmp       t1,#%1010_0000              wz                ' Z is set if device is an EEPROM
          if_z          rdword    I2C_data,register_address                     ' Send high byte of EEPROM address 
          if_z          shr       I2C_data,#8
          if_z          call      #I2C_write
                        rdword    I2C_data,register_address
                        call      #I2C_write
          if_c          jmp       #:loop                                        ' C is set if NAK (Some devices acknowledge the device code even when not ready)
send_start_ret          ret
'======================================================================================================================
I2C_start                                                                       ' SCL    
                        mov       delay_target,bit_delay                        ' SDA    
                        add       delay_target,cnt                              '     0 1 2 3    
                        andn      dira,SDA_mask                                 
                        andn      dira,SCL_mask                                                                   
                        waitcnt   delay_target,bit_delay                         
                        waitcnt   delay_target,bit_delay
                        or        dira,SDA_mask
                        waitcnt   delay_target,bit_delay
                        or        dira,SCL_mask                                                                   
                        waitcnt   delay_target,bit_delay
I2C_start_ret           ret
'----------------------------------------------------------------------------------------------------------------------
I2C_write                                                                        '   (Write)      (Read ACK or NAK)      
                        mov       bit_counter,#8                                 '                                     
                        rol       I2C_data,#24                                   ' SCL                   
                                                                                 ' SDA  ───────                 
                        mov       delay_target,bit_delay                         '     0 1 2 3  0 1 2 3                  
                        add       delay_target,cnt                               
:Loop                                                                               
                        rol       I2C_data,#1                 wc               
                        muxnc     dira,SDA_mask
                        waitcnt   delay_target,bit_delay                        
                        andn      dira,SCL_mask
{'check_stretch                                                                  ' some devices stretch the clock before the first bit
                        waitcnt   delay_target,bit_delay                        ' allow 1/4 bit width for clock to go high
                        test      SCL_mask,ina                wc
          if_c          jmp       #:lower1
                        waitpeq   SCL_mask,SCL_mask                             ' wait for clock to go high
                        mov       delay_target,bit_delay                        ' re-establish bitrate
                        add       delay_target,cnt}
:lower1
                        waitcnt   delay_target,bit_delay
                        or        dira,SCL_mask                                                                   
                        waitcnt   delay_target,bit_delay
                        djnz      bit_counter,#:Loop
:Read_ack_or_nak
                        andn      dira,SDA_mask
                        waitcnt   delay_target,bit_delay
                        andn      dira,SCL_mask
{'check_stretch                                                                  ' some devices stretch the clock after the last bit
                        waitcnt   delay_target,bit_delay                        ' allow 1/4 bit width for clock to go high
                        test      SCL_mask,ina                wc
          if_c          jmp       #:lower2
                        waitpeq   SCL_mask,SCL_mask                             ' wait for clock to go high
                        mov       delay_target,bit_delay                        ' re-establish bitrate
                        add       delay_target,cnt}
:lower2
                        test      SDA_mask,ina                wc                ' C is set if NAK
                        waitcnt   delay_target,bit_delay
                        or        dira,SCL_mask                                                                   
                        waitcnt   delay_target,bit_delay                                                              
I2C_write_ret           ret
'----------------------------------------------------------------------------------------------------------------------
I2C_read                                                                        '      (Read)
                        mov       bit_counter,#8                                '                
                        mov       delay_target,bit_delay                        ' SCL     
                        add       delay_target,cnt                              ' SDA ───────    
:loop                                                                           '     0 1 2 3    
                        andn      dira,SDA_mask                                 
                        waitcnt   delay_target,bit_delay                        
                        andn      dira,SCL_mask
{'check_stretch                                                                  ' not inconceivable that a device might stretch the clock during a read    
                        waitcnt   delay_target,bit_delay                        ' allow 1/4 bit width for clock to go high
                        test      SCL_mask,ina                wc
          if_c          jmp       #:lower
                        waitpeq   SCL_mask,SCL_mask                             ' wait for clock to go high
                        mov       delay_target,bit_delay                        ' re-establish bitrate
                        add       delay_target,cnt}
:lower
                        test      SDA_mask,ina                wc                ' Read
                        rcl       I2C_data,#1                                   ' Store in lsb
                        waitcnt   delay_target,bit_delay
                        or        dira,SCL_mask                                                                   
                        waitcnt   delay_target,bit_delay
                        djnz      bit_counter,#:Loop                            ' Repeat until eight bits received
I2C_read_ret            ret                        
'----------------------------------------------------------------------------------------------------------------------
I2C_ack                                                                          
                        mov       delay_target,bit_delay                        ' SCL  
                        add       delay_target,cnt                              ' SDA  
                        or        dira,SDA_mask                                 '     0 1 2 3  
                        waitcnt   delay_target,bit_delay                         
                        andn      dira,SCL_mask                                 
                        waitcnt   delay_target,bit_delay
                        waitcnt   delay_target,bit_delay
                        or        dira,SCL_mask                                 
                        waitcnt   delay_target,bit_delay
I2C_ack_ret             ret
'----------------------------------------------------------------------------------------------------------------------
I2C_nak                                                                         '                           
                        mov       delay_target,bit_delay                        ' SCL      
                        add       delay_target,cnt                              ' SDA      
                        andn      dira,SDA_mask                                 '     0 1 2 3      
                        waitcnt   delay_target,bit_delay                        
                        andn      dira,SCL_mask                                 
                        waitcnt   delay_target,bit_delay                        
                        waitcnt   delay_target,bit_delay                        
                        or        dira,SCL_mask                                 
                        waitcnt   delay_target,bit_delay                        
I2C_nak_ret             ret
'----------------------------------------------------------------------------------------------------------------------
I2C_stop
                        mov       delay_target,bit_delay                        ' SCL  
                        add       delay_target,cnt                              ' SDA  
                        or        dira,SDA_mask                                 '     0 1 2 3  
                        waitcnt   delay_target,bit_delay                        
                        andn      dira,SCL_mask                                                                   
                        waitcnt   delay_target,bit_delay                        
                        andn      dira,SDA_mask
                        waitcnt   delay_target,bit_delay                        
                        waitcnt   delay_target,bit_delay
                        mov       t1,#$FF
                        wrbyte    t1,command_address
I2C_Stop_ret            ret
'----------------------------------------------------------------------------------------------------------------------
no_response
                        mov       t1,#0
                        wrbyte    t1,command_address
                        jmp       #main
'======================================================================================================================
_10ms                   long      800_000
d1                      long      |< 9
command_address         res       1
device_address          res       1
register_address        res       1
data_address            res       1
count_address           res       1
order_address           res       1
size_address            res       1

command_byte            res       1
device_byte             res       1
count                   res       1
order                   res       1
size                    res       1
loop_counter            res       1

SCL_mask                res       1                                             
SDA_mask                res       1                                             
bit_delay               res       1
delay_target            res       1   
bit_counter             res       1
I2C_data                res       1
t1                      res       1
t2                      res       1
timeout                 res       1

                        fit

DAT  
parameters              long        0[7]
cog                     byte        0

                       
{{
┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
│                                                   TERMS OF USE: MIT License                                                  │                                                            
├──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┤
│Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation    │ 
│files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,    │
│modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software│
│is furnished to do so, subject to the following conditions:                                                                   │
│                                                                                                                              │
│The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.│
│                                                                                                                              │
│THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE          │
│WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR         │
│COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,   │
│ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                         │
└──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
}}                      
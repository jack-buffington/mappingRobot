'' This program does ADC
''
'' It appears that I really need to use the capacitor values given in app note 008
'' and to keep traces short.   Try this one again in a circuit board.  It isn't working
'' in a breadboard very well with the wrong value capacitors.
''

  _CLKMODE = xtal1 + pll16x
  _XINFREQ = 5_000_000



CON
    INP_PIN = 3 'Counter input pin for sigma-delta.
    FB_PIN = 4 'Counter feedback pin for sigma-delta.
    ADC_INTERVAL = 32768 'Time interval over which to accumulate counts.
        
PUB Start_ADC (address)                                    
    cognew(@adc_cog, address) 


DAT
	    org     0
adc_cog     mov     frqa,#1             'Initialize frqa to count up by one.
            movi    ctra,#%0_01001_000  'Set ctra mode to positive w/feedback.
            movd    ctra,#FB_PIN        'Write fback pin number to dst field.
            movs    ctra,#INP_PIN       'Write input pin number to src field.
            mov     dira,fb_mask        'Make the feedback pin an output.
            mov     result_addr,par     'Save @value into result_addr.
                                       '
main_loop   call    #adc                'Get a new acquisition.
            wrlong  acc,result_addr     'Write result to hub.
            jmp     #main_loop          'Back for another.

adc         mov     time,cnt            'Get the current counter value.
            add     time,#16            'Add a little to get ahead.
            waitcnt time,interval       'Sync to clock; add interval to time.
            neg     acc,phsa            'Negate phsa into result.
            waitcnt time,#0             'Wait for interval to pass.
            add     acc,phsa            'Add phsa into result.
adc_ret     ret



fb_mask     long    1 << FB_PIN         'Mask for feedback pin.
result_addr long    0-0                 'Result address gets plugged in here.
interval    long    ADC_INTERVAL        'Acquisition time.
acc         res     1                   'General-purpose accumulator.
time        res     1                   'Time variable use for waitcnt. 





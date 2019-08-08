/*   Firmware for Arduino board V3

     Written by Jack Buffington 2019 with the exception of the Morse Code beep functions, which are lightly modified from
     what can be found on the Arduino website.

     This board controls the motors and power for the mapping robot.  It is tasked with the following:
 *   * Drive according to commands given by the main processor
 *   * Control the power to the main processor, motors, and itself.
 *   * Monitor the battery voltage
 *   * Track position via encoders
 *   * Output beep or tone codes via a speaker
 *   * Determines if a wheel is stuck
 *   * Monitors bump sensors

     Control of this board will be done via serial data <TODO:  Verify that this is true.  It could be through USB serial.>

     This program will address serial commands through the main loop and will do the real-time tasks such as PID and reading
     the encoders from within interrupts.

     This program maintains a status byte which can be sent to the host processor.  The status byte is as follows:
     
    Status byte: <76543210>
    bit 7: in motion
    bit 6: left wheel stuck
    bit 5: right wheel stuck
    bit 4: bump sensor pressed
    bit 3: extra 4 is high
    bit 2: extra 3 is high
    bit 1: extra 2 is high
    bit 0: last motion completed successfully


    Currently using 5878 bytes of program space.  
     
     

*/

#define VOLTAGE_DETECT        A0
#define SERVO_PWM             A2
#define PROCESSOR_ENABLE      A3
#define MOTORS_ENABLE         A4
#define BUTTON                A5
#define FIVE_VOLT_ENABLE      A6
#define BEEPER                A7
#define RIGHT_ENCODER_A       2
#define RIGHT_ENCODER_B       4
#define LEFT_ENCODER_A        3
#define LEFT_ENCODER_B        5

#define MOTOR_CONTROLLER_6    6
#define MOTOR_CONTROLLER_5    7
#define MOTOR_CONTROLLER_4    8
#define MOTOR_CONTROLLER_3    9

#define LEFT_MOTOR_PWM        6   // TODO: THESE PINS ARE JUST A GUESS!  Figure out what they really are and fix this.
#define LEFT_MOTOR_DIRECTION  7
#define RIGHT_MOTOR_PWM       8
#define RIGHT_MOTOR_DIRECTION 9


#define EXTRA_IO_4            10
#define EXTRA_IO_3            11
#define EXTRA_IO_2            12
#define EXTRA_IO_1            A1
#define LED                   13

// 14.97V is where my drill considers the battery to be dead
// It reads 23.85V when full.
#define BATTERY_FULL          23500
#define BATTERY_DEAD          15000

#define SERIAL_BUFFER_SIZE    8
// queue depth is how many messages can be waiting for the main program at any given time.
#define QUEUE_DEPTH           3

#define TICKS_PER_METER       10000   // TODO:   Figure out the correct value for this (1920 pulses per wheel revolution)

// How many times per second the PID interrupt happens per second.
#define PID_INTERRUPT_RATE    10
#define P_TERM                32   
#define I_TERM                0
#define D_TERM                0
#define PID_SHIFT_POSITIONS   6     // effectively divide the PID terms by 64


#define DOT_DELAY            200 // for morse code

#define MAX_USER_MESSAGE_LENGTH   100



char* letters[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", // A-I
  ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", // J-R
  "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.." // S-Z
};

char* numbers[] = {
  "-----", ".----", "..---", "...--", "....-", ".....",
  "-....", "--...", "---..", "----."
};



byte serialBuffer[SERIAL_BUFFER_SIZE];
byte messageQueue[QUEUE_DEPTH][SERIAL_BUFFER_SIZE - 1];
byte messagesInQueue = 0;
byte userMessage[MAX_USER_MESSAGE_LENGTH];

// for motor control
int leftMotorTicksPerInterrupt = 0;
int rightMotorTicksPerInterrupt = 0;
unsigned int remainingInterruptsToDriveMotors = 0;

// For keeping track of position
long leftEncoder; 
long rightEncoder;
long lastLeftEncoder; // since it was last checked via serial command
long lastRightEncoder;

byte statusByte;  // for sending status messages

bool thereIsAnUserMessage;
byte userMessageSize;
byte currentUserMessageCharacter;



void setup()
{
  // Enable power to the Arduino
  digitalWrite(FIVE_VOLT_ENABLE, HIGH);

  // Wait a moment for power to stabilize then check the main battery voltage
  delay(100);
  int batteryPercent = readBattery();
  if (batteryPercent > 10)
  {
    // Set the pins to be inputs or outputs
    pinMode(VOLTAGE_DETECT, INPUT);
    pinMode(SERVO_PWM, OUTPUT);
    pinMode(PROCESSOR_ENABLE, OUTPUT);
    pinMode(MOTORS_ENABLE, OUTPUT);
    pinMode(BUTTON, INPUT);
    pinMode(FIVE_VOLT_ENABLE, OUTPUT);
    pinMode(BEEPER, OUTPUT);
    pinMode(RIGHT_ENCODER_A, INPUT);
    pinMode(RIGHT_ENCODER_B, INPUT);
    pinMode(LEFT_ENCODER_A, INPUT);
    pinMode(LEFT_ENCODER_B, INPUT);
    pinMode(MOTOR_CONTROLLER_6, OUTPUT);
    pinMode(MOTOR_CONTROLLER_5, OUTPUT);
    pinMode(MOTOR_CONTROLLER_4, OUTPUT);
    pinMode(MOTOR_CONTROLLER_3, OUTPUT);
    pinMode(EXTRA_IO_4, OUTPUT);
    pinMode(EXTRA_IO_3, OUTPUT);
    pinMode(EXTRA_IO_2, OUTPUT);
    pinMode(EXTRA_IO_1, OUTPUT);

    statusByte = 1;

    // Give the user an idea of the battery voltage in the range of 1 to 10
    batteryPercent /= 10;
    char percentChar[] = {batteryPercent + 48, 0};  // 48 shifts it into the correct ascii character range
    sendMorseString(percentChar);

    // Enable the main processor
    digitalWrite(PROCESSOR_ENABLE, HIGH);

    Serial1.begin(115200);


    // Set up the encoder interrupts
    // By default the external interrupts have higher priority than timer interrupts.  That is what I want.
    attachInterrupt(digitalPinToInterrupt(LEFT_ENCODER_A), leftEncoderInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(RIGHT_ENCODER_A), rightEncoderInterrupt, CHANGE);
   

    // Set up the PID interrupt
    leftEncoder = 0;
    rightEncoder = 0;
    lastLeftEncoder = 0; 
    lastRightEncoder = 0;


    thereIsAnUserMessage = false;

  }
  else
  { // The battery is too low to operate the robot so shut down.
    // disable power to the Arduino
    digitalWrite(FIVE_VOLT_ENABLE, LOW);
    while (1)
    { // hang out here while the power dies.

    }
  }


}

void loop()
{
  /* Watch for commands via the serial port
     00 - drive motors
     01 - request positional change since last request
     03 - get status
     05 - signal user

     Alert the main processor of issues
     04 - status message
  */
  if(thereIsAnUserMessage)
  { // Send one character per pass so that the main loop doesn't hang for too long.  
    
    
    char ch = userMessage[currentUserMessageCharacter];

    if(++currentUserMessageCharacter == userMessageSize)
      thereIsAnUserMessage = false;
    
    if (ch >= 'a' && ch <= 'z')
    {
      sendMorseCharacter(letters[ch - 'a']);
    }

    else if (ch >= 'A' && ch <= 'Z')
    {
      sendMorseCharacter(letters[ch - 'A']);
    }

    else if (ch >= '0' && ch <= '9')
    {
      sendMorseCharacter(numbers[ch - '0']);
    }

    else if (ch == ' ') // SPACE
    {
      delay(DOT_DELAY * 4);
    }
    
  }
  
  if (messagesInQueue)
  {
    if (messageQueue[0][0] == 0) // drive motors
      driveMotors();
    else if (messageQueue[0][0] == 1) // request positional change since last request
      requestPositionalChange();
    else if (messageQueue[0][0] == 3) // send status
      sendStatus();
    else if (messageQueue[0][0] == 5) // Signal user
      signalUser();  



    // Now that it is done processing this message, shuffle evertyhing down one message
    for (byte I = 1; I < messagesInQueue; I++)
    {
      byte* inPtr = messageQueue[I][0];
      byte* outPtr = messageQueue[I - 1][0];
      for (byte J = 0; J < SERIAL_BUFFER_SIZE; J++)
      { // Yes, I know that these five lines could be just one line but it is more clear to me this way.
        *outPtr = *inPtr;
        outPtr++;
        inPtr++;
      }
    }
    messagesInQueue--;
  }
}



void serialEvent1()
{ // This is run between each call to loop and only really runs if there is data that has arrived on the serial port

  // Reads bytes until a command is complete then checks its checksum.  If it checks out, then it passes the command to the main program.
  static byte* bufferPtr = serialBuffer;
  static bool startByteReceived = false;
  static byte bytesInMessage = 0;
  static byte bytesReceived = 0;
  static bool thisIsAnUserMessage = false;

  while (Serial1.available())
  {
    byte temp = Serial1.read();
    
    switch(bytesReceived)
    {
      case 0: // should be 0xA5
        if (temp == 0xA5)
          bytesReceived++;
        break;
        
      case 1: // // Should be bytes in message
        bytesInMessage = temp;
        bytesReceived++;
        break;
        
      case 2: // Should be message type
        if(temp == 0x05) // Signal user
        {
          bufferPtr = userMessage;
          thisIsAnUserMessage = true;
        }
          
        else
        {
          bufferPtr = serialBuffer;
          thisIsAnUserMessage = false;
        }
          
        *bufferPtr = temp;
        bufferPtr++;
        bytesReceived++;
        break;
        
      default: // user data and checksum
        *bufferPtr = temp;
        bytesReceived++;
        
        if(bytesReceived == bytesInMessage)
        {
          if (checkChecksum(bytesInMessage - 2,thisIsAnUserMessage)) // -2 because bytesInMessage includes the start byte and bytesInMessage byte, which aren't needed for the checksum.
          { // pass this data to the main program.
            if(thisIsAnUserMessage)
            {
              thereIsAnUserMessage = true;
              userMessageSize = bytesReceived; 
            }
            else // this was something other than an user message - copy it into the main program's buffer
            {
              if(messagesInQueue <= QUEUE_DEPTH)
              {
                byte* outPtr = messageQueue[messagesInQueue][0];
                byte* inPtr = serialBuffer;
                
                for(int I = 0; I < bytesInMessage; I++)
                {
                  *outPtr = *inPtr; 
                }
                messagesInQueue++;
              }
            else
            { // This message would overflow the message buffer.  Ignore it and alert the host processor that it wasn't received.
              sendNACK(); 
            }
          }
        }
        else // the checksum wasn't good.
        {
          sendNACK(); 
        }
        bytesReceived = 0;
      } // end of if this is the last byte in a message 
    }
  }
}



void sendNACK()
{ // Alerts the host computer that a message wasn't received properly.  
  byte checksum = 6;
  checksum ^= 0xFF;  // invert the bits
  checksum += 1;     // Now this is the two's complement of the sum of the other bytes.   
                    // When added with them, this will result in a total of zero if using byte-wise math
  
  // 0xA5 
  // 0x06  -- six bytes in the message including the checksum
  // 0x02 
  // <left wheel in millimeters (high byte)> 
  // <left wheel in millimeters (low byte)>
  // <right wheel in millimeters (high byte)>
  // <right wheel in millimeters (low byte)> 
  // <checksum>
  
  Serial1.write(0xA5); // start byte
  Serial1.write(4);    // bytes in message
  Serial1.write(6);    // NACK message
  Serial1.write(checksum);
}



bool checkChecksum(byte bytesInMessage, bool isUserMessage)
{ // Checks the serial buffer to see if it passes
  // The packet is correct if all bytes are added up and sum to zero.
  byte runningTotal = 0;
  byte* bufferPtr;
  
  if(isUserMessage)
    bufferPtr = serialBuffer;
  else
    bufferPtr = serialBuffer;

    
  for (byte I = 0; I < bytesInMessage; I++)
  {
    runningTotal += *bufferPtr;
  }

  if (runningTotal)
    return false;
  else
    return true;
}




byte readBattery()
{ // outputs a value in the range of 0 to 100 which represents the roughly remaining capacity of the battery in percentage.ch tex
  

  /*
   *  Due to the voltage divider that I am using:  200K and 47K, this multiplies the voltage by .1903
   *  With 1024 counts for 5V, to get the voltage read, I need to multiply the counts by 5 and divide by 1024.
   *  To get the actual voltage of the battery, I would want to multiply that by 1/.1903.  To get this voltage, 
   *  it is roughly equivalent to multiplying by (1681/65536).  Of course, since we are dealing with only whole 
   *  numbers, this doesn't leave much resolution so this program is working in voltage * 1000.  This makes the 
   *  calculation be <analog reading> * (1681702/65536).  An unsigned long can handle the values in the range 
   *   0-4,294,967,295.  1681702 * 1024 (the max reading) is 1,722,062,848.  This will fit within an unsigned long!
   */
  unsigned long batteryVoltage;
  batteryVoltage = analogRead(VOLTAGE_DETECT); // gives me a value in the range of 0 to 1024.
  batteryVoltage *= 1681702;
  batteryVoltage >>= 16; // Make sure that the calculation is fast rather than doing a real division.


  // Calculate the rough remaining battery capacity based on a linear relationship to the voltage
  batteryVoltage -= BATTERY_DEAD;

  word batteryRange = BATTERY_FULL - BATTERY_DEAD;
  // Since I am working in integer math, I can't just divide batteryVoltage by the battery range since I
  // would get zero every time so I'm going to multiply the battery voltage by 100 first to give me a result in the range of
  // 0 to ~100.
  batteryVoltage *= 100;

  batteryVoltage /= batteryRange;

  if (batteryVoltage > 100) // It could be higher because I set the full voltage a little lower than what it really
    batteryVoltage = 100;   // is when it is full.

  return batteryVoltage;
}








void sendMorseString(char* theString)
{ // Sends letters, numbers and spaces only.  Punctuation or other characters will be ignored.
  // This is used at startup only.  During normal operation, characters are sent one at a time between successive loop()'s.
  char ch;
  int I = 0;
  while (theString[I] != NULL)
  {
    ch = theString[I];
    I++;

    if (ch >= 'a' && ch <= 'z')
    {
      sendMorseCharacter(letters[ch - 'a']);
    }

    else if (ch >= 'A' && ch <= 'Z')
    {
      sendMorseCharacter(letters[ch - 'A']);
    }

    else if (ch >= '0' && ch <= '9')
    {
      sendMorseCharacter(numbers[ch - '0']);
    }

    else if (ch == ' ') // SPACE
    {
      delay(DOT_DELAY * 4);
    }
  }
}

void sendMorseCharacter(char* sequence)
{
  int I = 0;
  while (sequence[I] != NULL)
  {
    outputDotOrDash(sequence[I]);
    I++;
  }
  delay(DOT_DELAY * 3);
}


void outputDotOrDash(char dotOrDash)
{
  digitalWrite(BEEPER, HIGH);
  if (dotOrDash == '.')
  {
    delay(DOT_DELAY);
  }
  else // must be a -
  {
    delay(DOT_DELAY * 3);
  }
  digitalWrite(BEEPER, LOW); 
  delay(DOT_DELAY);
}



void driveMotors()
{ // reads data from the first message in the messageQueue and uses it to drive the motors.
  // The message gives speeds in mm/s and a time in 10ths of a second.  This function calculates how many encoder ticks must happen 
  // per PID interrupt in order to maintain this speed.  

  // Figure out how long to run for.
  unsigned int runTime = twoBytesToUnsignedInt(messageQueue[0][5], messageQueue[0][6]); // this is in 10ths of a second
  /*
     Convert the run time into the number of PID interrupts to run for.  Although this will lead to some inaccuracy in
     how long it will run, it should be close enough.

     runTime    1 s    PID_INTERRUPT_RATE   interrupts
     ------- * ----- * ------------------ = ----------
         1      10            1 s               1
  */
  runTime *= PID_INTERRUPT_RATE;
  runTime /= 10;




  int leftMotor = twoBytesToSignedInt(messageQueue[0][1], messageQueue[0][2]);
  int rightMotor = twoBytesToSignedInt(messageQueue[0][3], messageQueue[0][4]);

  /*  convert mm/s to ticks/interrupt
          mm      1 m     ticks       1 s        ticks
        ----- * ------- * ----- * ---------- = ---------
         sec    1000 mm    1 m    interrupts   interrupt
  */

  long temp = TICKS_PER_METER;          // TODO: Change this to a lesser data type if it will fit
  temp /= (PID_INTERRUPT_RATE * 1000);

  leftMotor *= temp; // Now these values are in ticks per interrupt
  rightMotor *= temp;


  // Set all values at once so I only have to disable interrupts once.
  // For anyone reading this who is asking why I would disable interrupts, it is because an interrupt could happen
  // in the middle of writing out these values if I didn't disable them.  This could lead to unexpected behaviour if
  // ony one of two bytes for a variable had been written.
  noInterrupts();
  leftMotorTicksPerInterrupt = leftMotor;
  rightMotorTicksPerInterrupt = rightMotor;
  remainingInterruptsToDriveMotors = runTime;
  interrupts();
}



int twoBytesToSignedInt(byte byte1, byte byte2)
{ // converts two bytes into a signed in using the following calculation:
  // ((byte1 * 256) + byte2) - 32767;

  long temp = byte1;
  temp <<= 8;
  temp += byte2;
  temp -= 32767;
  return temp;
}




unsigned int twoBytesToUnsignedInt(byte byte1, byte byte2)
{ // converts two bytes into a signed in using the following calculation:
  // (byte1 * 256) + byte2

  unsigned int temp = byte1;
  temp <<= 8;
  temp += byte2;

  return temp;
}



void requestPositionalChange()
{
  // Find the change in encoder counts
  long leftDif = leftEncoder - lastLeftEncoder;
  long rightDif = rightEncoder - lastRightEncoder;

  // Convert these into millimeters
  leftDif *= 1000;
  leftDif /= TICKS_PER_METER;
  rightDif *= 1000;
  rightDif /= TICKS_PER_METER;

  // It would be nice to compute a true dead reckoning solution here but because the Arduino is so slow, 
  // I'll leave that up to the computer to do.  

  // Send this information off to the computer. 
  // To make this easy, I'm going to add an offset to hopefully keep things out of negative territory and then will 
  // just break the values into bytes and send them.  THe receiving program will have to reconstruct the values back into
  // signed values.  

   leftDif += 32767; 
   rightDif += 32767;

   byte leftLow = leftDif;
   byte leftHigh = leftDif >> 8;

   byte rightLow = rightDif;
   byte rightHigh = rightDif >> 8;

  // Compute the checksum
   byte checksum = 0x02;
   checksum += leftLow + leftHigh + rightLow + rightHigh;

   checksum ^= 0xFF;  // invert the bits
   checksum += 1;     // Now this is the two's complement of the sum of the other bytes.   
                      // When added with them, this will result in a total of zero if using byte-wise math

   // 0xA5 
   // 0x06  -- six bytes in the message including the checksum
   // 0x02 
   // <left wheel in millimeters (high byte)> 
   // <left wheel in millimeters (low byte)>
   // <right wheel in millimeters (high byte)>
   // <right wheel in millimeters (low byte)> 
   // <checksum>

   Serial1.write(0xA5);
   Serial1.write(6);
   Serial1.write(2);
   Serial1.write(leftHigh);
   Serial1.write(leftLow);
   Serial1.write(rightHigh);
   Serial1.write(rightLow);
   Serial1.write(checksum);
}


void sendStatus()
{
  /*
   * 0xA5 <bytes in message> 0x04 <status byte> <battery level> <checksum>
   
    Status byte: <76543210>
    bit 7: in motion
    bit 6: left wheel stuck
    bit 5: right wheel stuck
    bit 4: extra 4 is high
    bit 3: extra 3 is high
    bit 2: extra 2 is high
    bit 1: extra 1 is high
    bit 0: last motion completed successfully

    Battery level is in percentage from 0 to 100.
   */

  byte message[8];

  message[0] = 0xA5;
  
  // build up the rest of the status byte - PID loop is responsible for maintaining bits 7,6,5, and 0.
  if(digitalRead(EXTRA_IO_4) == HIGH)
    statusByte |= B00010000;
  else
    statusByte &= B11101111;

  if(digitalRead(EXTRA_IO_3) == HIGH)
    statusByte |= B00001000;
  else
    statusByte &= B11110111;

  if(digitalRead(EXTRA_IO_2) == HIGH)
    statusByte |= B00000100;
  else
    statusByte &= B11111011;

  if(digitalRead(EXTRA_IO_1) == HIGH)
    statusByte |= B00000010;
  else
    statusByte &= B11111101;

  byte battery = readBattery();

  byte checksum = 4 + statusByte + battery;
  checksum ^= 0xFF;  
  checksum += 1;

  Serial1.write(0xA5);
  Serial1.write(4);
  Serial1.write(4);
  Serial1.write(statusByte);
  Serial1.write(battery);
  Serial1.write(checksum);
}


void leftEncoderInterrupt()
{ /*  This interrupt happens when the A input of the left encoder changes. 
      I'm not sure how to look at the assembly code generated by an Arduino sketch so I'm basing my timing information 
      on information from http://www.gammon.com.au/interrupts
      It is saying that using attachInterrupt() It takes 5.125 uS to just jump into and out of an interrupt.  That gives an interrupt
      rate of 195,121 interrupts per second if nothing is done. Divide that by two encoders and I get 97560 interrupts per second per encoder.
      Given that my encoders are 16 pulses per revolution and the gearbox is 120:1 I should have 1920 pulses per wheel revolution.  The motor has a 
      no-load RPM of 160 at 6V.  This works out to 2.667 revolutions per second.  This gives a maximum pulse rate of 5120 pulses per second.  This is 
      well within the maximum so I won't bother to optimize to make this routine faster.
  */  
  bool A = digitalRead(LEFT_ENCODER_A);
  bool B = digitalRead(LEFT_ENCODER_B);

  if(A)
  {
    if(B)
      leftEncoder++;
    else
      leftEncoder--;   
  }
  else
  {
    if(B)
      leftEncoder--;
    else
      leftEncoder++;
  }
}

void rightEncoderInterrupt()
{ /*  This interrupt happens when the A input of the left encoder changes. 
  */  
  bool A = digitalRead(RIGHT_ENCODER_A);
  bool B = digitalRead(RIGHT_ENCODER_B);

  if(A)
  {
    if(B)
      rightEncoder++;
    else
      rightEncoder--;   
  }
  else
  {
    if(B)
      rightEncoder--;
    else
      rightEncoder++;
  }
}


void PIDinterrupt()
{/* There is a line of thinking that interrupt should be as short as possible and only set flags for the main program to address.
    I'm not going to do that in this case because I worry about there being too much jitter in the timing of the PID loop, which 
    could lead to instability in the motors' speeds. That worry may be unfounded though.  If this code is still here when you read
    this then everything has worked out. It looks like the motor encoder interrupts happen rarely enough that the relative length of 
    this routine won't cause it to miss any encoder ticks.

    This function implements velocity PID as really just positional PID where the desired position is just incremented at each PID
    loop.  For now, the velocity is fixed throughout the move.  I may change this to ramp up and down the velocity if I find that 
    there is excessive wheel slip at the starts and stops.  

    NOTE:  I used to work with the PIC processor, which doesn't have a multiply instruction so I was prepared to write a specialized 
    multiply function to multiply a long and something shorter together but after looking around, apparently it only takes 12 cycles
    to multiply two longs together on an AVR so there is no need to optimize.  This will be plenty fast as long as I am doing division 
    using bit shifts.
  
  */

  LEDon(); // This is here so that I can use my logic analyzer or oscilliscope to see how long this takes to run and what the actual 
           // interrupt rate is.  

//  static long lastISRleftEncoder = 0;
//  static long lastISRrightEncoder = 0;
  static long desiredLeftEncoder = 0;
  static long desiredRightEncoder = 0;
  
  static int leftDifHistory[] = {0,0,0,0};
  static int rightDifHistory[] = {0,0,0,0};


  int leftDif = desiredLeftEncoder - leftEncoder;       // diference between desired and actual position
  int rightDif = desiredRightEncoder - rightEncoder;    //  This could be up to about 512 counts on first interrupt after starting and 
                                                        //  would probably go higher for a bit.  Let's say 1500 max.
  
  desiredRightEncoder += rightMotorTicksPerInterrupt;   // Increment the desired position
  desiredLeftEncoder += leftMotorTicksPerInterrupt;


  

  
  // Proportional
  long leftP = leftDif;
  leftP *= P_TERM;  // Assuming a 1500 count difference, with P_TERM being 100, this would be a max of +-150,000
  leftP >>= PID_SHIFT_POSITIONS; // divide by 16
  
  long rightP = rightDif;
  rightP *= P_TERM;  
  rightP >>= PID_SHIFT_POSITIONS; 



  // Integral
  // Sum up this and previous differences
  long leftI = leftDif + leftDifHistory[0] + leftDifHistory[1] + leftDifHistory[2] + leftDifHistory[3];
  leftI *= I_TERM;
  leftI >>= PID_SHIFT_POSITIONS;

  long rightI = rightDif + rightDifHistory[0] + rightDifHistory[1] + rightDifHistory[2] + rightDifHistory[3];
  rightI *= I_TERM;
  rightI >>= PID_SHIFT_POSITIONS;


  // Differential
  long leftD = leftDif - leftDifHistory[3];
  leftD *= D_TERM;
  leftD >>= PID_SHIFT_POSITIONS;

  long rightD = rightDif - rightDifHistory[3];
  rightD *= D_TERM;
  rightD >>= PID_SHIFT_POSITIONS;

  // Shuffle things around now in the histories to prep for the next PID interrupt
  leftDifHistory[0] = leftDifHistory[1];
  leftDifHistory[1] = leftDifHistory[2];
  leftDifHistory[2] = leftDifHistory[3];
  leftDifHistory[3] = leftDif;

  rightDifHistory[0] = rightDifHistory[1];
  rightDifHistory[1] = rightDifHistory[2];
  rightDifHistory[2] = rightDifHistory[3];
  rightDifHistory[3] = rightDif;

  // Calculate the final PWM values and motor directions
  long temp = leftP + leftI + leftD;
  bool leftDir = temp > 0;
  byte leftPWM = min(255,abs(temp));

  
  temp = rightP + rightI + rightD;
  bool rightDir = temp > 0;
  byte rightPWM = min(255, abs(temp));

  // Set the PWM values and directions.
  analogWrite(LEFT_MOTOR_PWM, leftPWM);
  analogWrite(RIGHT_MOTOR_PWM, rightPWM);
  digitalWrite(LEFT_MOTOR_DIRECTION, leftDir);
  digitalWrite(RIGHT_MOTOR_DIRECTION, rightDir);
    
  LEDoff();
}


inline void LEDon()
{
  digitalWrite(LED, HIGH);
}

inline void LEDoff()
{
  digitalWrite(LED, LOW);
}


void signalUser()
{ // Sets things up so that Morse code will be output with the desired user message.
  userMessageSize = messageQueue[0][1];
  thereIsAnUserMessage = true;
  currentUserMessageCharacter = 0;

}

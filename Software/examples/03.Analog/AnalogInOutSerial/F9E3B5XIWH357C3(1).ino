/*
   Created by:  Jay Weeks
   On date:     30-Aug-2016
   Last update: 8-Sep-2016

   Purpose:
    A tool for exploring the PWM response of a motor.
    PWM output is controlled by the two buttons and onboard POT.
    
    Interrupts connected motor encoder measure time between
    interrupts, and direction of rotation.
    
    Settings and encoder response are output to serial.

   Current:
    ISR
      Calculates time between ISR updates.
      Measures the direction at the time of interrupt
    Loop
      Runs state machine to controll PWM
      POT changes increment value based off discreet ranges:
        1, 10, 100
        Button 2 increments PWM up.
        Button 3 increments PWM down.
        At any of these events, update 
      Outputs measured period, direction, and current direction
      to serial.
      Also outputs statistics for encoder response
   
   Next:
    We're Done!
    
   Notes:
    This sketch is used in tandem to a Pmod HB5.
    pin7/RB14       ->  Direction
    pin6/RB13/OC5   ->  Enable
    AD OS1          <-  SA
    AD OS2          <-  SB
    All grounds tied together
    3v3             <-  V

    The micros() function will loop back to zero after about 70
    minutes.
    The millis() function will loop after about 50 days.
    Both of these functions return an unsigned long, which has a
    maximal value of 4294967295, or 0xFFFFFFFF.

    Our direction pin happens to allign with our assigned
    direction. In other words, when we tell our motor to spin in
    the 0 direction, our encoder direction pin will show a 0,
    and vice-versa.
    It's important to note that this will not always be the case,
    and if it is not, then notation must be change in code so
    the input and the output agree. Otherwise the PID will be
    confused and useless, and that makes PIDs sad. :(
*/

#define DIR 7
#define EN  6
#define SA  3
#define SB  9
#define POT 8
#define BTN2  PIN_BTN1
#define BTN3  PIN_BTN2

#define DELAY 20  // Our delay between speed updates in millis
#define INCREMENT 5   // Our increment value for speed control

#define HISTORY_SIZE 32   // The size of our history for
                          // averaging the period
#define TIMEOUT 1000  // MS to wait for averaging period to
                      // complete before timing out

/********************************************************
 * Utility functions
 ********************************************************/
unsigned long getDiff(unsigned long prev, unsigned long curr);

bool changeDir();
bool setDir(bool newDir);
bool setSpd(int newSpeed);


/********************************************************
 * State Machines
 ********************************************************/
void speedControl();
void PWMControl();

/********************************************************
 * Interrupt Service Routine
 ********************************************************/
void myISR();

/********************************************************
 * Variables
 ********************************************************/
// PWMControl variables
bool currentDir = 0;
int PWMincrement = 1;
unsigned long PotTime = 0;
int Btn2Prev = 0;
unsigned long Btn2Time = 0;
int Btn3Prev = 0;
unsigned long Btn3Time =0;
int currentPWM = 0;
unsigned int histCount = 0;
unsigned long histMin = 0xFFFFFFFF;
unsigned long histMax = 0;
unsigned long histAvg = 0;

// ISR variables
volatile bool measuredDir = 0;
volatile int prevInt = 0;
volatile int diffInt = 0;

void setup()
{
  pinMode(DIR, OUTPUT);
  pinMode(EN, OUTPUT);
  pinMode(SA, INPUT);
  pinMode(SB, INPUT);
  pinMode(POT, INPUT);
  pinMode(BTN2, INPUT);
  pinMode(BTN3, INPUT);

  // Set up our interrupt and ISR
  attachInterrupt(1, myISR, RISING);

  // Initialize our direction and speed to 0
  analogWrite(EN, currentPWM);
  digitalWrite(DIR, currentDir);
  delay(10);   // Wait for things to settle

  Serial.begin(9600);
}

void loop()
{
  PWMControl();
}

void PWMControl()
{
  bool updateSerial = false;
  int Btn2Curr = 0;
  int Btn3Curr = 0;
  unsigned long currentTime = millis();

  // Set our PWM increment
  // Has it been long enough since our last change?
  if ((currentTime - PotTime) >= 50)
  { // It has? Cool.
    // First get our Pot value
    int readPot = analogRead(POT);
    // Convert our POT reading to a discreet PWM increment
    if (readPot <= 341) readPot = 1;
    else if (readPot <= 682) readPot = 10;
    else readPot = 100;
    // See if that's different from what we've already got
    if (PWMincrement != readPot)
    { // If it is
      // Set our new PWM increment
      PWMincrement = readPot;
      // Let serial know it needs to update
      updateSerial = true;
    }
    // Store our current Pot time
    PotTime = currentTime;
  }

  // Increment up
  // Check for a button 2 change
  // Has it been long enough since last time?
  if ((currentTime - Btn2Time) >= 50)
  { // It has? Cool.
    // Get our current Btn2 state
    Btn2Curr = digitalRead(BTN2);
    // Has it changed since last time?
    if (Btn2Curr && !Btn2Prev)
    { // If it has, increment our PWM up
      currentPWM += PWMincrement;
      // Reset our history so we start getting new period stats
      histCount = -1;
      // Let serial know it needs to update
      updateSerial = true;
    }
    // Store our current Btn2 state
    Btn2Prev = Btn2Curr;
    // Store our current Btn2 time
    Btn2Time = currentTime;
  }

  // Increment down
  // Check for a button 3 change
  // Has it been long enough since last time?
  if ((currentTime - Btn3Time) >= 50)
  { // It has? Cool.
    // Get our current Btn3 state
    Btn3Curr = digitalRead(BTN3);
    // Has it changed since last time?
    if (Btn3Curr && !Btn3Prev)
    { // If it has, increment our PWM down
      currentPWM -= PWMincrement;
      // Reset our history so we start getting new period stats
      histCount = -1;
      // Let serial know it needs to update
      updateSerial = true;
    }
    // Store our current Btn3 state
    Btn3Prev = Btn3Curr;
    // Store our current Btn3 time
    Btn3Time = currentTime;
  }

  // Look for direction change
  // Has our PWM gone negative?
  if (currentPWM < 0)
  { // What? It HAS?
    // Invert the current PWM
    currentPWM = -currentPWM;
    // Set our new speed
    analogWrite(EN, currentPWM);
    // Reverse direction
    changeDir();
    // Wait a little while to let everything settle
    delay(100);
  }
  else if (currentPWM > 255)
  {
    // Otherwise, if the PWM has exceeded it's max, just truncate
    currentPWM = 255;
    // And set our current PWM
    analogWrite(EN, currentPWM);
    // Wait a little while to let everything settle
    delay(100);
  }
  else
  { // Otherwise just set our new PWM
    analogWrite(EN, currentPWM);
    // Wait a little while to let everything settle
    delay(100);
  }
  
  // Collect period stats
  // Check to see if our history has reset
  if (histCount == -1)
  { // Start the reset process
    // Reset our min and max values
    histMin = 0xFFFFFFFF;
    histMax = 0;
    histAvg = 0;
    // Also reset our interrupt period so we don't store old data
    diffInt = 0;
    // Finally, update histCount so we don't do this again
    histCount = 0;
  }
  // Now check to see if our history is full yet
  if (histCount < 127)
  { // If it's not full yet
    // Check for an update from our interrupt
    if (diffInt)
    {
      histCount ++;

      // Check for min
      if (diffInt < histMin) histMin = diffInt;
      // Check for max
      if (diffInt > histMax) histMax = diffInt;
      // Update the sum
      histAvg += diffInt;
      
      // Clear our update
      diffInt = 0;
    }
    // Now check to see if we've just filled our history
    if (histCount == 127)
    {
      // Let serial know we're ready to update
      updateSerial = true;
      // Push the index up so we don't keep updating Serial
      histCount = 128;
      // Get our average
      histAvg = histAvg / 128.0;
    }
  }
  
  // Check for any changes to promt a Serial update
  if (updateSerial)
  {
    // Output our header
    Serial.println("PWMinc\tCurrent\tDir\thistAvg\thistMin\thistMax");
    // Output our increment
    Serial.print(PWMincrement);
    Serial.print("\t");
    // Output our current PWM
    Serial.print(currentPWM);
    Serial.print("\t");
    // Output our direction
    Serial.print(measuredDir);
    Serial.print("\t");
    // Output our average period
    Serial.print(histAvg);
    Serial.print("\t");
    // Output our min period
    Serial.print(histMin);
    Serial.print("\t");
    // Output our max period
    Serial.print(histMax);
    Serial.print("\t");

    // End the line
    Serial.println("");
  }
}

unsigned long getDiff(unsigned long prev, unsigned long curr)
{
  long diff = curr - prev;
  if (diff < 0) diff = diff + 0xFFFFFFFF;
  return diff;
}

void myISR()
{
  // Get our time difference
  diffInt = getDiff(prevInt, micros());
  // Get our speed
  measuredDir = digitalRead(SB);
  // Store our current time
  prevInt = micros();
}

bool changeDir()
{
  // Stop our motor so we can change direction
  analogWrite(EN, 0);
  delay(10);  // Wait a bit for things to settle

  // Change our direction
  currentDir = !currentDir;
  digitalWrite(DIR, currentDir);
  delay(10);  // Wait a bit for things to settle

  // Start our motor back up again
  analogWrite(EN, currentPWM);
}

bool setDir(bool newDir)
{
  // Stop our motor so we can change direction
  analogWrite(EN, 0);
  delay(10);  // Wait a bit for things to settle

  // Change our direction
  currentDir = newDir;
  digitalWrite(DIR, currentDir);
  delay(10);  // Wait a bit for things to settle

  // Start our motor back up again
  analogWrite(EN, currentPWM);
}

bool setSpd(int newSpeed)
{
  // Ensure that new speed is within bounds
  if (newSpeed > 255) newSpeed = 255;
  else if (newSpeed < 0) newSpeed = 0;

  currentPWM = newSpeed;
  analogWrite(EN, currentPWM);
}



// I hate typing at the very bottom of the screen, don't you?

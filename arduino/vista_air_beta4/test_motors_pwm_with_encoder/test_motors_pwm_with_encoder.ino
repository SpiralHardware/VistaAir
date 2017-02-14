
// needed for hacked encoder library
#define ESP32 1

#include <Encoder.h>

#include <string.h>
#include <stddef.h>


#define M1PIN 14
#define M2PIN 27
#define M1CHANNEL 1
#define M2CHANNEL 2

#define CLOCKWISE 1
#define ANTICLOCKWISE 2

#define LEFT_MOTOR 0
#define RIGHT_MOTOR 1

#define SYNC_THRESHOLD 5
#define SYNC_DIFF_LIMIT 1000

#define PWM_MAX 255

int inApin[2] = {26, 13};  // INA: Clockwise input
int inBpin[2] = {25, 33}; // INB: Counter-clockwise input
int ledcs[2] = {M1CHANNEL, M2CHANNEL}; // PWM input

int boardLed = 5;

Encoder leftMotor(16, 17);
Encoder rightMotor(18, 23);

void setup(void) {
  Serial.begin(115200);
  delay(3000);

  Serial.println("turn off led");
  pinMode(boardLed, OUTPUT);
  digitalWrite(boardLed, LOW);// Turn off on-board blue led
  
  // assign pwm pins to ledc channels
  ledcAttachPin(M1PIN, M1CHANNEL);
  ledcAttachPin(M2PIN, M2CHANNEL);

  // intialise the ledc channels
  ledcSetup(M1CHANNEL, 500, 8); // 500Hz PWM, 8-bit resolution
  ledcSetup(M2CHANNEL, 500, 8);
  
  // Initialize digital pins as outputs
  for (int i=0; i<2; i++)
  {
    pinMode(inApin[i], OUTPUT);
    pinMode(inBpin[i], OUTPUT);
   // pinMode(pwmpin[i], OUTPUT);
  }
  // Initialize braked
  for (int i=0; i<2; i++)
  {
    digitalWrite(inApin[i], LOW);
    digitalWrite(inBpin[i], LOW);
    //digitalWrite(pwmpin[i], LOW);
    pinMode(ledcs[i], OUTPUT);
  }
  motorOff(0);
  motorOff(1);
}


void motorOff(int motor)
{
  // Initialize braked
  for (int i=0; i<2; i++)
  {
    digitalWrite(inApin[i], LOW);
    digitalWrite(inBpin[i], LOW);
  }
  ledcWrite(ledcs[motor], 0);
   //digitalWrite(pwmpin[motor], LOW);
}

/* motorGo() will set a motor going in a specific direction
 the motor will continue going in that direction, at that speed
 until told to do otherwise.
 
 motor: this should be either 0 or 1, will selet which of the two
 motors to be controlled
 
 direct: Should be between 0 and 3, with the following result
 0: Brake to VCC
 1: Clockwise
 2: CounterClockwise
 3: Brake to GND
 
 pwm: should be a value between ? and 1023, higher the number, the faster
 it'll go
 */
void motorGo(uint8_t motor, uint8_t direct, uint8_t pwm)
{
  if (motor <= 1)
  {
    if (direct <=4)
    {
      // Set inA[motor]
      if (direct <=1)
        digitalWrite(inApin[motor], HIGH);
      else
        digitalWrite(inApin[motor], LOW);

      // Set inB[motor]
      if ((direct==0)||(direct==2))
        digitalWrite(inBpin[motor], HIGH);
      else
        digitalWrite(inBpin[motor], LOW);

      ledcWrite(ledcs[motor], pwm);
    }
  }
}


long positionLeft  = -999;
long positionRight = -999;

/* run motors together for ms milliseconds, and keep them in synch 
 *  by reading the encoders. If one motor gets ahead by more than the SYNCH_THRESHOLD
 *  then then slow it down by the fraction of the SYNCH_DIFF_LIMIT that it is ahead by.
 *  That is - if the SYNCH_DIFF_LIMIT is 1000 and motor A is ahead of motor B by 100 encoder tics
 *  then slow it down by 10% 
 *  If motor A is ahead of motor B by encoder 1000 tics or more, then set it to stop.
 */
void goMotorsForMs(int motorDir, int motorSpeed, long ms){

  long startTime = millis();

  while(millis() < (startTime + ms)) {
    motorGo(0, motorDir, motorSpeed);
    motorGo(1, motorDir, motorSpeed);

    long newLeft, newRight;
    newLeft = leftMotor.read();
    newRight = rightMotor.read();
    if (newLeft != positionLeft || newRight != positionRight) {
      positionLeft = newLeft;
      positionRight = newRight;
    }
        
    // note the PJRC library uses clockwise as positive direction; that is, a higher position count
    // tells us that the motor has progressed further clockwise.
    // We need to be sure out motors are wired correctly to match !!
    long difference = abs(abs(positionLeft) - abs(positionRight));
    
    if (difference > SYNC_THRESHOLD){
        
      // now, slow the offending motor proportionally - to how far it is out of synch.
      // As a percertage of SYNC_DIFF_LIMIT, where if the encoder readings difference is equal or greater 
      // than the limit then the offending motor is told to run at zero duty cycle (pwm)
      if (difference >= SYNC_DIFF_LIMIT) {
        difference = SYNC_DIFF_LIMIT;
      }
      float slowFraction = (float(SYNC_DIFF_LIMIT) - float(difference)) / float(SYNC_DIFF_LIMIT);
      int slowedMotorSpeed = (int) (PWM_MAX * slowFraction);
      
      // clockwise - so the motor with the higher count has gone too far - slow it down
      if(motorDir == CLOCKWISE){
        if(positionLeft > positionRight){
           motorGo(LEFT_MOTOR, motorDir, slowedMotorSpeed);
        } else {
           motorGo(RIGHT_MOTOR, motorDir, slowedMotorSpeed);
        }
      
      // anticlockwise - so the motor with the lower count has gone too far - slow it down
      } else if (motorDir == ANTICLOCKWISE){
        if(positionLeft < positionRight){
           motorGo(LEFT_MOTOR, motorDir, slowedMotorSpeed);
        } else {
           motorGo(RIGHT_MOTOR, motorDir, slowedMotorSpeed);
        }
      }
    }

    // 15 milliseconds seems like a reasonable amount of time to run the motors before
    // starting a new cycle to check if they are in synch 
    delay(15); 
  }
}

int newDirection = ANTICLOCKWISE;
int currentDirection = CLOCKWISE;
int swapDirection = CLOCKWISE;
void loop(void) {

  Serial.println("testing motors");
  Serial.println("both high 15 secs ");
  
  long newLeft, newRight;
  newLeft = leftMotor.read();
  newRight = rightMotor.read();
  if (newLeft != positionLeft || newRight != positionRight) {
    Serial.print("Left = ");
    Serial.print(newLeft);
    Serial.print(", Right = ");
    Serial.print(newRight);
    Serial.println();
    positionLeft = newLeft;
    positionRight = newRight;
  }

  digitalWrite(boardLed, HIGH);

  // run both motors in synch for 15 seconds.
  goMotorsForMs(currentDirection, PWM_MAX, 15000);
  swapDirection = currentDirection;
  currentDirection = newDirection;
  newDirection = swapDirection;
  
  motorOff(0);
  motorOff(1);
  digitalWrite(boardLed, LOW);
  Serial.println("both off 3 secs ");
  delay(3000);
}





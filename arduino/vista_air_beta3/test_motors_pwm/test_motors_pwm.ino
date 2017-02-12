
// needed for hacked encoder library
#define ESP32 1
#include <string.h>
#include <stddef.h>


#define M1PIN 14
#define M2PIN 27
#define M1CHANNEL 1
#define M2CHANNEL 2
#define CW   1
#define CCW  2


int inApin[2] = {26, 13};  // INA: Clockwise input
int inBpin[2] = {25, 33}; // INB: Counter-clockwise input
//int pwmpin[2] = {14, 27}; // PWM input
//int inApin[2] = {7, 4};  // INA: Clockwise input
//int inBpin[2] = {8, 9}; // INB: Counter-clockwise input
//int pwmpin[2] = {5, 6}; // PWM input

int ledcs[2] = {M1CHANNEL, M2CHANNEL}; // PWM input
// testing without pwm
//int mPins[2] = {M1PIN, M2PIN};

int boardLed = 5;
//int boardLed = 12;
  
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
  
  //Serial.println("try motor go");
  //Serial.println("try motor go");
 // motorGo(1, CW, 255);
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
      //digitalWrite(pwmpin[motor], HIGH);
    }
  }
}
  
void loop(void) {

  Serial.println("testing motors");
  Serial.println("both high 15 secs ");
  motorGo(0, 1, 255);
  motorGo(1, 1, 255);
  digitalWrite(boardLed, HIGH);
  delay(15000);

  motorOff(0);
  motorOff(1);
  digitalWrite(boardLed, LOW);
  Serial.println("both off 10 secs ");
  delay(10000);
  
}





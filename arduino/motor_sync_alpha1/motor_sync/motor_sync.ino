/* Encoder Library - TwoKnobs Example
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 *
 * This example code is in the public domain.
 */

#include <Encoder.h>

#define TOGGLE_CLOCKWISE 10
#define TOGGLE_ANTI_CLOCKWISE 11

#define SPEED_CONTROL_LEFT 12
#define SPEED_CONTROL_RIGHT 13

#define DIRECTION_CONTROL_LEFT 14
#define DIRECTION_CONTROL_RIGHT 15

#define CLOCKWISE LOW
#define ANTICLOCKWISE HIGH

#define DEFAULT_SPEED 255

#define SYNC_THRESHOLD 30

// Change these pin numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder leftMotor(33, 32);
Encoder rightMotor(28, 27);

void setup() {
  Serial.begin(9600);

  pinMode(TOGGLE_CLOCKWISE, INPUT_PULLUP); // up position of full speed toggle switch
  pinMode(TOGGLE_ANTI_CLOCKWISE, INPUT_PULLUP); // down position of full speed toggle switch

  pinMode(DIRECTION_CONTROL_LEFT, OUTPUT);
  pinMode(DIRECTION_CONTROL_RIGHT, OUTPUT);

  // start both motors as off
  analogWrite(SPEED_CONTROL_LEFT, 0);
  analogWrite(SPEED_CONTROL_RIGHT, 0);
}

long positionLeft  = -999;
long positionRight = -999;

void loop() {
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

  // track the motor direction for later when checking encoder difference
  int motorDirection = 0; // default to off
  
  // read toggle switch to determine direction of motors
  if((digitalRead(TOGGLE_CLOCKWISE) == HIGH) && ( digitalRead(TOGGLE_ANTI_CLOCKWISE) == HIGH)) {
    // both motors off 
    analogWrite(SPEED_CONTROL_LEFT, 0);
    analogWrite(SPEED_CONTROL_RIGHT, 0);
    
  } else {
    // in theory only one can be LOW at a time - since its a toggle switch
    if (digitalRead(TOGGLE_CLOCKWISE) == LOW) {
      digitalWrite(DIRECTION_CONTROL_LEFT, CLOCKWISE);
      digitalWrite(DIRECTION_CONTROL_RIGHT, CLOCKWISE);
      motorDirection = 1;
      
    } else  if (digitalRead(TOGGLE_ANTI_CLOCKWISE) == LOW) {
      digitalWrite(DIRECTION_CONTROL_LEFT, ANTICLOCKWISE);
      digitalWrite(DIRECTION_CONTROL_RIGHT, ANTICLOCKWISE);
      motorDirection = -1;
      
    }
    analogWrite(SPEED_CONTROL_LEFT, DEFAULT_SPEED);
    analogWrite(SPEED_CONTROL_RIGHT, DEFAULT_SPEED);
    
    // now - having set motor speeds and diretion - check if they are in sync or not
    // note the PJRC library uses clockwise as positive direction; that is a higher position count
    // tells us that the motor has progressed further clockwise.
    // NOTA BENE - we need to be sure out motors are wired correctly to match !!
    long difference = abs(positionLeft - positionRight);
  
    if (difference > SYNC_THRESHOLD){
  
      // if motors are off and also out of synch - wait, what???
      if(motorDirection == 0){
        Serial.print("WTF? motors are off and also out of synch ");
      }
  
      
      // now slow the offending motor proportionally to how far it is out of synch
      // Given a motor RPM of about 9000, and an output RPM of 500, and 64 encoder
      // ticks per revolution, 1000 ticks should be nearly one complete output 
      // revolution, so lets take that as a limiting difference and stop the motor
      // entirely when the difference is that high, otherwise slow the motor by
      // the difference as a percentage of 1000
      if (difference > 1000) {
        difference = 1000;
      }
      float slowFraction = (float(1000) - float(difference)) / float(1000);
      int slowedMotorSpeed = (int) (255 * slowFraction);
      
      // clockwise - so the motor with the higher count has gone too far - slow it down
      if(motorDirection == 1){
        if(positionLeft > positionRight){
           analogWrite(SPEED_CONTROL_LEFT, slowedMotorSpeed);
        } else {
          analogWrite(SPEED_CONTROL_RIGHT, slowedMotorSpeed);
        }
      
      // anticlockwise - so the motor with the lower count has gone too far - slow it down
      } else if (motorDirection = -1){
        if(positionLeft < positionRight){
           analogWrite(SPEED_CONTROL_LEFT, slowedMotorSpeed);
        } else {
          analogWrite(SPEED_CONTROL_RIGHT, slowedMotorSpeed);
        }
      }
    }
  }

  
  // wait for a little bit to run motors for small amount of time - before checking
  // if they are in sync again - note that because encoder inputs are on 
  // interrupt pins, the position counts will still be incremented (or decremented)
  // during the delay
  
  delay(20);
}












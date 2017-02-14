/* Encoder Library - TwoKnobs Example
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 *
 * This example code is in the public domain.
 */

#define ESP32 1
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
Encoder leftMotor(17, 16);
Encoder rightMotor(23, 18);

void setup() {
  Serial.begin(115200);
  Serial.printf("ok - setup done\n");
  delay(2000);
  Serial.println("turn off led");
  int boardLed = 5;
  pinMode(boardLed, OUTPUT);
  digitalWrite(boardLed, LOW);// Turn off on-board blue led
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH);// Turn off on-board blue led

}

void loop() {
  long newLeft, newRight;
  newLeft = leftMotor.read();
  newRight = rightMotor.read();
  Serial.printf("left motor reading %d\n", newLeft);
  Serial.printf("right motor reading %d\n", newRight);
  
  
  // wait for a little bit to run motors for small amount of time - before checking
  // if they are in sync again - note that because encoder inputs are on 
  // interrupt pins, the position counts will still be incremented (or decremented)
  // during the delay
  
  delay(2000);
}












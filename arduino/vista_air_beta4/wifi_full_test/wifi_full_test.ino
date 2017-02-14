
// needed for hacked encoder library
#define ESP32 1

#include <Encoder.h>

#include <string.h>
#include <stddef.h>


#include  <WiFi.h>

const char* WIFI_SSID = "ESP32";
WiFiServer* server = new WiFiServer(80);
boolean isAP = false;

// not sure whether last byte of MAC is C or D - conflicting reports, internal ESP32 compiled lib wrappers
// report D during startup, but my code below reports C
byte mac[] = {
  0x24, 0x0A, 0xC4, 0x04, 0xBB, 0xDC
};
//IPAddress ip(192, 168, 4, 177);


// pins on the Sparkfun ESP32 used for PWM
#define M1PIN 14
#define M2PIN 27

#define M1CHANNEL 1
#define M2CHANNEL 2

#define CLOCKWISE 1
#define ANTICLOCKWISE 2

#define LEFT_MOTOR 0
#define RIGHT_MOTOR 1

#define SYNC_THRESHOLD 10
#define SYNC_DIFF_LIMIT 200

#define PWM_MAX 255

int inApin[2] = {26, 13};  // INA: Clockwise input
int inBpin[2] = {25, 33}; // INB: Counter-clockwise input
int ledcs[2] = {M1CHANNEL, M2CHANNEL}; // PWM input

int boardLed = 5;

Encoder leftMotor(16, 17);
Encoder rightMotor(18, 23);

void startWifiAP(){
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID);
  
  delay(2000); 
  uint8_t MAC_array[6];
  char MAC_char[18];
  WiFi.macAddress(MAC_array);
  for (int i = 0; i < sizeof(MAC_array); ++i){
    sprintf(MAC_char,"%s%02x:",MAC_char,MAC_array[i]);
  }
  
  // note MAC address last digit changes after we switch from default station mode to AP mode.
  Serial.printf("\nAP mac address is %s\n", MAC_char); //- See more at: http://www.esp8266.com/viewtopic.php?f=29&t=3587#sthash.IXbCrpCP.dpuf
  
  // seems the default AP IP is 192.168.4.1?? - although, dont need that, client phone can determine AP IP
  server->begin();
  isAP = true;
}

void setupMotors(){
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
  
void setup(void) {
  Serial.begin(115200);
  delay(500);

  setupMotors();
  
  // when board starts up, begin as an Access Point (AP) - so that controller clients (phones, tablets) can 
  // connect and supply the local wife access details (AP name and password)
  startWifiAP();
  
  Serial.println("turn off led");
  pinMode(boardLed, OUTPUT);
  digitalWrite(boardLed, LOW);// Turn off on-board blue led
 
}

boolean motorServer = false;

// take the SSID and password submitted, and log on to the access point.
void handleWifiSetupRequest(String request) {
  
  int start = request.indexOf('/');
  int middle = request.indexOf('/', start+1);
  int last = request.indexOf('/', middle+1);

  String ssid = request.substring(start+1, middle);
  String password = request.substring(middle+1, last);

  
  Serial.printf("Connecting to %s with %s\n", ssid.c_str(), password.c_str());
  
  WiFi.mode(WIFI_STA);
  isAP = false;
  WiFi.begin(ssid.c_str(), password.c_str());


  // confirm connection
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // start http server on port 60 listening for requests with "stop" "up" and "down" in the url path
  // but first delete the old one on port 80 that we used to get local AP credentials
  delete server;
  server = new WiFiServer(60);
  server->begin();
  motorServer = true;
}

void motorOff(int motor){
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
 
 pwm: should be a value between ? and 255(6?), higher the number, the faster
 it'll go
 */
void motorGo(uint8_t motor, uint8_t direct, uint8_t pwm){
  if (motor <= 1){
    if (direct <=4){
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

int motorDirection = 0; // stop
int motorSpeed = 0; // also stop

/** 
 * check motors are in or out of synch - reduce the speed of one of them (or stop it) if it's ahead
 * and then run motors at that setting for 10ms before next cycle.
 * This needs to be called from main loop - and all other activities in main loop need to take 
 * next to no time once we are in motor driving mode
 */
void motorSyncLogic(){

  motorGo(0, motorDirection, motorSpeed);
  motorGo(1, motorDirection, motorSpeed);

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
    int slowedMotorSpeed = (int) (motorSpeed * slowFraction);
    
    // clockwise - so the motor with the higher count has gone too far - slow it down
    if(motorDirection == CLOCKWISE){
      if(positionLeft > positionRight){
         motorGo(LEFT_MOTOR, motorDirection, slowedMotorSpeed);
      } else {
         motorGo(RIGHT_MOTOR, motorDirection, slowedMotorSpeed);
      }
    
    // anticlockwise - so the motor with the lower count has gone too far - slow it down
    } else if (motorDirection == ANTICLOCKWISE){
      if(positionLeft < positionRight){
         motorGo(LEFT_MOTOR, motorDirection, slowedMotorSpeed);
      } else {
         motorGo(RIGHT_MOTOR, motorDirection, slowedMotorSpeed);
      }
    }
  }
  // 10 milliseconds seems like a reasonable amount of time (according to finger in the air) to run the motors before
  // starting a new cycle to check if they are in synch 
  delay(10); 
}

/**
 *  really want to use websockets, but for now (until esp32 has WS lib) making do with URL paths 
 */
void handleMotorCommand(String request) {
  int start = request.indexOf('/');
  int last = request.indexOf('/', start+1);

  String command = request.substring(start+1, last);
  Serial.print("received command ");
  Serial.println(command);
  
  if(command.equals("up")){
    motorDirection = 1;
    motorSpeed = PWM_MAX;
  } else if (command.equals("down")){
    motorDirection = 2;
    motorSpeed = PWM_MAX;
  } else if (command.equals("stop")){
    motorDirection = 0;
    motorSpeed = 0;
  }
}

/**
 * if we have a client http submission (of local AP access details) -  or if we've already handled those
 * and are now on to motor commands and have received one of those then return true
 */
boolean checkForClientRequest(){
  // Check if a client has connected
  WiFiClient client = server->available();
  if (client) {  
    // Read the first line of the request
    String request = client.readStringUntil('\r');
    Serial.print("Request: ");
    Serial.println(request);
  
    client.flush();
  
    String httpResponse = "HTTP/1.1 200 OK\r\n";
    String content = "<!DOCTYPE HTML><html></html>\r\n";
  
    client.printf("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %u\r\n\r\n%s",
      content.length(), content.c_str());
    client.flush();

    if(isAP){
      handleWifiSetupRequest(request);
      
    } else if (motorServer){
      handleMotorCommand(request);
      
    } else {
      return false;
    }
    return true;
  }
  return false;
}

boolean waitingMessage = true;
/**
 * this is it - the main loop 
 */
void loop(void) {
  if(isAP || motorServer){
    motorSyncLogic();
    if(checkForClientRequest()){
    } else {
      long timeModulo = millis() %  5000;
      if ((timeModulo < 20) && waitingMessage){
        if(isAP){
          Serial.println("waiting for client wifi to connect and tell us local AP details");
        } else if(motorServer){
          Serial.println("waiting for motor command");          
        }
        waitingMessage = false;
      } else if (timeModulo >= 20) {
        waitingMessage = true;
      }
    }
  }
}





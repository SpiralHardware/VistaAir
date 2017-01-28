#include <WiFi.h>
#include <string.h>
#include <stddef.h>

const char* WIFI_SSID = "ESP32";
WiFiServer server(80);

#define M1PIN 12
#define M2PIN 14
#define M1CHANNEL 1
#define M2CHANNEL 2
#define CW   1
#define CCW  2

boolean isAP = false;

int inApin[2] = {27, 13};  // INA: Clockwise input
int inBpin[2] = {26, 25}; // INB: Counter-clockwise input
int ledcs[2] = {M1CHANNEL, M2CHANNEL}; // PWM input

// not sure whether last byte of MAC is C or D - conflicting reports, internal ESP32 compiled lib wrappers
// report D during startup, but my code below reports C
byte mac[] = {
  0x24, 0x0A, 0xC4, 0x04, 0xBB, 0xDC
};
IPAddress ip(192, 168, 4, 177);

void setup(void) {

  
  Serial.begin(115200);
  delay(3000);

  Serial.println("turn off led");
  int boardLed = 5;
  pinMode(boardLed, OUTPUT);
  digitalWrite(boardLed, LOW);// Turn off on-board blue led

  delay(2000);
  Serial.println("test motor 1");
  
  // Initialize digital pins as outputs
  for (int i=0; i<2; i++)
  {
    pinMode(inApin[i], OUTPUT);
    pinMode(inBpin[i], OUTPUT);
  }
  // Initialize braked
  for (int i=0; i<2; i++)
  {
    digitalWrite(inApin[i], LOW);
    digitalWrite(inBpin[i], LOW);
    pinMode(ledcs[i], OUTPUT);
  }
  
  // assign pwm pins to ledc channels
  ledcAttachPin(M1PIN, M1CHANNEL);
  ledcAttachPin(M2PIN, M2CHANNEL);

  // intialise the ledc channels
  ledcSetup(M1CHANNEL, 12000, 8); // 12 kHz PWM, 8-bit resolution
  ledcSetup(M2CHANNEL, 12000, 8);

  // ledcWrite(channel, dutycycle)
  ledcWrite(M1CHANNEL, 255);

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
  server.begin();
  isAP = true;

  Serial.println("try motor go");
  Serial.println("try motor go");
  motorGo(0, CW, 255);
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
  
// take the SSID and password submitted, and log on to the access point.
void handleRequest(String request) {
  
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
  
}

void loop(void) {

  if(isAP){
    // Check if a client has connected
    WiFiClient client = server.available();
    if (client) {

      // Wait until the client sends some data
      Serial.println("New client");
      delay(1);
    
      // Read the first line of the request
      String request = client.readStringUntil('\r');
      Serial.print("Request: ");
      Serial.println(request);
    
      client.flush();
    
      String httpResponse = "HTTP/1.1 200 OK\r\n";
      String content = "<!DOCTYPE HTML>\r\n<html><h1>thanks for that</h1></html>\r\n";
    
      Serial.println("Response:");
      Serial.println(content);
      client.printf("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %u\r\n\r\n%s",
        content.length(), content.c_str());
      client.flush();
    
      delay(1);
      Serial.println("Client disonnected"); //When method ends
    
      handleRequest(request);
    } else{
      delay(10);
      yield();
    }
  } else {
    delay(5000);
    Serial.println(WiFi.localIP());
  }
}





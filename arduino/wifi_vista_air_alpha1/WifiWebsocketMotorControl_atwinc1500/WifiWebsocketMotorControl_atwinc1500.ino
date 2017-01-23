/*
 * WebSocketServer.ino
 *
 *  Created on: 22.05.2015
 *
 */

#include <Arduino.h>

#include <WiFi101.h>
#include <WebSocketsServer.h>

//#define USE_SERIAL Serial
#define MOTOR_DIRECTION_PIN 14
#define SPEED_PIN 4

char ssid[] = "WimbledonAveTalkTalk";      //  your network SSID (name)
char pass[] = "dunedin1";   // your network password

WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
            
        case WStype_CONNECTED:
            {
                // IPAddress ip = webSocket.remoteIP(num); // only for ESP8266
                Serial.printf(" Client Connected \n");
        				// send message to client
        				webSocket.sendTXT(num, "Connected");
            }
            break;

        // we expect only one button to be pressed at a time, if second button is pressed before receiving a stop
        // command this will cause issues, client websocket apps should prevent this.
        case WStype_TEXT:
        
            Serial.printf("[%u] get Text: %s\n", num, payload);
            
            if(payload[0] == 'u'){
              digitalWrite(MOTOR_DIRECTION_PIN, 0);
              analogWrite(SPEED_PIN, 255);
              Serial.printf("going up\n");
              
            } else if (payload[0] == 'd'){
              digitalWrite(MOTOR_DIRECTION_PIN, 1);
              analogWrite(SPEED_PIN, 255);
              Serial.printf("going down\n");
              
            } else if (payload[0] == 's'){
              analogWrite(SPEED_PIN, 0);
              Serial.printf("stopping\n");
              
            } else {
              Serial.printf("unrecognised command from App\n");
            }

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
            
        case WStype_BIN:
            Serial.printf("[%u] get binary lenght: %u\n", num, lenght);
            //hexdump(payload, lenght);

            // send message to client
            // webSocket.sendBIN(num, payload, lenght);
            break;
    }

}

// to handle continuous button press, use onClick and "onClickUp" rather than 
// reading a continuous stream of "button down" while button is pressed.
// more efficient and easier to implement

int status = WL_IDLE_STATUS;
void setup() {
  
  Serial.begin(9600);      // initialize serial communication
  
  Serial.println();
  Serial.println();
  Serial.println();
  
  //Configure pins for Adafruit ATWINC1500 Breakout
  WiFi.setPins(8,3,4);

  for(uint8_t t = 4; t > 0; t--) {
      Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
      Serial.flush();
      delay(1000);
  }


  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);       // don't continue
  }

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    
    // wait 5 seconds for connection:
    delay(5000);
    Serial.printf("connection status : %d\n", status);
  }
  printWifiStatus();                        // you're connected now, so print out the status
  
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  pinMode(MOTOR_DIRECTION_PIN, OUTPUT);
  pinMode(SPEED_PIN, OUTPUT);
    
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}


void loop() {
    webSocket.loop();
}


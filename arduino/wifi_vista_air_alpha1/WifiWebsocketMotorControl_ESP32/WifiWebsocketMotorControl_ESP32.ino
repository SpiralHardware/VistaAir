/*
 * WebSocketServer.ino
 *
 *  Created on: 22.05.2015
 *
 */

#define ESP32 1

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsServer.h>

WiFiMulti WiFiMulti;

WebSocketsServer webSocket = WebSocketsServer(81);

#define USE_SERIAL Serial
#define MOTOR_DIRECTION_PIN 14
#define SPEED_PIN 4
#define MOTOR_SPEED_CHANNEL 5

/* analogWrite susbstitues :
 *  
      //SigmaDelta Setup(pin 16, channel 0)
      sdSetup(0, 10000);//channel, frequency
      sdAttachPin(16, 0);//pin, channel
      //SigmaDelta Write
      sdWrite(0, 128);//channel, 8bit value
      
      //LEDC Setup(pin 17, channel 5)
      ledcSetup(5, 10000, 16);//channel, frequency, bits
      ledcAttachPin(17, 5);//pin, channel
      //LEDC Write
      ledcWrite(5, 1024);//channel, bits value
      
      //DAC (pins 25 or 26)
      sdWrite(25, 128);//pin, 8bit value
      sdWrite(26, 65);//pin, 8bit value
 */
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
            
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        				// send message to client
        				webSocket.sendTXT(num, "Connected");
            }
            break;

        // we expect only one button to be pressed at a time, if second button is pressed before receiving a stop
        // command this will cause issues, client websocket apps should prevent this.
        case WStype_TEXT:
        
            USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
            
            if(payload[0] == 'u'){
              digitalWrite(MOTOR_DIRECTION_PIN, 0);
              ledcWrite(MOTOR_SPEED_CHANNEL, 1023);//channel, bits value
              USE_SERIAL.printf("going up\n");
              
            } else if (payload[0] == 'd'){
              digitalWrite(MOTOR_DIRECTION_PIN, 1);
              ledcWrite(MOTOR_SPEED_CHANNEL, 1023);
              USE_SERIAL.printf("going down\n");
              
            } else if (payload[0] == 's'){
              ledcWrite(MOTOR_SPEED_CHANNEL, 0);
              USE_SERIAL.printf("stopping\n");
              
            } else {
              USE_SERIAL.printf("unrecognised command from App\n");
            }

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
            
        case WStype_BIN:
            USE_SERIAL.printf("[%u] get binary lenght: %u\n", num, lenght);
            // send message to client
            // webSocket.sendBIN(num, payload, lenght);
            break;
    }
}

// to handle continuous button press, use onClick and "onClickUp" rather than 
// reading a continuous stream of "button down" while button is pressed.
// more efficient and easier to implement

void setup() {
    // USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);

    //Serial.setDebugOutput(true);
    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    WiFiMulti.addAP("WimbledonAveTalkTalk", "dunedin1");

    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    pinMode(MOTOR_DIRECTION_PIN, OUTPUT);
    pinMode(SPEED_PIN, OUTPUT);
  
    //LEDC Setup(pin 17, channel 5)
    ledcSetup(MOTOR_SPEED_CHANNEL, 10000, 16);//channel, frequency, bits
    ledcAttachPin(SPEED_PIN, MOTOR_SPEED_CHANNEL);//pin, channel
    //LEDC Write
    ledcWrite(MOTOR_SPEED_CHANNEL, 0);//channel, bits value
}

void loop() {
    yield();
    //ESP.deepSleep(1000);
    webSocket.loop();
}


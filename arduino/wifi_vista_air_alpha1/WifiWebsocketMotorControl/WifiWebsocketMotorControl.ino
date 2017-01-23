/*
 * WebSocketServer.ino
 *
 *  Created on: 22.05.2015
 *
 */

#include <Arduino.h>

#include <WiFi101.h>
#include <WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>

#define USE_SERIAL Serial
#define MOTOR_DIRECTION_PIN 14
#define SPEED_PIN 4

WiFiMulti WiFiMulti;

WebSocketsServer webSocket = WebSocketsServer(81);

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
              analogWrite(SPEED_PIN, 255);
              USE_SERIAL.printf("going up\n");
              
            } else if (payload[0] == 'd'){
              digitalWrite(MOTOR_DIRECTION_PIN, 1);
              analogWrite(SPEED_PIN, 255);
              USE_SERIAL.printf("going down\n");
              
            } else if (payload[0] == 's'){
              analogWrite(SPEED_PIN, 0);
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
            hexdump(payload, lenght);

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
    
}

void loop() {
    yield();
    //ESP.deepSleep(1000);
    webSocket.loop();
}


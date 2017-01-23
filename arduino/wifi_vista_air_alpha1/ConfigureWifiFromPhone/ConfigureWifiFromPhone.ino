#include <WiFi.h>
#include <string.h>
#include <stddef.h>

const char* WIFI_SSID = "ESP32";
WiFiServer server(80);

boolean isAP = false;

// not sure whether last byte of MAC is C or D - conflicting reports, internal ESP32 compiled lib wrappers
// report D during startup, but my code below reports C
byte mac[] = {
  0x24, 0x0A, 0xC4, 0x04, 0xBB, 0xDC
};
IPAddress ip(192, 168, 4, 177);

void setup(void) {
  Serial.begin(115200);

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





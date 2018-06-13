#include <ESP8266WiFi.h>

const char* ssid     = "DIMAp-Visitante";
const char* password = "v1s1t4nt3@d1m4p";

const char* host = "dweet.io";
const int httpPort = 80;

String inputString="";
boolean stringComplete;
String value;

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  Serial.print("connecting to ");
  Serial.println(host);
  
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  serialEvent();

  if (stringComplete) {
    Serial.print(inputString);
    value = inputString;//.toInt();
    inputString = "";
    stringComplete = false;
  }
  
  String url = "/dweet/for/test?hello=";
  url += "SADDAS";
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  //delay(100);
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  Serial.println();
  Serial.println("closing connection");
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == 126) {
      inputString = "";
    }
    if (inChar != 13 && inChar != 126) {    // add it to the inputString:
      inputString += inChar;
    }
    if (inChar == 13) { //13 is ASCI carriage return
      stringComplete = true;
    }
  }
}

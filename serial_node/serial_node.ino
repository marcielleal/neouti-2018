#include <ESP8266WiFi.h>

const char* ssid     = "DIMAp-Visitante";
const char* password = "v1s1t4nt3@d1m4p";

//const char* host = "www.dweet.io";
const char* host = "10.9.99.38";
const int httpPort = 8080;

String inputString="";
boolean stringComplete;
String value="";


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

  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);  
}


void loop() {
  Serial.print("connecting to ");
  Serial.println(host);
  
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    digitalWrite(D1,HIGH);
    return;
  }
  
  digitalWrite(D1,HIGH);
  digitalWrite(D2,LOW);
  
  serialEvent(); 
  value="";
  
  if (stringComplete) {
    //Serial.print(inputString);
    value = inputString;//.toInt();
    inputString = "";
    stringComplete = false;
    digitalWrite(D1,LOW);
  }
  
  //String url = "/dweet/for/neouti?alive=true&";
  String url ="/neouti.php?"+value;
  
  Serial.print("Requesting URL: ");
  Serial.println(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  
  digitalWrite(D2,HIGH);             
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      digitalWrite(D1,HIGH);
      break;
    }
  }
//  Serial.println("RESPONSE:");
//  while(client.available()){
//    String line = client.readStringUntil('\r');
//    Serial.print(line);
//  }
  
  Serial.println();
  Serial.println("closing connection");
  delay(500);
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == 126) {
      inputString = "";
    }
    if (inChar != 13 && inChar != 126 && !stringComplete) {    // add it to the inputString:
      inputString += inChar;
    }
    if (inChar == 13) { //13 is ASCI carriage return
      stringComplete = true;
    }
  }
}

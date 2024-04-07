#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <FS.h>

const char* ssid = "NSUT_WIFI";
const char* password = "";


//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.43.36:8000/temperature";
String rfid = "iWLl1iwYjHaYhyJEgOpx"; // Variable to store the detected RFID UID
String location="Delhi";
float temp;

JsonArray arr;


bool iswifi = false;

void setup() {
  Serial.begin(115200);
  temp=readTemperature();
  delay(10);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Not Working >>>");
  }

  iswifi=true;
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop() { 
  temp=readTemperature();
  if (isnan(temp)) {
    Serial.println("No value assigned to temp variable");
    return;
  }
  int status = 0;
  if(iswifi){
    status = sendToBackend();
    delay(3600000);
  }else{
    // try wifi
    WiFi.begin(ssid, password);
    int times=0;
    while (WiFi.status() != WL_CONNECTED && times<2) {
      delay(500);
      Serial.print(".");
      times++;
    }
    iswifi = WiFi.status() == WL_CONNECTED;
    Serial.println("");

    if(iswifi){
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());

      status=sendToBackend();
    }
  }
  // status = logAndAuth();
  // readAllDataFromFile();
  Serial.println("");
  Serial.println(status);

}


int sendToBackend(){
    WiFiClient client;
    HTTPClient http;
    
    // Your Domain name with URL path or IP address with path
    http.begin(client, serverName);

    // If you need Node-RED/server authentication, insert user and password below
    //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

    // Specify content-type header
    // Data to send with HTTP POST
    
    http.addHeader("Content-Type", "application/json");

    // {"rfid": "adsfas", "time": "1231./123/12"}
    // date is static for now (either get dedicated hardware or use wifi)
    String date = "2012-04-23T18:25:43.511Z";
    String data = "{\"rfid\":\"" + rfid + "\", \"" + "temperature\":\"" + temp + "\"}";
    Serial.println(data);
    int httpResponseCode = http.POST(data); 
    // delay(5000);
    // Serial.print("HTTP Response code: ");
    // Serial.println(httpResponseCode);
      
    // Free resources
    http.end();

    return httpResponseCode;
}


float readTemperature(){
  
  int sensorValue = analogRead(A0);
  float voltage = (sensorValue / 1023.0) * 5000; // Convert analog value to voltage (mV)
  float temperatureC = voltage / 10; // Convert voltage to temperature (°C)
  String res="This is temp : ";
  res+=(temperatureC);
  Serial.println(res);

  return temperatureC;
}

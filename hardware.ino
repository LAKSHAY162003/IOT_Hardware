#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <FS.h>

const char* ssid = "NSUT_WIFI";
const char* password = "";


// Define constants for file paths
const char* logFilePath = "/log.txt";

//Your Domain name with URL path or IP address with path
String serverName = "http://10.100.193.107:8000/temperature";
String rfid = "cA7Pge96eqHOozgQobuc"; // Variable to store the detected RFID UID
String location="Delhi";
float temp;

JsonArray arr;
bool gotdata=false;
bool iswifi = false;
bool dataPresentInBuffer=false;

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
  
  initializeSPIFFS();

  File log = SPIFFS.open(logFilePath, "w");
  if (!log) {
    Serial.println("Failed to open file for writing");
  }
  else{
    log.close();
    Serial.println("log file created");
  }

}


void loop() { 
  temp=readTemperature();
  if (isnan(temp)) {
   
    Serial.println("No value assigned to temp variable");
    return;
  }
  logAndAuth();
  int status = 0;
  if(iswifi){
    dataPresentInBuffer=false;
    status = sendToBackend();
    delay(5000);
  }else{
    dataPresentInBuffer=true;
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

      readAllDataFromFile(logFilePath);
    }
  }
  // status = logAndAuth();
  // readAllDataFromFile();
  Serial.println("");

}



// PRINT ALL THE LOG FILE DATA !! { VVIMP } And send it to the cloud !! { temp data !! }
void readAllDataFromFile(String path) {
  WiFiClient client;
  HTTPClient http;  
  // Open the file for reading
  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  // Read data line by line
  Serial.println("data: ");
  while (file.available()) {
    // Read a line from the file
    String data = file.readStringUntil('\n');
    // delay(5000);
    // Serial.print("HTTP Response code: ");
    // Serial.println(httpResponseCode);
    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST(data); 
    
    // Free resources
    http.end();
  }

  // Close the file
 file.close();
}


// PUT DATA INTO THE LOG FILE IF WIFI NOT AVAILABLE !!
int logAndAuth(){
  // Open the file for reading
  String data = "{\"rfid\":\"" + rfid + "\", \"" + "temperature\":\"" + temp + "\"}";

  File log = SPIFFS.open(logFilePath, "a");
  if (!log) {
      Serial.println("Failed to open file for writing");
      return 500;
  }
  log.println(rfid+" "+temp);
  Serial.println("log created");
  log.close();
  return 200;
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

// Function to initialize SPIFFS
void initializeSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("Failed to initialize SPIFFS");
    return;
  }
  Serial.println("SPIFFS initialized successfully");
}


float readTemperature(){
  
  int sensorValue = analogRead(A0);
  float voltage = (sensorValue / 1023.0) * 3300; // Convert analog value to voltage (mV)
  float temperatureC = voltage / 10; // Convert voltage to temperature (°C)
  String res="This is temp : ";
  res+=(temperatureC);
  Serial.println(res);

  return temperatureC;
}

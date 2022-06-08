#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

const int MOVEMENT_SENSOR_PIN = 12;
const int LIGHT_SENSOR_PIN = 14;
const int FLUID_LEVEL_SENSOR_PIN = 27;
const int MOISTURE_SENSOR_PIN = 26;
const int PUMP_CONTROL_PIN = 13;
const int BUZZER_PIN = 33;

const char *ssid = "yourAP";
const char *password = "yourPassword";
WiFiServer server(8080);

void checkIfPlantNeedsWater( void *pvParameters );
TaskHandle_t checkIfPlantNeedsWaterHandle;
void checkWaterLevel( void *pvParameters );
void pumpingTask( void *pvParameters );
TaskHandle_t pumpingTaskHandle;
void watchForMovement( void *pvParameters );
TaskHandle_t watchForMovementHandle;

int getMoisture();
int getWaterLevel();
void activateWaterPump();
void deactivateWaterPump();
void sendMessageToUser(String msg);
void buzz();
bool detectMovement();

String connectionParams[5];

// the setup function runs once when you press reset or power the board
void setup() {

  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);


  pinMode(MOVEMENT_SENSOR_PIN, INPUT); //digital movement sensor
  pinMode(LIGHT_SENSOR_PIN, INPUT); //digital light sensor
  pinMode(FLUID_LEVEL_SENSOR_PIN, INPUT); //analog fluid level sensor
  pinMode(MOISTURE_SENSOR_PIN, INPUT); //analog moisture sensor
  pinMode(PUMP_CONTROL_PIN, OUTPUT);

  // Now set up tasks to run independently.
  xTaskCreate(checkIfPlantNeedsWater, "check if plant shoud be watered", 1024, NULL, 2, &checkIfPlantNeedsWaterHandle);
  xTaskCreate(checkWaterLevel, "check water level in tank", 1024, NULL, 2, NULL);
  xTaskCreate(pumpingTask, "pump water", 1024, NULL, 2, &pumpingTaskHandle);
  xTaskCreate(watchForMovement, "watch for movement", 1024, NULL, 2, &watchForMovementHandle);

  xTaskCreate(runWifi, "runWifi", 20000, NULL, 2, NULL);

  pinMode(2, OUTPUT);
  digitalWrite(2, 5000);


}

void loop()
{
  // Empty. Things are done in Tasks.
}

int getMoisture() {
  return analogRead(MOISTURE_SENSOR_PIN);
}

int getWaterLevel() {
  return analogRead(FLUID_LEVEL_SENSOR_PIN);
}

void activateWaterPump() {
  digitalWrite(PUMP_CONTROL_PIN, HIGH);
  Serial.println("Activated water pump....");
}

void deactivateWaterPump() {
  digitalWrite(PUMP_CONTROL_PIN, LOW);
  Serial.println("Deactivated water pump....");
}

void sendMessageToUser(String msg) {
  Serial.print("Sending message: ");
  Serial.println(msg);
}

bool detectMovement() {
  return digitalRead(MOVEMENT_SENSOR_PIN);
}

void buzz() {
  Serial.println("BUZZZZZZZZZZ BUZZZZZZ BUUUUUUZZZ!1!");
}

void checkIfPlantNeedsWater(void *pvParameters) {

  int checkDelay = 1000;
  int moistureThreshold = 3000;

  for (;;) {
    vTaskDelay(checkDelay);
    Serial.print("checking moisture: ");
    Serial.print(getMoisture());
    Serial.print(" | moisture threshold: ");
    Serial.println(moistureThreshold);
    if (getMoisture() > moistureThreshold) {
      vTaskResume(pumpingTaskHandle);
      vTaskSuspend(checkIfPlantNeedsWaterHandle);
    }
  }
}

void checkWaterLevel(void *pvParameters) {

  int checkDelay = 1000;
  int waterLevelThreshold = 500;

  for (;;) {
    vTaskDelay(checkDelay);
    Serial.print("checking water level: ");
    Serial.print(getWaterLevel());
    Serial.print(" | water level threshold: ");
    Serial.println(waterLevelThreshold);
    if (getWaterLevel() < waterLevelThreshold) {
      sendMessageToUser("Refill the tank");
      vTaskResume(watchForMovementHandle);
    }
    else {
      vTaskSuspend(watchForMovementHandle);
    }
  }
}

void pumpingTask(void *pvParameters) {

  int pumpingPeriod = 1000;

  vTaskSuspend(pumpingTaskHandle);

  for (;;) {
    activateWaterPump();
    vTaskDelay(pumpingPeriod);
    deactivateWaterPump();
    vTaskResume(checkIfPlantNeedsWaterHandle);
    vTaskSuspend(pumpingTaskHandle);
  }
}

void watchForMovement(void *pvParameters) {

  int checkInterval = 200;

  vTaskSuspend(watchForMovementHandle);

  for (;;) {
    vTaskDelay(checkInterval);
    if (detectMovement()) {
      buzz();
      vTaskDelay(2000);
    }
  }
}

void runWifi( void *pvParameters ) {
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  for (;;) {
    
    WiFiClient client = server.available();

    if (client) {
      String currentLine = "";
      int start_ = 0;
      int stop_ = 0;
      int counter = 0;
      while (client.connected()) {
        if (client.available()) {
          char c = client.read();
          Serial.write(c);
          if (c == '\n') {
            if (currentLine.length() == 0) {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();

              client.print("<h1>Network configuration</h1><form method=\"GET\"><label>SSID: <label><input type=\"text\" name=\"ssid\" /><br /><label>Password: <label><input type=\"text\" name=\"password\" /><br /><label>IP: <label><input type=\"text\" name=\"ip\" /><br /><label>Netmask: <label><input type=\"text\" name=\"netmask\" /><br /><label>Gateway: <label><input type=\"text\" name=\"gateway\" /><br /><button type=\"submit\">Save</button>");

              client.println();
              break;
            } else {
              currentLine = "";
            }
          } else if (c != '\r') {
            currentLine += c;
          }

          if(currentLine.endsWith("=")){
            start_ = currentLine.length();
          }
          if(currentLine.endsWith("&")){
            stop_ = currentLine.length()-1;
          }
          if(currentLine.endsWith(" HTTP/1.1")){
            stop_ = currentLine.length() - 9;
          }
          
          if (counter < 5 && (currentLine.endsWith("&") || currentLine.endsWith(" HTTP/1.1") ) ){
            connectionParams[counter] = currentLine.substring(start_, stop_);
            counter++;
          }
          
        }
      }
      client.stop();
      Serial.println("Client Disconnected.");
      for(int i = 0; i < 5; i++){
        Serial.println(connectionParams[i]);
      }

      Serial.println(counter);

      if(counter == 5){
        Serial.write("Run as client\n");
        xTaskCreate(runWifi2, "runWifi", 40000, NULL, 2, NULL);
        vTaskDelete(NULL);
      }
    }
  }
}

void runWifi2( void *pvParameters ) {
  
  delay(2000);
    

  const char* ssid     = connectionParams[0].c_str();;
  const char* password = connectionParams[1].c_str();;

  IPAddress ip;
  IPAddress netmask;
  IPAddress gateway;

  ip.fromString(connectionParams[2]);
  netmask.fromString(connectionParams[3]);
  gateway.fromString(connectionParams[4]);
  
  
  WiFiServer server2(80);
  
  delay(10);
  WiFi.config(ip, gateway, netmask);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  server2.begin();

  for(;;){
    WiFiClient client = server2.available();

    if (client) {
      String currentLine = "";
      while (client.connected()) {
        if (client.available()) {
          char c = client.read();
          if (c == '\n') {
            if (currentLine.length() == 0) {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();

              client.print("Movement sensor ");
              client.print(digitalRead(MOVEMENT_SENSOR_PIN));
              client.print("<br>Light sensor ");
              client.print(digitalRead(LIGHT_SENSOR_PIN));
              client.print("<br>Fluid level sensor ");
              client.print(analogRead(FLUID_LEVEL_SENSOR_PIN));
              client.print("<br>Moisture sensor ");
              client.print(analogRead(MOISTURE_SENSOR_PIN));
              client.print(" ");
              client.print(digitalRead(25));
              client.print("<br>Pump ");
              client.print(digitalRead(PUMP_CONTROL_PIN));
              client.print("<br>Buzzer ");
              client.print(digitalRead(BUZZER_PIN));

              client.println();
              break;
            } else {
              currentLine = "";
            }
          } else if (c != '\r') {
            currentLine += c;
          }

          if (currentLine.endsWith("GET /H")) {
            Serial.write("first action\n");
          }
          if (currentLine.endsWith("GET /L")) {
            Serial.write("second action\n");
          }
        }
      }
      client.stop();
      Serial.println("Client Disconnected.");
    }
  }

}

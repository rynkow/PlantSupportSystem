const int MOVEMENT_SENSOR_PIN = 12;
const int LIGHT_SENSOR_PIN = 14;
const int FLUID_LEVEL_SENSOR_PIN = 27;
const int MOISTURE_SENSOR_PIN = 26;
const int PUMP_CONTROL_PIN = 13;
const int BUZZER_PIN = 33;

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
}

void loop()
{
  // Empty. Things are done in Tasks.
}

int getMoisture(){
  return analogRead(MOISTURE_SENSOR_PIN);
}

int getWaterLevel(){
  return analogRead(FLUID_LEVEL_SENSOR_PIN);
}

void activateWaterPump(){
  digitalWrite(PUMP_CONTROL_PIN, HIGH);
  Serial.println("Activated water pump....");
}

void deactivateWaterPump(){
  digitalWrite(PUMP_CONTROL_PIN, LOW);
  Serial.println("Deactivated water pump....");
}

void sendMessageToUser(String msg){
  Serial.print("Sending message: ");
  Serial.println(msg);
}

bool detectMovement(){
  return digitalRead(MOVEMENT_SENSOR_PIN);
}

void buzz(){
  Serial.println("BUZZZZZZZZZZ BUZZZZZZ BUUUUUUZZZ!1!");
}

void checkIfPlantNeedsWater(void *pvParameters){

  int checkDelay = 1000;
  int moistureThreshold = 3000;

  for(;;){
      vTaskDelay(checkDelay);
      Serial.print("checking moisture: ");
      Serial.print(getMoisture());
      Serial.print(" | moisture threshold: ");
      Serial.println(moistureThreshold);
      if (getMoisture() > moistureThreshold){
        vTaskResume(pumpingTaskHandle);
        vTaskSuspend(checkIfPlantNeedsWaterHandle);
      }
  }
}

void checkWaterLevel(void *pvParameters){

  int checkDelay = 1000;
  int waterLevelThreshold = 500;

  for(;;){
      vTaskDelay(checkDelay);
      Serial.print("checking water level: ");
      Serial.print(getWaterLevel());
      Serial.print(" | water level threshold: ");
      Serial.println(waterLevelThreshold);
      if (getWaterLevel() < waterLevelThreshold){
        sendMessageToUser("Refill the tank");
        vTaskResume(watchForMovementHandle);
      }
      else{
        vTaskSuspend(watchForMovementHandle);
      }
  }
}

void pumpingTask(void *pvParameters){

  int pumpingPeriod = 1000;

  vTaskSuspend(pumpingTaskHandle);

  for(;;){
      activateWaterPump();
      vTaskDelay(pumpingPeriod);
      deactivateWaterPump();
      vTaskResume(checkIfPlantNeedsWaterHandle);
      vTaskSuspend(pumpingTaskHandle);
  }
}

void watchForMovement(void *pvParameters){

  int checkInterval = 200;

  vTaskSuspend(watchForMovementHandle);

  for(;;){
      vTaskDelay(checkInterval);
      if(detectMovement()){
        buzz();
        vTaskDelay(2000);
      }
  }
}

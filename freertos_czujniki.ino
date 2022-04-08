void checkIfPlantNeedsWater( void *pvParameters );
void checkWaterLevel( void *pvParameters );

int getMoisture();
int getWaterLevel();
void activateWaterPump();
void sendMessageToUser(char* msg);

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

  
  pinMode(12, INPUT); //digital movement sensor
  pinMode(14, INPUT); //digital light sensor
  pinMode(27, INPUT); //analog fluid level sensor
  pinMode(26, INPUT); //analog moisture sensor
  pinMode(25, INPUT); //digital moisture sensor

  // Now set up tasks to run independently.
  xTaskCreate(checkIfPlantNeedsWater, "check if plant shoud be watered", 1024, NULL, 2, NULL);
  xTaskCreate(checkWaterLevel, "check water level in tank", 1024, NULL, 2, NULL);

}

void loop()
{
  // Empty. Things are done in Tasks.
}

int getMoisture(){
  return analogRead(25);
}

int getWaterLevel(){
  return analogRead(27);
}

void activateWaterPump(){
  Serial.println("pumping Water....");
}

void sendMessageToUser(char* msg){
  Serial.println(msg);
}

void checkIfPlantNeedsWater(void *pvParameters){

  int checkDelay = 1000;
  int moistureThreshold = 50;

  for(;;){
      vTaskDelay(checkDelay);
      if (getMoisture() < moistureThreshold){
        activateWaterPump();
      }
  }
}

void checkWaterLevel(void *pvParameters){

  int checkDelay = 1000;
  int waterLevelThreshold = 50;

  for(;;){
      vTaskDelay(checkDelay);
      if (getWaterLevel() < waterLevelThreshold){
        sendMessageToUser("Refill the tank");
      }
  }
}

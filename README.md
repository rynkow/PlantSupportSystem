# PlantSupportSystem

System wspomagający użytkownika w hodowli roślin doniczkowych.

## Funkcjonalności

### Przewidywane
* monitorowanie wilgotności gleby i automatyczne podlewanie
* monitorowanie dostępnej ilości wody i powiadamianie użytkownika o potrzebie uzupełnienia zbiornika
  * za pomocą WiFi
  * za pomocą buzzera w przypadku wykrycia ruchu
  * za pomocą bluetooth 
  * za pomocą sygnalizacji świetlnej
* monitorowanie natężenia światła
  * powiadamianie o potrzeby przeniesienia rośliny w miejsce o bardziej sprzyjających warunkach
  * doświetlanie specjalnymi LEDami 

### Zrealizowane
* monitorowanie wilgotności gleby i automatyczne podlewanie
* monitorowanie dostępnej ilości wody i powiadamianie użytkownika o potrzebie uzupełnienia zbiornika
   * za pomocą buzzera w przypadku wykrycia ruchu
   * za pomocą sygnalizacji świetlnej
* komunikacja przez wifi
   * dostęp do aktualnych wartości czujników
   * modyfikacja wartości progowych
* monitorowanie natężenia światła 
   * doświetlanie LEDami 
   
## wykorzystany sprzęt
### Mikrokontroler
[ESP32-DEVKITC-VE](https://www.tme.eu/pl/details/esp32-devkitc-ve/zestawy-uruchomieniowe-pozostale/espressif/)
### Czujniki
* czujnik wilgotności gleby
* czujnik poziomu cieczy

### Inne moduły
 * pompa cieczy
 * buzzer

## Schemat
TU SCHEMAT

## Działanie
Wykorzystaliśmy system FreeRTOS i stworzyliśmy szereg zadań, które operują poszczególnymi elementami systemu.

### Zadania

### `checkIfPlantNeedsWater`
Zadanie ma na celu regularne sprawdzanie, czy poziom wilgotności gleby nie spadł poniżej progu.
W przypadku przekroczenia minimalnej dozwolonej wilgotności uruchamiany jest task `pumpingTask` a  task `checkIfPlantNeedsWater` jest zawieszany do czasu zakończenia pompowania.

```C++
void checkIfPlantNeedsWater(void *pvParameters) {

  int checkDelay = 1000;
  int moistureThreshold = 3000;

  for (;;) {
    vTaskDelay(checkDelay);
    if (getMoisture() > MOISTURE_THRESHOLD) {
      vTaskResume(pumpingTaskHandle);
      vTaskSuspend(checkIfPlantNeedsWaterHandle);
    }
  }
}
```

### `checkWaterLevel`
Zadanie ma na celu regularne sprawdzanie, czy poziom cieczy w zbiorniku nie spadł poniżej progu.
W przypadku przekroczenia progu wysyłana jest wiadomość do użytkownika, uruchamiany jest task `watchForMovement` i włączana dioda.
Gdy zbiornik zostanie uzupełniony (tzn. gdy task `checkWaterLevel` wykryje poziom wody wyższy bądź równy od progu) task `watchForMovement` jest zawieszany, a dioda wyłączana.


```C++
void checkWaterLevel(void *pvParameters) {

  int checkDelay = 1000;
  int waterLevelThreshold = 500;

  for (;;) {
    vTaskDelay(checkDelay);
    if (getWaterLevel() < WATER_LEVEL_THRESHOLD) {
      digitalWrite(FLUID_LEVEL_LED_PIN, HIGH);
      sendMessageToUser("Refill the tank");
      vTaskResume(watchForMovementHandle);
    }
    else {
      digitalWrite(FLUID_LEVEL_LED_PIN, LOW);
      vTaskSuspend(watchForMovementHandle);
    }
  }
}
```

### `pumpingTask`
Zadanie jest odpowiedzialne za kontrole pompy. Gdy zostanie wznowione uruchamia pompę na określony stałą okres czasu, po czym wznawia task `checkIfPlantNeedsWater` i zawiesza własne działanie.


```C++
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
```

### `watchForMovement`
Zadanie sprawdza regularnie (z dość dużą częstotliwością) czy czujnik ruchy nie wykrył  jakieś aktywności.
W przypadku wykrycia ruchu zadanie uruchamia buzzer w celu przyciągnięcia uwagi użytkownika i zawiesza działanie na dłuższy okres czasu (aby sygnał był powtarzany z częstotliwością nieirytującą użytkownika).


```C++
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
```

### `checkLightLevel`
Zadanie sprawdza, czy natężenie światła jest odpowiednie. Gdy nie jest aktywowana jest dioda doświetlająca.


```C++
void checkLightLevel(void *pvParameters) {

  int checkDelay = 1000;
  int ledDuration = 10000;

  for (;;) {
    vTaskDelay(checkDelay);
    if (digitalRead(LIGHT_SENSOR_PIN) == 1) {
      sendMessageToUser("Low light level");
      digitalWrite(GROWTH_LED_PIN, HIGH);
      vTaskDelay(ledDuration);
      digitalWrite(GROWTH_LED_PIN, LOW);
    }
  }
}
```


### `runWifi`
Zadanie odpowiada za nawiązanie połączania przez WiFi oraz obsługę zapytań przychodzących od klienta. W odpowiedzi zwracany jest aktualny stan czujników.




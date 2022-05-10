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
* komunikacja przez wifi

## wykorzystany sprzęt
### Mikrokontroler
[ESP32-DEVKITC-VE](https://www.tme.eu/pl/details/esp32-devkitc-ve/zestawy-uruchomieniowe-pozostale/espressif/)
### Czujniki
* czujnik wilgotności gleby
* czujnik poziomu cieczy

### Inne moduły
 * pompa cieczy
 * buzzer

## Działanie
Wykorzystaliśmy system FreeRTOS i stworzyliśmy szereg zadań, które operują poszczególnymi elementami systemu.

### Zadania

### `checkIfPlantNeedsWater`
Zadanie ma na celu regularne sprawdzanie, czy poziom wilgotności gleby nie spadł poniżej progu.
W przypadku przekroczenia minimalnej dozwolonej wilgotności uruchamiany jest task `pumpingTask` a  task `checkIfPlantNeedsWater` jest zawieszany do czasu zakończenia pompowania.

### `checkWaterLevel`
Zadanie ma na celu regularne sprawdzanie, czy poziom cieczy w zbiorniku nie spadł poniżej progu.
W przypadku przekroczenia progu wysyłana jest wiadomość do użytkownika oraz uruchamiany jest task `watchForMovement`.
Gdy zbiornik zostanie uzupełniony (tzn. gdy task `checkWaterLevel` wykryje poziom wody wyższy bądź równy od progu) task `watchForMovement` jest zawieszany.

### `pumpingTask`
Zadanie jest odpowiedzialne za kontrole pompy. Gdy zostanie wznowione uruchamia pompę na określony stałą okres czasu, po czym wznawia task `checkIfPlantNeedsWater` i zawiesza własne działanie.

### `watchForMovement`
Zadanie sprawdza regularnie (z dość dużą częstotliwością) czy czujnik ruchy nie wykrył  jakieś aktywności.
W przypadku wykrycia ruchu zadanie uruchamia buzzer w celu przyciągnięcia uwagi użytkownika i zawiesza działanie na dłuższy okres czasu (aby sygnał był powtarzany z częstotliwością nieirytującą użytkownika).

### `runWifi`
Zadanie odpowiada za nawiązanie połączania przez WiFi oraz obsługę zapytań przychodzących od klienta. W odpowiedzi zwracany jest aktualny stan czujników.




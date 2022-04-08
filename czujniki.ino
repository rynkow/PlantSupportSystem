void setup() {

  pinMode(12, INPUT);
  pinMode(14, INPUT);
  pinMode(27, INPUT);
  pinMode(26, INPUT);
  pinMode(25, INPUT);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");
}

void loop() {
  Serial.println("Czujnik ruchu");
  Serial.println(digitalRead(12));
  Serial.println("Czujnik światła");
  Serial.println(digitalRead(14));
  Serial.println("Czujnik poziomu cieczy");
  Serial.println(analogRead(27));
  Serial.println("Czujnik wilgotności gleby");
  Serial.println(analogRead(26));
  Serial.println(digitalRead(25));

  delay(100);
}

/*
  Blink

  Schaltet die interne rote LED des ftDuino für eine Sekunde ein
  und für eine Sekunde aus und wiederholt dies endlos.

  Original:
  http://www.arduino.cc/en/Tutorial/Blink
*/

// die setup-Funktion wird einmal beim Start aufgerufen
void setup() {
  // Konfiguriere den Pin, an den die interne LED angeschlossen ist, als Ausgang
  pinMode(LED_BUILTIN, OUTPUT);
}

// die loop-Funktion wird immer wieder aufgerufen
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // schalte die LED ein (HIGH ist der hohe Spannungspegel)
  delay(1000);                       // warte 100 Millisekunden (eine Sekunde)
  digitalWrite(LED_BUILTIN, LOW);    // schalte die LED aus, indem die Spannung auf
  			    	     // niedrigen Pegel (LOW) geschaltet wird
  delay(1000);                       // warte eine Sekunde
}

/*
   ComPort - Kommunikation mit dem PC über den COM:-Port

 */

int zaehler = 0;

void setup() {
  // Port initialisieren und auf USB-Verbindung warten
  Serial.begin(9600);
  while(!Serial);      // warte auf USB-Verbindung

  Serial.println("ftDuino COM:-Port test");
}

void loop() {
  Serial.print("Zähler: ");       // gib "Zähler:" aus
  Serial.println(zaehler, DEC);   // gib zaehler als Dezimalzahl aus

  zaehler = zaehler+1;            // zaehler um eins hochzählen

  delay(1000);                    // warte 1 Sekunde (1000 Millisekunden)
}

#include <Ftduino.h>

void setup(void) {
  Serial.begin(115200);
  while(!Serial);         // warte auf USB-Verbindung zu PC
  
  ftduino.init();
  ftduino.ultrasonic_enable(Ftduino::ON);
}

void loop(void) {
  int16_t distance = ftduino.ultrasonic_get();
  String dist_str = String(distance) +"cm";
  if(distance < 0)           dist_str = "...";  // kein gueltiger Werte
  else if(distance == 1023)  dist_str = ">10m"; // maximale Distanz erreicht

  Serial.println("Distance: " + dist_str);
  delay(1000);
}

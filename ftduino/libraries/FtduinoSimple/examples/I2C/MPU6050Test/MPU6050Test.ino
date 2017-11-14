// MPU-6050-Beispiel-Sketch
//
// Quelle:
// MPU-6050 Short Example Sketch
// By Arduino User JohnChi
// August 17, 2014
// Public Domain

#include <Wire.h>

#define MPU_ADDR 0x68  // I2C-Adresse des MPU-6050

// Variablen, um die Messwerte des MPU-6050 zu speichern
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

void setup() {
  // Der MP-6050 muss "aufgeweckt" werden, wenn er Messwerte liefern soll,
  // da er direkt nach dem Einschalten in einem Energiesparmodus ist
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);                  // PWR_MGMT_1-Register
  Wire.write(0);                     // Auf Null setzen (weckt MPU-6050 auf)
  Wire.endTransmission(true);

  // Starte serielle Ausgabe zum PC/seriellen Monitor
  Serial.begin(9600);
  while(!Serial);
}

void loop(){

  // Register des MPU_6050 auslesen
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);                    // lies ab Register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR,14,true);  // hole den Inhalt von insgesamt 14 Registern
  
  AcX = Wire.read()<<8 | Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
  AcY = Wire.read()<<8 | Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read()<<8 | Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read()<<8 | Wire.read();  // 0x41 (TEMP_OUT_H)   & 0x42 (TEMP_OUT_L)
  GyX = Wire.read()<<8 | Wire.read();  // 0x43 (GYRO_XOUT_H)  & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read()<<8 | Wire.read();  // 0x45 (GYRO_YOUT_H)  & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read()<<8 | Wire.read();  // 0x47 (GYRO_ZOUT_H)  & 0x48 (GYRO_ZOUT_L)

  // alle linearen Beschleunigungswerte ausgeben
  Serial.print("AcX = "); Serial.print(AcX);
  Serial.print(" | AcY = "); Serial.print(AcY);
  Serial.print(" | AcZ = "); Serial.print(AcZ);
  
  Serial.print(" | Tmp = "); Serial.print(Tmp/340.00+36.53);  // Gleichung für Grad-Celsius aus dem Datenblatt

  // alle Drehbeschleunigungen ausgeben
  Serial.print(" | GyX = "); Serial.print(GyX);
  Serial.print(" | GyY = "); Serial.print(GyY);
  Serial.print(" | GyZ = "); Serial.println(GyZ);
  
  delay(500);
}

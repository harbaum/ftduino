// CounterPorts/Bridge.ino
//
// Bridge between a bluetooth module connected to ftDuino counter ports C1
// (and optional C2)

void setup() 
{
    Serial.begin(9600);
    Serial1.begin(9600); 
};

void loop() 
{
    while(Serial1.available() && Serial.availableForWrite())
      Serial.write(Serial1.read());
      
    if(Serial.available() && Serial1.availableForWrite())
      Serial1.write(Serial.read());
};

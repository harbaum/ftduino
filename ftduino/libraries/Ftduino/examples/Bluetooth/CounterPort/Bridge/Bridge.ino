// CounterPorts/Bridge.ino
//
// Bridge between a bluetooth module connected to ftDuino counter ports C1
// (and optional C2)

void setup() 
{
    Serial.begin(9600);
    while(!Serial);
       
    Serial1.begin(9600); 
};

void loop() 
{
    while(Serial1.available())
      Serial.write(Serial1.read());
      
    if(Serial.available())
      Serial1.write(Serial.read());
};

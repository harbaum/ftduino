#include <Wire.h>

uint8_t search() {
  uint8_t address;
  Serial.println("Searching for MiniServoAdapter ...");

  for(address = 3; address <= 0x77; address++ ) {
    Wire.beginTransmission(address);
    Wire.write(0x01);  // read flag
    
    if(Wire.endTransmission() == 0) {   
      Wire.requestFrom(address, 0x01);
      while(!Wire.available());
      uint8_t flag = Wire.read();
      Wire.endTransmission();     
      
      // device found at address
      Serial.print("I²C device found at address ");
      Serial.print(address, DEC);
      Serial.print(" (0x");
      Serial.print(address, HEX);
      Serial.println(")");

      // flag must be 0x5A
      if(flag != 0x5a) {
        Serial.print("Not a MiniServoAdapter (flag = ");
        Serial.print(flag, HEX);
        Serial.println(")");        
      } else {       
        // check version
        Wire.beginTransmission(address);
        Wire.write(0x00);  // read version
        Wire.endTransmission();     
        Wire.requestFrom(address, 0x01);
        while(!Wire.available());
        uint8_t version = Wire.read();
        Wire.endTransmission();     
        
        Serial.print("MiniServoAdapter V");
        Serial.print(version/16, DEC);
        Serial.print(".");
        Serial.print(version%16, DEC);
        Serial.println(" found.");        

        return address;
      }
    }
  }

  Serial.println("No MiniServoAdapter found!");  
  return 0x00;   // no device found
}

void setup() {
  Wire.begin();
  
  Serial.begin(9600);  
  while (!Serial);
  Serial.println("\n>> MiniServoAdapter address changer <<\n");
}

uint8_t get_number() {
  // 0xXX or XXX
  
  // wait for input
  Serial.setTimeout(100);
  while(!Serial.available());
  String s = Serial.readString();
  Serial.print(s);

  // hex number?
  if(s.startsWith("0x") || s.startsWith("0X")) {
    byte tens = (s[2] < '9') ? s[2] - '0' : 
        (s[2] >= 'a' && s[2] <= 'f') ? s[2] - 'a' + 10 :
        (s[2] >= 'A' && s[2] <= 'F') ? s[2] - 'A' + 10 : 0;
    byte ones = (s[3] < '9') ? s[3] - '0' :
        (s[3] >= 'a' && s[3] <= 'f') ? s[3] - 'a' + 10 :
        (s[3] >= 'A' && s[3] <= 'F') ? s[3] - 'A' + 10 : 0;
    return (16 * tens) + ones;
  }

  return s.toInt();
}

void loop() {
  uint8_t addr;

  // empty serial buffer
  while(Serial.available())
    Serial.read();
    
  // search ...
  while((addr = search()) == 0) 
    delay(5000);  // retry after 5 seconds

  Serial.print("\nPlease enter new address: ");

  uint8_t newaddr = get_number();
  
  Serial.print("New address ");
  Serial.print(newaddr, DEC);
  Serial.print(" (0x");
  Serial.print(newaddr, HEX);
  Serial.println(").");
      
  if((newaddr < 3) || (newaddr > 0x77) || (newaddr == 0x3c)) {
    Serial.println("\nERROR: The new address is out of range. It must be ");
    Serial.println("in the range of 3 (0x03) to 119 (0x77) and must ");
    Serial.println("not be 60 (0x3c).\n");
  } else {  
    Serial.println("\nDo you really want to change the address (Y/N)?");

    while(!Serial.available());
    char y = Serial.read();
    if(y != 'y' && y != 'Y')
      Serial.println("\nAddress change aborted!");
    else {
      // set new address
      Wire.beginTransmission(addr);
      Wire.write(0x08);  // set address
      Wire.write(newaddr);
      Wire.endTransmission();     
      
      Wire.beginTransmission(addr);
      Wire.write(0x09);  // store in eeprom
      Wire.write(0xa5);
      Wire.endTransmission();     

      Serial.println("\nAddress changed!\n");

      delay(1000);
    }
  }
}


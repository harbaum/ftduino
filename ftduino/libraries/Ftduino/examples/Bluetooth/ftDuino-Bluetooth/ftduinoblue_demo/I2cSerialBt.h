#include <inttypes.h>
#include <Stream.h>

class I2cSerialBt : public Stream
{
public:
  I2cSerialBt();
  ~I2cSerialBt() { end(); }
  void begin(uint32_t baud);
  void end();

  bool check(int timeout);
  void key(bool on);
  int available();
  int availableForWrite();
  int read();
  int peek();
  void flush();
  size_t write(uint8_t byte);

private:
  uint8_t registerRead(char r);
  void registerWrite(char r, uint8_t d);
};


#include <Arduino.h>
#include <inttypes.h>
#include <Stream.h>

class FirmataSerial : public Stream
{
public:
  FirmataSerial(Stream &);
  ~FirmataSerial() { end(); }
  void begin();
  void end();
  int available();
  int availableForWrite();
  int read();
  int peek();
  void flush();
  size_t write(uint8_t byte);

private:
  Stream *m_s;
  bool parse_sysex(byte st, byte b);
  void parse(byte b);
  void poll(void);

  byte uart_buffer;
  bool uart_buffer_valid;
};

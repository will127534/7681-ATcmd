#include "Arduino.h"
#include <IPAddress.h>

class LC7681Wifi
{
  public:
  typedef void (*LC7681WifiCallback)(int event, const uint8_t* data, int dataLen);

  enum {
    EVENT_NONE,
    EVENT_DATA_RECEIVED,
    EVENT_SOCKET_DISCONNECT,
    EVENT_MAX
  };
  
public:
  LC7681Wifi(Stream *s, Stream* l = NULL);

  void begin();
  bool connectAP(const char* ssid, const char* key,int type=9);
  IPAddress s2ip(const char* str);
  IPAddress IP();
  IPAddress nslookup(const char* server);
  bool connect(IPAddress ip, int port, bool udp = false);
  bool connect(const char* ip, int port, bool udp = false);
  bool print(const char* data, int dataLen);
  bool print(const char* data = NULL);
  bool println(const char* data = NULL);
  bool listenTo(uint16_t port, bool udp);
  void process(LC7681WifiCallback cb);

  Stream *m_stream;
private:
  String _wait_for(const char* pattern,uint32_t timeout = 300);
  int freeRam();
private: 
  
  Stream *m_log;
  
  uint16_t m_lport;
  int m_bufferPos;
  char m_buffer[70];
};

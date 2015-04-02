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
  void process(LC7681WifiCallback cb);

private:
  String _wait_for(const char* pattern, unsigned int timeout = 300);

private:
  Stream *m_stream;
  Stream *m_log;
  
  int m_lport;
  int m_bufferPos;
  char m_buffer[128];
};

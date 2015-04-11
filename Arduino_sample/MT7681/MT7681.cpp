
#include <Arduino.h>
#include <Stream.h>
#include <MT7681.h>
static int base64_encode_len(int len)
{
  return ((len+2)/3)*4;
}

static uint8_t base64_enc_map(uint8_t n)
{
  if(n < 26)
  return 'A'+n;
  if(n < 52)
  return 'a'+(n-26);
  if(n < 62)
  return '0'+(n-52);
  return n == 62 ? '+' : '/';
}

static void base64_encode(const uint8_t* src, int len, uint8_t* dest)
{
  uint32_t w;
  uint8_t t;
  while(len >= 3)
  {
    w = ((uint32_t)src[0])<<16 | ((uint32_t)src[1])<<8 | ((uint32_t)src[2]);

    dest[0] = base64_enc_map((w>>18)&0x3F);
    dest[1] = base64_enc_map((w>>12)&0x3F);
    dest[2] = base64_enc_map((w>>6)&0x3F);
    dest[3] = base64_enc_map((w)&0x3F);

    len-=3;
    src+=3;
    dest+=4;
  }
  if(!len)
  return;

  if(len == 2)
  {
    w = ((uint32_t)src[0])<<8 | ((uint32_t)src[1]);

    dest[0] = base64_enc_map((w>>10)&0x3F);
    dest[1] = base64_enc_map((w>>4)&0x3F);
    dest[2] = base64_enc_map((w&0x0F)<<2);
    dest[3] = '=';
  }
  else
  {
    w = src[0];

    dest[0] = base64_enc_map((w>>2)&0x3F);
    dest[1] = base64_enc_map((w&0x03)<<4);
    dest[2] = '=';
    dest[3] = '=';
  }
}

static int base64_decode_len(int len)
{
  return ((len+3)/4)*3;
}

static uint32_t base64_dec_map(uint8_t n)
{
  if(n >= 'A' && n <= 'Z')
  return n - 'A';
  if(n >= 'a' && n <= 'z')
  return n - 'a' + 26;
  if(n >= '0' && n <= '9')
  return n - '0' + 52;
  return n == '+' ? 62 : 63;
}

static int base64_decode(const uint8_t* src, int len, uint8_t* dest)
{
  uint32_t w;
  uint8_t t;
  int result = 0;

    // remove trailing =
    while(src[len-1] == '=')
    len--;

    while(len >= 4)
    {
      w = (base64_dec_map(src[0]) << 18) |
      (base64_dec_map(src[1]) << 12) |
      (base64_dec_map(src[2]) << 6) |
      base64_dec_map(src[3]);

      dest[0] = (w>>16)&0xFF;
      dest[1] = (w>>8)&0xFF;
      dest[2] = (w)&0xFF;

      len-=4;
      src+=4;
      dest+=3;
      result+=3;
    }
    if(!len)
    return result;

    if(len == 3)
    {
      w = (base64_dec_map(src[0]) << 18) |
      (base64_dec_map(src[1]) << 12) |
      (base64_dec_map(src[2]) << 6) |
      0;

      dest[0] = (w>>16)&0xFF;
      dest[1] = (w>>8)&0xFF;
      result+=2;
    }
    else if(len == 2)
    {
      w = (base64_dec_map(src[0]) << 18) |
      (base64_dec_map(src[1]) << 12) |
      0;

      dest[0] = (w>>16)&0xFF;
      result+=1;
    }
    else
    {
        // should not happen.
      }
      return result;
    }


    

    LC7681Wifi::LC7681Wifi(Stream *s, Stream* l):
    m_lport(0)
    {
      m_stream = s;
      m_log = l;
      m_bufferPos = 0;
    }

    void LC7681Wifi::begin()
    {
      
    }
    
    bool LC7681Wifi::connectAP(const char* ssid, const char* key,int type)
    {
      String str = F("AT+WCAP=");
      str += String(ssid);
      str += ",";
      str += String(key);
      str += ",";
      str += String(type);
      m_stream->println(str);
      if(m_log){
      //m_log->print("[Send cmd]");
      //m_log->println(str);
      }
      str = String("+WCAP:") + String(ssid);
      if(m_log){
      //m_log->print("[Wait for]");
      //m_log->println(str);
      }
      if(_wait_for(str.c_str(),50) != 0)
      {
        return true;
      }
      return false;
    }
    
    IPAddress LC7681Wifi::s2ip(const char* str)
    {
      uint32_t ip[4];
      String ipaddress(0);
      int count = 0;

    //  Serial.print("INTPUT IP:");
    //  Serial.print(str);
     // Serial.print(" Size:");
     // Serial.println(strlen(str));
      for (int i = 0; i <= strlen(str); ++i)
      { 
      //  Serial.print(i);
      //  Serial.println(str[i]);
        if(str[i]=='.' || i==strlen(str)){
                 // Serial.print("GET .");
              //    Serial.print(ipaddress);
                  ip[count]=ipaddress.toInt();
                //  Serial.print("=>");
               //   Serial.print(count);
               //   Serial.print("=>");
                //  Serial.println(ip[count]);
                  ipaddress = "";
                  count++;
        }
        else{
          ipaddress+=String(str[i]);

        }
      }
      //sscanf(str, "%d.%d.%d.%d", ip, ip+1, ip+2, ip+3);
      return IPAddress(ip[0], ip[1], ip[2], ip[3]);
    }
    
    IPAddress LC7681Wifi::IP()
    {
      String result, ips;
      m_stream->println(F("AT+WQIP?"));
      result = _wait_for("+WQIP:", 5);
      if(result.length() == 0)
      return IPAddress();
      ips = result.substring(6, result.indexOf(',', 6));
      return s2ip(ips.c_str());
    }
    
    IPAddress LC7681Wifi::nslookup(const char* server)
    {
      String result, ips;
      
      result = F("AT+WDNL=");
      result += server;
      m_stream->println(result);
      //Serial.println(result);
      result = _wait_for("+WDNL:", 5);
      if(result.length() == 0)
      return IPAddress();
      
      ips = result.substring(result.lastIndexOf(',')+1);
      return s2ip(ips.c_str());
    }

    bool LC7681Wifi::connect(IPAddress ip, int port, bool udp )
    {
      char buf[20];
      String AT = String(ip[0])+"." +String(ip[1])+"." +String(ip[2])+"." +String(ip[3]);
     // Serial.println(ip[3]);
      //AT.reserve(AT.length());
      AT.toCharArray(buf, AT.length()+1);

        //sprintf(buf, "AT+WSW=%d,", m_lport);
      
     
      //sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
      return connect(buf, port, udp);
    }
    
     bool LC7681Wifi::connect(const char* ip, int port, bool udp )
  {
    String str = "AT+WSO=";
    str += ip;
    str += ",";
    str += port;
    str += ",";
    str += udp ? "1" : "0";
    m_stream->println(str);
    Serial.println(str);
    str = _wait_for("+WSO:",30);
    if(str.length() == 0)
      return false;
    
    m_lport = str.substring(5).toInt();
    if(udp)
      return true;
      
    str = "+WSS:";
    str += m_lport;
    str = _wait_for(str.c_str(),30);
    Serial.println(str);
    if(str.length() == 0)
    {
     
      m_lport = 0;
      return false;
    }
    str.remove(0,str.lastIndexOf(',')+1);

    
    if(str.toInt() == 0)
    {
     
      
      m_lport = 0;
      return false;
    }

    return true;
  }

  int LC7681Wifi::freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
    bool LC7681Wifi::print(const char* data, int dataLen)
    {
      char buf[100];
      const char *src = data;
      char *dest = buf;
      int remain = dataLen;
      
      while(remain > 0)
      {
        int part = remain > 48 ? 48 : remain;
        String AT = F("AT+WSW=");
        
        
        AT = AT +String(m_lport);
        AT = AT +',';
        //Serial.println(AT);
        AT.toCharArray(buf, AT.length()+1);
        //Serial.println(buf);
        //sprintf(buf, "AT+WSW=%d,", m_lport);
        dest = buf + strlen(buf);
        base64_encode((uint8_t*)src, part, (uint8_t*)dest);
        dest+= base64_encode_len(part);
        dest[0] = '\r';
        dest[1] = '\n';
        dest[2] = 0;
        
        m_stream->print(buf);
        Serial.print(buf);
        src+=part;
        remain-=part;

        String str = _wait_for("+WSDS:",30);
       // Serial.print("Get:");
        //Serial.println(str);
        while(str.length() == 0){
          m_stream->print(buf);
          String str = _wait_for("+WSDS:",30);
        }
       
      }    
      return true;
    }
    
    bool LC7681Wifi::print(const char* data )
    {
      if(!data)
      return false;
      
      return print(data, strlen(data));
    }

    bool LC7681Wifi::println(const char* data)
    {
      String str;
      if(data)
      str = data;
      str += "\r\n";
      
      return print(str.c_str());
    }
    
    void LC7681Wifi::process(LC7681WifiCallback cb)
    {
      int c;
      while(m_stream->available())
      {
        c = m_stream->read();
        m_buffer[m_bufferPos] = (char)c;
        m_bufferPos++;
        if(c == '\n')
        {
          m_buffer[m_bufferPos] = 0;
          m_bufferPos = 0;
          
          if(m_log)
          {
            m_log->print(F("[log]"));
            m_log->println(m_buffer);
          }

          if(!strncmp(m_buffer, "+WSDR:", 6))
          {
            String s = m_buffer+6;
            s.trim();
            int t1 = s.indexOf(',');
            int t2 = s.indexOf(',', t1+1);
            int port = s.substring(0, t1).toInt();
            
            if(port == m_lport)
            {
              int len = s.substring(t1+1, t2).toInt();
              String data = s.substring(t2+1);
              if(m_log && data.length() != len)
              {
                m_log->print("[Warning] length not matching:");
                m_log->print(len);
                m_log->print(data.length());
                m_log->println(data);
              }

            // use m_buffer as temp buffer
            t1 = base64_decode((const uint8_t*)data.c_str(), len, (uint8_t*)m_buffer);
            m_buffer[t1] = 0;
            cb(EVENT_DATA_RECEIVED, (uint8_t*)m_buffer, t1);
          }
        }
        else if(!strncmp(m_buffer, "+WSS:", 5))
        {
          String s = m_buffer+5;
          s.trim();
          int t1 = s.indexOf(',');
          int port = s.substring(0, t1).toInt();
          int state = s.substring(t1+1).toInt();
          if(port == m_lport)
          {
            if(state == 0)
            cb(EVENT_SOCKET_DISCONNECT, NULL, 0);
            m_lport = 0;
          }
        }
        
      }
    }
  }
  
   String LC7681Wifi::_wait_for(const char* pattern, unsigned int timeout)
  {
    unsigned long _timeout = millis() + timeout*1000;
    char buf[128];
    int i, c;
    
    if(m_log)
      m_log->println("[wait_for]");
    
    i = 0;
    while(millis() <= _timeout)
    {
    //  while(Serial.available()){
    //        Serial1.write(Serial.read());
    //  }
      while(m_stream->available())
      {
        c = m_stream->read();
        if(m_log)
          m_log->write(c);

        buf[i] = (char)c;
        i++;
        if(c == '\n')
        {
          buf[i] = 0;
          
          if(0 == strncmp(buf, pattern, strlen(pattern)))
          {
            String result(buf);
            result.trim();
            return result;
          }
          
          i = 0;
        }
      }
    }
    return String(""); // timeout
  }

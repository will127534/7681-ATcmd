
#include "MT7681.h"


LC7681Wifi wifi(&Serial1);


const char ssid[] = "";
const char key[] = "";
const char server[] = "www.google.com";
const int port = 80;


int RST = 13;
void setup() {
   pinMode(RST, OUTPUT);
   digitalWrite(RST, HIGH);
  Serial.begin(115200);


  Serial1.begin(115200);
  while(!Serial)
    delay(100);

  
  // reset module
  Serial.print("[log] Resetting Module...");
  pinMode(RST, OUTPUT);
  digitalWrite(RST, LOW);
  delay(100);
  digitalWrite(RST, HIGH);
  delay(7000);
  Serial.println("done");
  
  wifi.begin();
  
  // attempt to connect to AP
  while(true)
  {
    Serial.println("[log] Trying to connect AP...");
    if(!wifi.connectAP(ssid, key))
    {
      Serial.println("[log] Fail to connect, wait 5 secs to retry...");
      delay(5000);
      continue;
    }

    Serial.println("[log] Connected to AP!");
    break;
  }

  // print status
  delay(1000);
  Serial.print("[log] IP Address: ");
  Serial.println(wifi.IP());

  // lookup server ip
  IPAddress serverIp = wifi.nslookup(server);
  Serial.print("[log] IP Address of ");
  Serial.print(server);
  Serial.print(" is ");
  Serial.println(serverIp);
  
  // connect to server
  while(true)
  {
    Serial.println("[log] Trying to start connection to server...");
    if (!wifi.connect(serverIp, port))
    {
      Serial.println("[log] Fail to connected to server, wait 5 secs to retry...");
      delay(5000);
      continue;
    }
    
    Serial.println("[log] Connected to server!");
    break;
  }
  
  Serial.println("[log] Send HTTP request");
  // Make a HTTP request:
  wifi.println("GET /search?q=arduino HTTP/1.1");
  wifi.println("Host: www.google.com");
  //wifi.println("Connection: close");
  wifi.println();
}

void callback(int event, const uint8_t* data, int dataLen)
{
  switch(event)
  {
  case LC7681Wifi::EVENT_DATA_RECEIVED:
    Serial.print((const char*)data);
    break;
  case LC7681Wifi::EVENT_SOCKET_DISCONNECT:
    Serial.println("[log] Connection to server is closed!");
    break;
  }
}

void loop() 
{
  wifi.process(callback);
}


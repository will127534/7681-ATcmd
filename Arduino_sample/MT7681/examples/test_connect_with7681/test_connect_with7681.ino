#include "MT7681.h"

#define wifiSerial Serial1
LC7681Wifi wifi(&wifiSerial);
                    
const char ssid[] = "Lab430 2.4GHz";
const char key[] = "";
const char server[] = "www.arduino.cc";
const int port = 80;


int RST = 13;
void setup() {
   pinMode(RST, OUTPUT);
   digitalWrite(RST, HIGH);
  Serial.begin(115200);
  wifiSerial.begin(115200);
  while(!Serial || !wifiSerial)
    delay(100);

  // reset module
  Serial.print("[log] Resetting Module...");
  digitalWrite(RST, LOW);
  delay(1000);
  digitalWrite(RST, HIGH);
  while(1){
//    Serial.println("Go!!");
    if(wifiSerial.available()){
    char c = wifiSerial.read(); 
//    Serial.write(c);
    if(c=='<'){
       break;
      }
    }
  }
  delay(1000);
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
  IPAddress serverIp;
  serverIp = wifi.nslookup(server);
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
  
//  Serial.println("[log] Send HTTP request");
//  // Make a HTTP request:
//  wifi.println("GET /asciilogo.txt HTTP/1.1");
//  wifi.println("Host: www.arduino.cc");
//  //wifi.println("Connection: close");
//  wifi.println();

  Serial.println("try to listen to port 8000");
  if(wifi.listenTo(8000,false)) {
    Serial.println("succeed to listen to 8000");
  }
  else {
    Serial.println("failed to listen to 8000");
  }

}

void callback(int event, const uint8_t* data, int dataLen)
{
  switch(event)
  {
  case LC7681Wifi::EVENT_DATA_RECEIVED:
    Serial.print("data received:");
    Serial.println((const char*)data);
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


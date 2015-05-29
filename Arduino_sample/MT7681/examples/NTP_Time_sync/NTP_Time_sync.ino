
#include "MT7681.h"


LC7681Wifi wifi(&Serial1);


const char ssid[] = "";
const char key[] = "";
const char server[] = "time.nist.gov";//tick.stdtime.gov.tw
const int port = 123;
const int RST = 13; //Module RST pin

#define SECONDS_FROM_1970_TO_2000 946684800

//Time related
int timeget = 0;
int hour = 0;
int minute = 0;
int sec = 0;
uint32_t lastmillis = 0;


void setup() {
   pinMode(RST, OUTPUT);
   digitalWrite(RST, HIGH);

   Serial.begin(115200);
   while(!Serial){    //Wait for COM port opening
   }

   Serial1.begin(115200);
   //Setup PWM pin & speed-up PWM Feq
   pinMode(5,OUTPUT);
   pinMode(9,OUTPUT);
   pinMode(10,OUTPUT);
   TCCR3B = TCCR3B & 0b11111000 | 0x01; 

   // reset module
   Serial.print("[log] Resetting Module...");
   digitalWrite(RST, LOW);
   delay(1000);
   digitalWrite(RST, HIGH);
   while(1){
     if(Serial1.available()){
     char c = Serial1.read(); 
     //Serial.write(c);
     if(c=='<'){
        break;
      }
    }
   }
   delay(1000);
   Serial.println("[log] Reset Completed Module...");
   wifi.begin();
  
   //attempt to connect to AP
   while(true){
    Serial.println("[log] Trying to connect AP...");
    if(!wifi.connectAP(ssid, key)){
      Serial.println("[log] Fail to connect, wait 5 secs to retry...");
      delay(5000);
      continue;
    }
    Serial.println("[log] Connected to AP!");
    break;
  }
  timesyncstart();
}

void callback(int event, const uint8_t* data, int dataLen){
  switch(event)
  {
    case LC7681Wifi::EVENT_DATA_RECEIVED:
      if(dataLen == 48){
        // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
        const unsigned long seventyYears = 2208988800UL;
 
        uint32_t epoch = ((uint32_t)data[40]<<24) | ((uint32_t)data[41]<<16) | ((uint32_t)data[42]<<8) | ((uint32_t)data[43]);
        epoch = epoch- seventyYears;
        Ntpparse(epoch);
        timeget = 1;
    }
    break;
    case LC7681Wifi::EVENT_SOCKET_DISCONNECT:
       Serial.println(F("[log] Connection to server is closed!"));
    break;
  }
}

void loop() {
 if(millis()-lastmillis>1000){
   sec++;
   if(sec==60){
     minute++;
     sec=0;
     if(minute==60){
        hour++;
        minute=0;
        if(hour==24){
            hour=0;
          } 
      }
    }
   analogWrite(5,sec*4);
   analogWrite(9,minute*4);
   analogWrite(10,hour*10);
   Serial.print(hour);
   Serial.print(":");
   Serial.print(minute);
   Serial.print(":");
   Serial.println(sec);
   lastmillis=millis();
 }
 
  
}

void Ntpparse(uint32_t t){
    t -= SECONDS_FROM_1970_TO_2000;    // bring to 2000 timestamp from 1970
    uint8_t daysInMonth [] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    sec = t % 60;
    t /= 60;
    minute = t % 60;
    t /= 60;
    hour = t % 24;
    uint32_t days = t / 24;
    uint8_t leap;
    int  yOff = 0;
    int  m = 0;
    int  d = 0;

    for (yOff = 0; ; ++yOff) {
        leap = yOff % 4 == 0;
        if (days < 365 + leap)
            break;
        days -= 365 + leap;
    }

    for (m = 1; ; ++m) {
        uint8_t daysPerMonth = daysInMonth[ m - 1];
        if (leap && m == 2)
            ++daysPerMonth;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }

    d = days + 1;

    Serial.print("NTP Time Sync finished,current time is :");
    Serial.print(hour);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.print(sec);
    Serial.print("  ");
    Serial.print(yOff);
    Serial.print("/");
    Serial.print(m);
    Serial.print("/");
    Serial.println(d);

}
void timesyncstart(){          //Send NTP request
  // lookup server ip
  IPAddress serverIp = IPAddress((uint32_t)0);
  while(serverIp[0]==0){
   serverIp = wifi.nslookup(server);
  }

  Serial.print("[log] IP Address of ");
  Serial.print(server);
  Serial.print(" is ");
  Serial.println(serverIp);

  // connect to server
  while(true)
  {
    Serial.println("[log] Trying to start connection to server...");
    if (!wifi.connect(serverIp, port, true)){
    Serial.println("[log] Fail to connected to server, wait 5 secs to retry...");
    delay(5000);
    continue;
    }
    Serial.println("Connected");
    break;
  }

  Serial.println("[log] Send NTP request");
  char buf[48];
  memset(buf, 0, 48);
  buf[0] = 0x1b;
  wifi.print(buf, 48);
  while(timeget==0){
    wifi.process(callback);
  }
  lastmillis = millis();
}



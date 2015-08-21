
#include "MT7681.h"


LC7681Wifi wifi(&Serial1);


const char ssid[] = "";
const char key[] = "";
const char server[] = "time.nist.gov";//tick.stdtime.gov.tw
const int port = 123;
const int RST = 13; //Module RST pin

#define SECONDS_FROM_1900_TO_2000 3155673600UL;
//Time related
int timeget = 0;
int hour = 0;
int minute = 0;
int sec = 0;
uint32_t lastmillis = 0;
uint32_t  epoch = 0;
//NTP Related
uint32_t  T4_offset = 0;
uint32_t  T1_offset = 0;
uint32_t  T1_num = 0;uint32_t  T1_digit = 0;
uint32_t  T2_num = 0;uint32_t  T2_digit = 0;
uint32_t  T3_num = 0;uint32_t  T3_digit = 0;
uint32_t  T4_num = 0;uint32_t  T4_digit = 0;

uint32_t  delta_num =0;
uint32_t  delta_digit =0;
uint32_t  delaymicros = 0;

uint64_t T1 = 0;
uint64_t T2 = 0;
uint64_t T3 = 0;
uint64_t T4 = 0;
uint64_t  delta = 0;
uint64_t delta_plus = 0;
uint64_t delta_minus = 0;


extern volatile unsigned long timer0_millis;
void setMillis(unsigned long new_millis){ //Prevent overflow

  uint8_t oldSREG = SREG;
  cli();
  timer0_millis = new_millis;
  SREG = oldSREG;
}

void setup() {

   pinMode(RST, OUTPUT);
   digitalWrite(RST, HIGH);
   
   pinMode(9, OUTPUT);
   digitalWrite(9, HIGH);

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
   SerialFlush();
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
        
        T4_offset = micros();
        T2= ((uint64_t)data[32]<<56) | ((uint64_t)data[33]<<48) | ((uint64_t)data[34]<<40) | ((uint64_t)data[35]<<32) | ((uint32_t)data[36]<<24) | ((uint32_t)data[37]<<16) | ((uint32_t)data[38]<<8) | ((uint32_t)data[39]);
        T3= ((uint64_t)data[40]<<56) | ((uint64_t)data[41]<<48) | ((uint64_t)data[42]<<40) | ((uint64_t)data[43]<<32) | ((uint32_t)data[44]<<24) | ((uint32_t)data[45]<<16) | ((uint32_t)data[46]<<8) | ((uint32_t)data[47]);
        //delaymicros = T3_digit;
        //delaymicros = delaymicros >>10;
        T4 = (T4_offset - T1_offset)/1000000UL;
        T4 = T4<<32;
        T4 += ((T4_offset - T1_offset)%1000000UL)/1000*4194304UL/1000;
        T4+= T1;
        delta_plus = 0;
        delta_minus = 0;
        
        if(T2>T1){
              delta_plus = (T2- T1)/2;
        }
        else{
              delta_minus = (T1- T2)/2;
        }
        
        if(T3>T4){
              delta_plus += (T3 - T4)/2;
        }
        else{
              delta_minus += (T4 - T3)/2;
        }
        
        if(delta_plus>delta_minus){
          delta = delta_plus - delta_minus;
        }
        else{
          delta = delta_minus - delta_plus;
        }

        delta_num = delta>>32;
        //delta_num += delta_digit>>32
        //(T2_num- T1_num + T3_num - T4_num)%2;
        delta_digit = delta;
        delaymicros = delta;
        delaymicros +=T4_digit;
        delaymicros = delaymicros>>10;

        if(delta_plus>delta_minus){
          epoch = delta_num+(T4>>32)+1;
          delayMicroseconds(1000000UL - delaymicros*1000/4194304UL*1000);
          Serial.println("PLUS");

        }
        else{
          epoch = (T4>>32)-delta_num;
          delayMicroseconds(delaymicros*1000/4194304UL*1000);
          Serial.println("MINUS");
        }       

        
        
        //setMillis(0);
        T1_num = T1>>32;
        T2_num = T2>>32;
        T3_num = T3>>32;
        T4_num = T4>>32;
        
        
        //Serial.print("T4_digit: ");Serial.println(T4_digit);
        Serial.print("T1: ");Serial.print(T1_num); Serial.print("   T1_offset: ");Serial.println(T1_offset);
        Serial.print("T2: ");Serial.println(T2_num);
        Serial.print("T3: ");Serial.println(T3_num);
        Serial.print("T4: ");Serial.print(T4_num);Serial.print("   T4_offset: ");Serial.println(T4_offset);
        Serial.print("delta: ");Serial.print(delta_num);Serial.print("   delta_digit: ");Serial.println(delta_digit);
        Serial.print("epoch: ");Serial.println(epoch);
        Serial.print("delayMicroseconds: ");Serial.println(1000000UL - delaymicros*1000/4194304UL*1000);

        Serial.println(T4_offset - T1_offset);
        //Serial.println(microseconds,HEX);
        
        Ntpparse(epoch);
        lastmillis=millis();
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
   epoch++;
   Ntpparse(epoch);
   analogWrite(5,sec*4);
  // analogWrite(9,minute*4);
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
    t -= SECONDS_FROM_1900_TO_2000;    // bring to 2000 timestamp from 1970
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

    /*Serial.print("NTP Time Sync finished,current time is :");
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
    */

}
void timesyncstart(){          //Send NTP request
  // lookup server ip

  for(int k=0; k<3; k++){
      timeget=0;
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

      T1_offset = micros();
      T1_digit = (T1_offset - T4_offset)%1000000UL;
      T1_digit = T1_digit/1000*4194304UL/1000;
      
      T1 = epoch + (T1_offset - T4_offset)/1000000UL;
      T1 = T1<<32;
      T1 += T1_digit;
      
      //uint32_t dptime = (micros()-T1_offset);
      
           // 1000/4194304UL*1000
            buf[24] = T1>>56;
            buf[25] = T1>>48;
            buf[26] = T1>>40;
            buf[27] = T1>>32;

            buf[28] = T1>>24;
            buf[29] = T1>>16;
            buf[30] = T1>>8;
            buf[31] = T1;
      
      wifi.print(buf, 48);
      uint32_t start = millis();
      while(timeget==0){
        wifi.process(callback);
        if(millis()-start>10000){
           k --;
           break;
        }
      }
      lastmillis = millis();

}


}

void SerialFlush(){

  while(Serial.available()){
     byte temp = Serial.read();
  }
}
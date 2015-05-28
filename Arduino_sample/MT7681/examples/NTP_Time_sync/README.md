LinkIt Connect 7681 Demo
==============

####LinkIt Connect 7681 是一個 Wi-Fi 模組, 利用它可以讓 Arduino 加上連網的功能

[官方網站在這裡](http://labs.mediatek.com/site/global/developer_tools/mediatek_7681/whatis_7681/index.gsp )
這個範例使用 7681 連接到 NTP server 取得目前的時間

##硬體的部分

建議使用 Arduino Leonardo, 因為它有二個 UART, 一個用來和 7681 連接, 另一個用來和作 Serial output, 

(PS. 若使用 Arduino Uno, 因為只有一個 UART, 在 upload sketch 時要把和 7681 的連線中斷, done 再接回來會比較麻煩)
(以下都用 Leonardo 來作)

接線照著圖來作
![revolunet logo](http://i.imgur.com/tMLCym9.jpg "revolunet logo")
 

	7681 的 RX 接到 Arduino TX
	        TX 接到 RX
	       VIN 接到 5V
	       GND 接到 GND
	       RST 接到 Arduino pin13(或者除了RX-TX以外的任何一支GPIO)

(PS. 這個範例還多使用了 PIN 5/6/10 的 PWM 來控制一顆共陽的 RGB LED)

##軟體的部分

 - 1.首先要更新韌體, [到這裡](https://github.com/will127534/7681-ATcmd) 下載二個 bin 檔和 7681 uploader 程式, 
把 Arduino 更新成 Serial bypass 的模式
```
void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
}

void loop() {
 while(Serial.available()){
  Serial1.write(Serial.read()); 
 }
 while(Serial1.available()){
  Serial.write(Serial1.read()); 
 }
}
```
它的作用是把二個 UART 互串, 讓我們能透過 Arduino 對電腦的 COM port 和 7681 講話
使用 7681 uploader 更新 bin 檔, 作法是在 command line 下輸入

	> mt7681_uploader.exe -f MT7681_sta_header.bin -c COMX

	(PS. 這裡的 COMX 替換成 Arduino 的 com port)

看到 Update done! bye 就表示成功了
(PS. 記得要作二次, 一次是 sta_header.bin, 一次是 ap_header.bin)

![revolunet logo](http://i.imgur.com/PDTB0gq.png "revolunet logo")

 - 2.安裝 7681 的 Arduino library, [在這裡下載](https://github.com/will127534/7681-ATcmd/tree/master/Arduino_sample), 放到Arduino 的 library資料夾就可以

 - 3.打開Arduino IDE>範例>NTP_Timesync.ino 之後, 更新 ssid / key 這二個變數改成你自己的 Wi-Fi 設定, 然後upload sketch

	> const char ssid[] = ""; 

	> const char key[] = "";

   如果有RST接上的GPIO不是PIN13的話記得在修改RST這個變數

 > int RST = 13;

 - 4.順利的話, 可以看到 RGB LED 開始根據時間而不斷的變化,也可以打開 Serial monitor 看到現在的時間

![revolunet logo](http://i.imgur.com/go5Nphb.png "revolunet logo")
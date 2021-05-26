#include "DFRobotDFPlayerMini.h"
#include "SoftwareSerial.h"
#include "Arduino.h"
#include "TM1637Display.h"

#define CLK 12
#define DIO 13

int coordination_state = "ON",numfile, pw = "ON",stay = "OFF"; //初期設定連携ON,読み込み曲数変数,PWスイッチの変数,待機中のテキストを一回だけ送信する変数
const int vol = 25; //ボリューム調整0-30
const int musictime = 25000;//5曲の秒数（1秒=1000ms）


const int now_sw = 2; //強制SW
const int now_led =3;//強制LED
const int coordination_sw = 4;//連携モード変更ボタン
const int coordination_led = 5;//連携LED
const int hand_sw = 6;//手動10分SW
const int interval_led = 7;//１０分モード時LED

const int now_signal =8;//通常信号
const int ten_signal = 9;//10分信号
const int RX = 10,TX = 11; //RXピン,TXピン

const int speaker_sw = 18;//speakerリレーピン
const int system_sw = 19;//キースイッチ入力ピン



TM1637Display display(CLK, DIO);
SoftwareSerial mySoftwareSerial(RX, TX); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

void printDetail(uint8_t type, int value);

const uint8_t SEG_STANDBY[] = {
  255,255,255,255
};
const uint8_t SEG_ON[] = {0,0,
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
  SEG_C | SEG_E | SEG_G  // n
};
const uint8_t SEG_OFF[] = {0,
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
  SEG_A | SEG_E | SEG_F | SEG_G ,                // F
  SEG_A | SEG_E | SEG_F | SEG_G                // F
};
const uint8_t SEG_SLEEP[] = {
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G ,        // S
  SEG_D | SEG_E | SEG_F ,                        // L
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G ,        // E
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G ,        // P
};
const uint8_t SEG_PLAY[] = {
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G ,        // P
  SEG_D | SEG_E | SEG_F ,                        // L
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G ,// A
  SEG_B | SEG_C | SEG_D | SEG_F | SEG_G | SEG_DP // y
};


void setup() {
  pinMode(now_sw, INPUT_PULLUP);//強制ボタン
  pinMode(coordination_sw, INPUT_PULLUP);//連携切替スイッチ
  pinMode(ten_signal, INPUT); //10分信号
  pinMode(coordination_led, OUTPUT);//連携LED
  pinMode(interval_led, OUTPUT);//10分間LED
  pinMode(now_signal, INPUT);//通常信号
  pinMode(now_led, OUTPUT);//通常信号
  attachInterrupt(0, forced,  FALLING);//強制関数
  pinMode(system_sw, INPUT_PULLUP);//キースイッチ切替
  pinMode(speaker_sw, OUTPUT);//スピーカーリレースイッチ
  pinMode(hand_sw, INPUT_PULLUP);//手動スイッチ
  digitalWrite(speaker_sw, HIGH);//スピーカーリレースイッチON
  display.setBrightness(7);//0-7セグメントの輝度調整



  mySoftwareSerial.begin(9600);
    Serial.begin(115200);
    Serial.println(F("＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝"));
    delay(500);
    Serial.println(F("据え置き型相互連動システム「チャイムル」"));
    Serial.println();
    delay(1000);
    Serial.println(F("DFPlayerを初期化しています... (3～5秒程かかる場合があります)"));

    if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
      Serial.println(F("起動出来ませんでした。:"));
      Serial.println(F("1.接続を確認してください"));
      Serial.println(F("2.SDカードを挿入してください"));
      while (true);
    }
    Serial.println();
    Serial.println(F("DFPlayer 起動しました"));
    numfile = myDFPlayer.readFileCounts(DFPLAYER_DEVICE_SD);
    delay(500);
    Serial.print(F("ファイル数"));
    Serial.print(numfile);
    Serial.print(" ");
    Serial.println(F("曲"));
    Serial.println();
    Serial.println(F("＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝"));

  myDFPlayer.volume(vol);  //Set volume value. From 0 to 30
  myDFPlayer.play(1);  //Play the first mp3
  display.setSegments(SEG_STANDBY);//セグメント全点灯

  digitalWrite(coordination_led, HIGH);
  digitalWrite(interval_led, HIGH);
  digitalWrite(now_led,HIGH);
  delay(3000);
  
  display.clear();
   delay(3000);
  digitalWrite(interval_led, LOW);
  digitalWrite(coordination_led, LOW);
  digitalWrite(now_led,LOW);
  delay(1000);
  display.setSegments(SEG_ON);//セグメント[On]表示
  digitalWrite(coordination_led, HIGH);
  delay(5000);

  display.clear();
}

void forced() { //割り込み処理
  myDFPlayer.play(5);
  display.setSegments(SEG_PLAY);
  digitalWrite(now_led,HIGH);
  Serial.println(F("^^強制再生^^"));
  stay="OFF";
}

void forced_switch() {
}
void loop() {
  if (digitalRead(system_sw) == LOW) { //システム電源がONになっているとき
    if (pw == "OFF") {
      digitalWrite(speaker_sw, HIGH);
      restart();
    }

    if (digitalRead(ten_signal) == HIGH || digitalRead(hand_sw) == LOW) { //１０分間処理のとき
      digitalWrite(interval_led, HIGH);
      interval();
      digitalWrite(interval_led, LOW);
      Serial.println(F("------------"));

    }
    if (digitalRead(now_signal) == HIGH && coordination_state == "ON") { //連動ONのときボタンを押したとき
      normal();
      Serial.println(F("------------"));

    } else { //待機中の時
      if (stay == "OFF") {
        Serial.print(F("入力待機中"));
         delay(500);
        Serial.print(F("."));
        delay(500);
        Serial.print(F("."));
        delay(500);
        Serial.println(F("."));
        delay(500);
        Serial.println();
        stay = "ON";
        digitalWrite(now_led,LOW);
        

      }
    }

    if (myDFPlayer.available()) {
      printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
    }

    if (coordination_state == "ON" && digitalRead(coordination_sw) == LOW) {
      coordination_off();
      delay(200);
    } else if (coordination_state == "OFF" && digitalRead(coordination_sw) == LOW) {
      coordination_on();
      delay(200);
    }

  } else if (digitalRead(system_sw) == HIGH) { //システム電源が斬られた時
    if (stay == "ON") {
      myDFPlayer.play(3);
      display.setSegments(SEG_OFF);
      Serial.println(F("システム休止...."));
      pw = "OFF";
      delay(1000);
      display.clear();
      delay(1000);
      display.setBrightness(1);
      display.setSegments(SEG_SLEEP);
      stay = "OFF";
      digitalWrite(coordination_led, LOW);
      digitalWrite(speaker_sw, LOW);


    }

  }
}

void interval() { //10分間のあれの処理
  Serial.println(F("10分間モードスイッチ信号を検知しました"));
  coordination_off();
  myDFPlayer.play(5);
  Serial.println(F("== 再生開始(休憩開始)"));
  int var = 1, Time = 601, Time_min, Time_sec;
  while (var < 602) {
    Time = Time - 1;
    Time_min = ((Time % 3600) / 60) * 100;
    Time_sec = Time % 60;
    display.showNumberDecEx(Time_min + Time_sec, 0b01000000, false);
    delay(1000);
    var++;
  }
  myDFPlayer.play(5);
  Serial.println(F("== 再生開始(休憩終了・授業開始)"));
  delay(musictime);
  Serial.println(F("== 再生終了"));
  display.clear();
  Serial.println();
  coordination_on();
  stay = "OFF";
}
void normal() { //通常チャイム
  Serial.println(F("通常チャイム信号を検知しました"));
  display.setSegments(SEG_PLAY);
  myDFPlayer.play(5);
  Serial.println(F("== 再生開始"));
  delay(musictime);
  Serial.println(F("== 再生終了"));
  Serial.println();
  display.clear();
  stay = "OFF";
}
void coordination_on() { //連動ON
  digitalWrite(coordination_led, HIGH);
  coordination_state = "ON";
  Serial.println(F("相互連動を「ON」にしました"));
  Serial.println();
}
void coordination_off() { //連動OFF
  digitalWrite(coordination_led, LOW);
  coordination_state = "OFF";
  Serial.println(F("相互連動を「OFF」にしました"));
  Serial.println();
}
void restart() {//再スタート
  display.clear();
  Serial.println(F("据え置き型相互連動システム「チャイムル」"));
  Serial.println();
  delay(1000);
  Serial.println(F("DFPlayer 再構築しました。"));
  myDFPlayer.volume(vol);  //Set volume value. From 0 to 30
  myDFPlayer.play(4);  //Play the first mp3
  display.setBrightness(7);
  display.setSegments(SEG_STANDBY);
  digitalWrite(coordination_led, HIGH);
  digitalWrite(interval_led, HIGH);
  digitalWrite(now_led, HIGH);
  delay(3000);
  digitalWrite(interval_led, LOW);
  digitalWrite(coordination_led, LOW);
  digitalWrite(now_led, LOW);
  delay(1000);
  display.setSegments(SEG_ON);
  coordination_state = "ON";
  digitalWrite(coordination_led, HIGH);
  delay(3000);
  display.clear();
  Serial.println(F("＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝"));
  pw = "ON";
}

void printDetail(uint8_t type, int value) {//DFPlayerのシステム状況
  switch (type) {
    case DFPlayerCardInserted:
      Serial.println(F("SDカードが挿入されました"));
      myDFPlayer.play(4);
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("SDカードが取り外されました"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    default:
      break;
  }
}

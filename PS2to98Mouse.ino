/*
 *  PS/2マウス -> PC-98バスマウス
 *  for Arduino Nano　2023/10/22
 *  
 * References And Thanks
 *  https://github.com/tyama501/ps2busmouse98
 */

//PS/2デバイス用ライブラリ https://playground.arduino.cc/ComponentLib/Ps2mouse/
#include "ps2.h"

//PS/2側のピンアサイン
#define PS2DATA A4
#define PS2CLK A5

//バスマウス側のピンアサイン
#define XA PD2
#define XB PD3
#define YA PD4
#define YB PD5
#define LB PD6
#define RB PD7

//マウス認識時LEDピン 点灯と移動時の点滅
#define LED 8

//最大移動量
#define MAX_MOVE 20

//移動量の調整 100標準値(100%)
#define ACCEL 100

PS2 mos(PS2CLK, PS2DATA);
bool LED_F = LOW;
int LED_C = 0;

void mos_init()
{
    //電源投入直後だとマウスが反応しない場合有2000ms程度ディレイ
    delay(2000);

    int nMouseID = 0;
    
    //PS2マウス初期化
    mos.write(0xff); // reset
    mos.read(); // ack byte
    mos.read(); // blank
    nMouseID = mos.read();
    Serial.print("nMouseID=");
    Serial.println(nMouseID);
    
    //リモートモードに設定
    mos.write(0xf0);
    mos.read();  // ack
    delayMicroseconds(100);
}
void setup()
{
    Serial.begin(9600);

    pinMode(LED, OUTPUT);
    digitalWrite(LED,LED_F); 

    pinMode(XA, INPUT);
    pinMode(XB, INPUT);
    pinMode(YA, INPUT);
    pinMode(YB, INPUT);
    pinMode(LB, INPUT);
    pinMode(RB, INPUT);
    digitalWrite(XA, LOW);
    digitalWrite(XB, LOW);
    digitalWrite(YA, LOW);
    digitalWrite(YB, LOW);
    digitalWrite(LB, LOW);
    digitalWrite(RB, LOW);
    
    while (!Serial);
    Serial.println("Start");
    mos_init();
    Serial.println("OK");
    LED_F = HIGH;
    digitalWrite(LED,LED_F); 

}
void loop()
{
    static int stateX = 0;
    static int stateY = 0;
    static int stateB = 0;
    static int dataX = 0;
    static int dataY = 0;
  
    int mstat = 0;
    int mx = 0;
    int my = 0;
    
    mos.write(0xeb);
    mos.read();
/*
PS/2マウスの標準の場合はリセット信号を送信後にデータ取得信号「0xeb」を送れば以下の順番で情報を受信
 Byte 1
  Bit 7 Y overflow
  Bit 6 X overflow
  Bit 5 Y sign bit
  Bit 4 X sign bit
  Bit 3 Always 1
  Bit 2 Middle Button
  Bit 1 Right Button
  Bit 0   Left Button
 Byte 2  Bit 7-0 X Movement
 Byte 3  Bit 7-0 Y Movement
 */
    
    //Byte 1のステータス情報の処理
    mstat = mos.read();
    if (mstat & 0x01) {
      stateB = stateB | 0x01; // L
    }
    else {
      stateB = stateB & 0xFE;
    }
    if (mstat & 0x02) {
      stateB = stateB | 0x02; // R
    }
    else {
      stateB = stateB & 0xFD;
    }
    if (mstat & 0x10) {
      stateX = stateX | 0x10; // Negative X
    }
    else {
      stateX = stateX & 0xEF; // Positive X
    }
    if (mstat & 0x20) {
      stateY = stateY | 0x20; // Negative Y
    }
    else {
      stateY = stateY & 0xDF; // Positive Y
    }
    

    //Byte 2の X移動量処理
    mx = mos.read();  //Byte 2  Bit 7-0 X Movement
    if ((stateX & 0x10) && mx) {
      dataX = 256 - mx;
    }
    else {
      dataX = mx;
    }    
    dataX = (dataX * ACCEL) / 100;

    
    //Byte 3の Y移動量処理
    my = mos.read();  //Byte 3  Bit 7-0 Y Movement
    if ((stateY & 0x20) && my) {
      dataY = 256 - my;
    }
    else {
      dataY = my;
    }
    dataY = (dataY * ACCEL) / 100;


/*
    Serial.print(mstat, BIN);
    Serial.print("\tX=");
    Serial.print(mx, DEC);
    Serial.print("\tY=");
    Serial.print(my, DEC);
    Serial.println();
    Serial.print(dataX);
    Serial.print(":");
    Serial.println(dataY);  
*/
    //LED制御
    if (dataX > 0 || dataY > 0) {
      if (LED_C > 3){
        LED_F = !LED_F;
        digitalWrite(LED,LED_F);
        LED_C = 0;
      }else{
        LED_C++;
      }
    }else{
      LED_F = HIGH;
      digitalWrite(LED,LED_F);   
    }
    

// Limiter
    if (dataX > MAX_MOVE) {
      dataX = MAX_MOVE;
    }

    if (dataY > MAX_MOVE) {
      dataY = MAX_MOVE;
    }

// Button
    switch (stateB) {
      case 0x00:
        pinMode(LB, INPUT);
        pinMode(RB, INPUT);
        break;
      case 0x01:
        pinMode(LB, OUTPUT);
        pinMode(RB, INPUT);
        break;
      case 0x02:
        pinMode(LB, INPUT);
        pinMode(RB, OUTPUT);
        break;
      case 0x03:
        pinMode(LB, OUTPUT);
        pinMode(RB, OUTPUT);
        break;
    }

// Cursor
// state      0 1 3 2
//          ___     ___
// A pulse |   |___|   |___
//            ___     ___
// B pulse   |   |___|   |___
//
// declease <--        --> increase
//
// For XA,XB the increasing pulse move the cursor rightward. (Positive for PS/2)
// For YA,YB the increasing pulse move the cursor downward. (Negative for PS/2)

    while (dataX) {
      switch (stateX) {
        case 0x00:
          stateX = 0x01;
          pinMode(XA, OUTPUT);
          pinMode(XB, INPUT);
          break;
        case 0x01:
          stateX = 0x03;
          pinMode(XA, OUTPUT);
          pinMode(XB, OUTPUT);
          break;
        case 0x03:
          stateX = 0x02;
          pinMode(XA, INPUT);
          pinMode(XB, OUTPUT);
          break;
        case 0x02:
          stateX = 0x00;
          pinMode(XA, INPUT);
          pinMode(XB, INPUT);
          break;
        case 0x10:
          stateX = 0x12;
          pinMode(XA, INPUT);
          pinMode(XB, OUTPUT);
          break;
        case 0x12:
          stateX = 0x13;
          pinMode(XA, OUTPUT);
          pinMode(XB, OUTPUT);
          break;
        case 0x13:
          stateX = 0x11;
          pinMode(XA, OUTPUT);
          pinMode(XB, INPUT);
          break;
        case 0x11:
          stateX = 0x10;
          pinMode(XA, INPUT);
          pinMode(XB, INPUT);
          break;
        default:
          stateX = 0x00;
      }
      dataX--;
      delayMicroseconds(150);
      //delayMicroseconds(3);
    }

    while (dataY) {
      switch (stateY) {
        case 0x20:
          stateY = 0x21;
          pinMode(YA, OUTPUT);
          pinMode(YB, INPUT);
          break;
        case 0x21:
          stateY = 0x23;
          pinMode(YA, OUTPUT);
          pinMode(YB, OUTPUT);
          break;
        case 0x23:
          stateY = 0x22;
          pinMode(YA, INPUT);
          pinMode(YB, OUTPUT);
          break;
        case 0x22:
          stateY = 0x20;
          pinMode(YA, INPUT);
          pinMode(YB, INPUT);
          break;
        case 0x00:
          stateY = 0x02;
          pinMode(YA, INPUT);
          pinMode(YB, OUTPUT);
          break;
        case 0x02:
          stateY = 0x03;
          pinMode(YA, OUTPUT);
          pinMode(YB, OUTPUT);
          break;
        case 0x03:
          stateY = 0x01;
          pinMode(YA, OUTPUT);
          pinMode(YB, INPUT);
          break;
        case 0x01:
          stateY = 0x00;
          pinMode(YA, INPUT);
          pinMode(YB, INPUT);
          break;
        default:
          stateY = 0x00;
      }
      dataY--;
      delayMicroseconds(150);
      //delayMicroseconds(3);
    }
  delayMicroseconds(5);    
    
    //delay(100);
}

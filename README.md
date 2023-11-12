# PS2to98Mouse
 PS/2キーボードをArduinoNanoを用いて、98用バスマウスに変換<br><br>
 tyama501さんが公開されているコードでは現在販売されている<br>
 [エレコム マウス 有線 Mサイズ 3ボタン PS2 光学式 ホワイト ROHS指令準拠 M-K6P2RWH/RS ](https://amzn.to/3swwrgN)<br>
 から、うまくデータを受信できなかったのでPS/2データ処理部を汎用のライブラリを用いて修正。<br>
 ほとんどのPS2マウスに対応したような感じです<br>
 また、開発ボードをArduinoNanoで行いました

## 必要なライブラリ
[PS/2デバイス用ライブラリ](https://playground.arduino.cc/ComponentLib/Ps2mouse/)

## ArduinoNanoとの接続
//98側バスマウス側のピンアサイン<br>
#define XA PD2<br>
#define XB PD3<br>
#define YA PD4<br>
#define YB PD5<br>
#define LB PD6<br>
#define RB PD7<br>
![md9_pc98m](/img/md9_pc98m.png)
![ds9_pc98m](/img/ds9_pc98m.png)

//PS/2側のピンアサイン<br>
#define PS2DATA A4<br>
#define PS2CLK A5<br>
![md6_ps2](/img/md6_ps2.png)

//マウス認識時LEDピン 点灯と移動時の点滅<br>
#define LED 8<br>

※LEDは適当な抵抗かませてください。不要なら接続しなくてもOK<br>
※5VとGNDの接続も忘れずに!<br>
※画像はyagura様の[コネクタ資料室](http://nkmm.org/yagura/lib/)より引用させていただきました


## 紹介と仕様解説ブログ
[[PC-98][Arduino] PS/2マウスをArduinoでPC-98につなげる](https://androiphone.uvs.jp/?p=4012)<br>

## 参考サイト・謝辞
このコードを作るにあたり、とても参考にさせていただきました。この場を借りてお礼申し上げます。<br>
[https://github.com/tyama501/ps2busmouse98](https://github.com/tyama501/ps2busmouse98)


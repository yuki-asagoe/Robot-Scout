# スカウト モータードライバー

## ビルド
*Arduino IDE*経由でビルドします。
六甲おろしモータードライバーにISPで書き込んでください。

ボードごとに変更する必要のある値は`variables.h`に切り出しています。

#### 依存関係
[六甲おろしモータードライバー v1.x 2016](https://github.com/RokkoOroshi/CanMotorBoard2016)

## 利用法
CAN通信を用いて制御します。

送信先アドレスにつきましては`variables.h`定義のIDを用いて構築されます。  
申し訳ないけど詳細[前述のリポジトリ](https://github.com/RokkoOroshi/CanMotorBoard2016)の `can_communication` Libを参照してね

### CANデータ構造
CANメッセージは8byteあります。以下その内訳です。1番モーターを`m1`、2番モーターを`m2`と書きます

支援物資モータについて停止位置は `0~1023`の値を2バイトで送信します。
`INPUT2_1`で得られる値が、正転時は停止位置の値を超えたら、逆転時は下回ればモータを停止します。

|1|2|3|4|5|
|:-:|:-:|:-:|:-:|:-:|
|`m1`回転法(0:停止,+:正転,-:逆転,255:Free)|`m1`出力強度(unsigned)|`m2`(支援物資)回転法(0:現状維持,+:正転,-:逆転)|停止位置上位バイト|停止位置下位バイト|

### リミットスイッチ
入力にリミットスイッチを受け入れることができます。`variables.h`で`Limit_Switch_Type`を編集してください。
`INPUT1_1`は正転を、`INPUT1_2`は逆転を止めます。
#  Chp-p01

技術検証用自立思考型会話ロボット　プロトタイプCHAPPYの最小構成のコード群です。

文書生成以外はすべてローカル環境で完結するのが利点です。(将来的にはTinySwallow-1.5Bをラズパイ5Bに搭載して完全ローカル化したい...)

### C.H.A.P.P.Y.:

	•	Conversational Human-like Assistant for Personalized Phrase-based Yielding

### 以下の機器を使用します

・M5Core2(シリアル通信内容の表示と録音データのUDP送信)

・M5AtomS3(シリアル通信内容のBLE送信)

・RaspberryPi Zero2(BLE通信内容をシリアルで送信)

・WinPC(UDPにより取得した録音のwhisperによるsttとその結果のgeminiレスポンスをシリアルで送信)

## コードについて

基本的にファイル名と同じ機械に対し、同名ファイル内のプログラムを一つだけ選んで実行可能にすればおK

例：M5Core2 -> PrototypeChappy/M5Core2/M5REC-USB.ino を実行可能な状態で書き込む

## 注意

・M5Core2のコードは基本的に、ボタンAを押したときに録音されますが、最初のみうまくいきませんし、その時に空白の音声がUDP通信に送信されます。

・M5Core2とWinPCのみ同一ネットワーク上の同一ルーターに接続され、M5core2のプログラムのローカルIPアドレス部をWinPCのものに変更する必要があります。

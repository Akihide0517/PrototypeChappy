#include <M5Unified.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Avatar.h>

m5avatar::Avatar avatar;

// WiFi設定
const char *ssid = "Y.A_Mac_mini";
const char *password = "damakatu0517";
const char *udp_address = "172.20.10.3";  // 送信先IP
const int udp_port = 5000;                  // 送信先ポート

// WAVフォーマット設定
static constexpr size_t SAMPLE_RATE = 16000;
static constexpr size_t RECORD_SECONDS = 8;
static constexpr size_t RECORD_SAMPLES = SAMPLE_RATE * RECORD_SECONDS;
static constexpr size_t RECORD_BYTES = RECORD_SAMPLES * sizeof(int16_t);

bool mode = true;

// バッファ確保
static int16_t *audio_buffer;
WiFiUDP udp;

void setup() {
    M5.begin();
    Serial.begin(115200);  // シリアル通信を開始
    while (!Serial) {  // シリアル通信が接続されるまで待機
        delay(100);
        M5.Display.print("*");
    }

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Display.print(".");
    }
    M5.Display.println("\nWiFi Connected!");
    udp.begin(udp_port);
    
    // 録音バッファ確保
    audio_buffer = (int16_t *)heap_caps_malloc(RECORD_BYTES, MALLOC_CAP_8BIT);
    if (!audio_buffer) {
        M5.Display.println("Memory allocation failed!");
        while (1);
    }

  avatar.init();
  avatar.setSpeechFont(&fonts::lgfxJapanGothic_12);
}

void loop() {
    M5.update();
    if (M5.BtnA.wasPressed()) {
        avatar.setSpeechText("Recording...");
        Serial.println("recording");
        M5.Mic.begin();
        M5.Mic.record(audio_buffer, RECORD_SAMPLES, SAMPLE_RATE);
        M5.Mic.end();
        //M5.Display.println("Recording Complete!");

        //if(mode){
          sendWAV();
        //}

        // シリアル通信で改行が来るまで待機
        //M5.Display.println("Waiting for serial input...");
        avatar.setSpeechText("考え中");
        Serial.println("Waiting for serial input...");
        while (true) {
            if (Serial.available()) {
                String input = Serial.readStringUntil('\n');  // 改行までの文字列を読み取る
                input.trim();  // 不要な空白や改行を削除
                if (input.length() > 0) {
                    //M5.Display.println("Received: " + input);
                    Serial.println("Received: " + input);
                    //avatar.setSpeechText(input.c_str());  // ← 修正
                    printInChunks(input.c_str(), 18);
                    break;  // ループを抜ける
                }
            }else if(mode){
              mode = false;
              break;
            }
            delay(10);
        }

        //M5.Display.println("Ready for next recording");
        Serial.println("Ready for next recording");
    }
}

void sendWAV() {
    avatar.setSpeechText("Sending WAV...");

    // WAVヘッダー作成
    uint8_t wav_header[44] = {
        'R', 'I', 'F', 'F',
        (uint8_t)(RECORD_BYTES + 36), (uint8_t)((RECORD_BYTES + 36) >> 8), (uint8_t)((RECORD_BYTES + 36) >> 16), (uint8_t)((RECORD_BYTES + 36) >> 24),
        'W', 'A', 'V', 'E',
        'f', 'm', 't', ' ',
        16, 0, 0, 0,    // fmt chunk size
        1, 0,          // Audio format (PCM)
        1, 0,          // Channels (Mono)
        (uint8_t)(SAMPLE_RATE), (uint8_t)(SAMPLE_RATE >> 8), (uint8_t)(SAMPLE_RATE >> 16), (uint8_t)(SAMPLE_RATE >> 24),
        (uint8_t)(SAMPLE_RATE * 2), (uint8_t)((SAMPLE_RATE * 2) >> 8), (uint8_t)((SAMPLE_RATE * 2) >> 16), (uint8_t)((SAMPLE_RATE * 2) >> 24),
        2, 0,          // Block Align
        16, 0,         // Bits per sample
        'd', 'a', 't', 'a',
        (uint8_t)(RECORD_BYTES), (uint8_t)(RECORD_BYTES >> 8), (uint8_t)(RECORD_BYTES >> 16), (uint8_t)(RECORD_BYTES >> 24)
    };

    udp.beginPacket(udp_address, udp_port);
    udp.write(wav_header, 44);
    udp.endPacket();

    // 512バイトごとに分割送信
    size_t chunk_size = 512;
    for (size_t i = 0; i < RECORD_BYTES; i += chunk_size) {
        size_t send_size = min(chunk_size, RECORD_BYTES - i);
        udp.beginPacket(udp_address, udp_port);
        udp.write((uint8_t *)&audio_buffer[i / sizeof(int16_t)], send_size);
        udp.endPacket();
    }

    avatar.setSpeechText("WAV Sent!");
    Serial.println("WAV Sent!");
}

void printInChunks(const char* text, size_t chunkSize) {
  size_t textLength = strlen(text);
  String displayText = "";  // 表示する文字列（最大10文字）

  // 文字列の表示を始める
  for (size_t i = 0; i < textLength; ++i) {
    // 新しい文字を追加
    displayText += text[i];

    // 文字列が10文字を超えた場合、先頭の文字を削除（FIFO）
    if (displayText.length() > chunkSize) {
      displayText.remove(0, 1);  // 先頭の1文字を削除
    }

    // 現在の表示文字列を設定
    avatar.setSpeechText(displayText.c_str());

    // 次の文字を表示するまで遅延（例えば、300ms）
    delay(150);
  }
}

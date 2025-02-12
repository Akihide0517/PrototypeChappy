#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <M5Unified.h>
#include <Avatar.h>

#define SERVICE_UUID           "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID    "12345678-1234-5678-1234-56789abcdef1"

m5avatar::Avatar avatar;

// グローバル変数
BLECharacteristic *pCharacteristic;
BLEServer *pServer;
BLEService *pService;

// BLE接続のコールバッククラス
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    //M5.Display.fillScreen(BLACK);  // 画面を黒にクリア
    //M5.Display.setCursor(0, 0);
    avatar.setSpeechText("Client Connected");
    Serial.println("Client Connected");
  }

  void onDisconnect(BLEServer* pServer) {
    //M5.Display.fillScreen(BLACK);  // 画面を黒にクリア
    //M5.Display.setCursor(0, 0);
    avatar.setSpeechText("Client Disconnected");
    Serial.println("Client Disconnected");
  }
};

void setup() {
  M5.begin();
  Serial.begin(115200);
  avatar.setScale(0.4);
  avatar.setPosition(-56, -96);
  avatar.init();

  // BLEデバイス初期化
  BLEDevice::init("ESP32_BLE_Peripheral");

  // サーバー作成
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());  // コールバック設定

  // サービス作成
  pService = pServer->createService(SERVICE_UUID);

  // キャラクタリスティック作成（読み書き通知対応）
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());  // 通知用のデスクリプタ追加

  // サービス開始
  pService->start();

  // BLEアドバタイズ開始
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  Serial.println("BLE Peripheral Started!");
  //M5.Display.fillScreen(BLACK);  // 初期化時は画面をクリア
  //M5.Display.setCursor(0, 0);
  avatar.setSpeechText("BLE Peripheral Started!");
}

void loop() {
  if (Serial.available()) {
    String message = Serial.readString();  // シリアルからメッセージを受信
    pCharacteristic->setValue(message.c_str());  // BLEキャラクタリスティックに設定
    pCharacteristic->notify();  // 通知
    Serial.println("Sent to BLE: " + message);
    String messageText = "Sent to BLE: " + message;
    avatar.setSpeechText(messageText.c_str());  // M5Stackに表示
  }
  delay(100);
}

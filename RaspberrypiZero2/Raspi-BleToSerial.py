import asyncio
from bleak import BleakScanner, BleakClient
import serial
import time

SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef0"
CHARACTERISTIC_UUID = "12345678-1234-5678-1234-56789abcdef1"
SERIAL_PORT = "/dev/ttyACM0"  # シリアルポートのパス

async def scan_and_connect():
    # シリアルポートの初期化
    ser = serial.Serial(SERIAL_PORT, 115200, timeout=1)
    print("シリアル通信を開始しました")

    while True:
        print("BLEデバイスをスキャン中...")
        devices = await BleakScanner.discover()

        target_device = None
        for device in devices:
            if device.name == "ESP32_BLE_Peripheral":
                target_device = device
                break

        if not target_device:
            print("ターゲットデバイスが見つかりませんでした。再度スキャンを開始します。")
            await asyncio.sleep(5)  # 5秒後に再スキャン
            continue  # デバイスが見つからない場合は再スキャン

        async with BleakClient(target_device.address) as client:
            print(f"接続しました: {target_device.address}")

            async def notification_handler(sender, data):
                # 受信データをシリアルポートに送信
                print(f"受信データ: {data.decode()}")
                ser.write(data)  # シリアル通信にデータ送信

            await client.start_notify(CHARACTERISTIC_UUID, notification_handler)
            print("通知を受信中...")

            try:
                while True:
                    await asyncio.sleep(1)  # BLE通信を継続的に受信
            except asyncio.CancelledError:
                print("通知停止")
                await client.stop_notify(CHARACTERISTIC_UUID)

            # BLEデバイスとの接続終了後、再度スキャンを開始
            print("BLE接続が切れました。再度スキャンを開始します。")

asyncio.run(scan_and_connect())

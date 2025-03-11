#UDPで受け取った一回分のsttと文書生成を一回だけシリアル通信でAtomS3に送信するコードです

import os
import socket
import wave
import time
import serial
import pyaudio
import whisper
from dotenv import load_dotenv
import google.generativeai as genai

# 環境変数の読み込み
load_dotenv()
genai.configure(api_key="key")

# UDP サーバーの設定
UDP_IP = "0.0.0.0"
UDP_PORT = 5000
BUFFER_SIZE = 4096
TIMEOUT = 1.0
WAV_FILE = "received_audio.wav"

# シリアル通信の設定（COM3に9600bpsで接続）
SERIAL_PORT = "COM7"
BAUD_RATE = 115200

# UDPソケットを作成
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening on port {UDP_PORT}...")

# WAVヘッダーを受信
data, addr = sock.recvfrom(8192)
wav_header = data

# 音声データの受信
audio_data = bytearray()
last_received_time = time.time()

while True:
    sock.settimeout(TIMEOUT)
    try:
        data, addr = sock.recvfrom(BUFFER_SIZE)
        if not data:
            break
        audio_data.extend(data)
        print(f"Received {len(data)} bytes")
        last_received_time = time.time()
    except socket.timeout:
        print("Timeout: No data received for 1 second. Moving to the next step.")
        break

# WAVファイルとして保存
with open(WAV_FILE, "wb") as f:
    f.write(wav_header)
    f.write(audio_data)

print(f"Saved as {WAV_FILE}")

# 音声の再生
def play_audio(filename):
    wf = wave.open(filename, "rb")
    p = pyaudio.PyAudio()
    stream = p.open(format=p.get_format_from_width(wf.getsampwidth()),
                    channels=wf.getnchannels(),
                    rate=wf.getframerate(),
                    output=True)

    chunk = 1024
    data = wf.readframes(chunk)
    while data:
        stream.write(data)
        data = wf.readframes(chunk)

    stream.stop_stream()
    stream.close()
    p.terminate()

#print("Playing audio...")
#play_audio(WAV_FILE)
#print("Playback finished.")

# STT（Whisperによる音声認識）
def transcribe_audio(filename):
    model = whisper.load_model("small")
    result = model.transcribe(filename)
    return result["text"]

print("Transcribing audio...")
transcription = transcribe_audio(WAV_FILE)
print("Transcription Result:")
print(transcription)

# Gemini APIを使って文書生成
def generate_text(prompt):
    gemini_pro = genai.GenerativeModel("gemini-2.0-flash")
    message = "あなたはチャッピーという、語尾が”だよ”のようなものを持つかわいい生物として、短めの改行を含まない文章で次の会話の返答をしてください。" + prompt;
    response = gemini_pro.generate_content(message)
    return response.text if hasattr(response, "text") else "Error in response"

print("Generating text with Gemini...")
generated_text = generate_text(transcription)
print("Generated Text:")
print(generated_text)

# シリアル通信で送信
ser = serial.Serial(SERIAL_PORT,115200,timeout=None)
data = generated_text.replace("\x00", "")  # NULL文字を削除
ser.write(data.encode("utf-8"))
time.sleep(3.0)#すぐにクローズするとesp32が即時再起動してしまうのでdelayする
ser.close()
print(f"Sent over Serial: {generated_text}")

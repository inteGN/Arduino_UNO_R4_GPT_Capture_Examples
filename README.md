# Arduino_UNO_R4_GPT_Capture_Examples

## 目的 / Purpose
このリポジトリは、Arduino UNO R4 (RA4M1)のGPTタイマーによるインプットキャプチャ機能の利用方法を示しています。  
This repository demonstrates how to use the GPT timer's input-capture functionality on the Arduino UNO R4 (RA4M1).  

---

## 使い方 / Usage
**これらの例はArduino UNO R4 MINIMAを対象としています。**  
UNO R4 WiFiでは、ArduinoピンとRA4M1ポートとの割り当てが異なるため、同じコードでも異なる結果になる可能性があります。  
**These examples target the Arduino UNO R4 MINIMA.**  
On the UNO R4 WiFi, the RA4M1 port assignment for Arduino pins differs, which may lead to different behavior even when running the same code  

1. **input-capture example** `gpt_capture_example.ino`  
   - Arduino UNO R4 MINIMAでGPT2タイマーを使用して入力パルスをキャプチャする方法を示します。  
   - D4/GPT2_Aピンの各立ち上がりエッジにはタイムスタンプが付けられ、キャプチャされた値はシリアルモニターを通じて表示されます。  
   - 検証のための簡単な入力ソースを提供するため、PWMテスト信号がD6ピンに生成されます。  
   - It demonstrates how to capture input pulses on the Arduino UNO R4 MINIMA using the timer GPT2.  
   - Each rising edge on the pin D4/GPT2_A is timestamped, and the captured value is shown through the serial monitor.  
   - A test signal is generated on pin D6 to provide a simple input source for verification.  
<br>

---

## ピン接続 / Pin connection
- 適切なパルス信号をD4ピンに入力する、またはD4ピンとD6ピンを接続する（テスト用）
- Input the appropriate pulse signal to pin D4, or connect pin D4 and D6 (if test)

---

## ポイント / Key insights
- RA4M1のGPTタイマーは、非常に低いジッタのハードウェアレベルのタイムスタンプを提供し、パルス間隔測定に最適です。
- インプットキャプチャとオーバーフローの割り込みを組み合わせ、ソフトウェアで16ビットのGPTカウンタを32ビットに拡張できます。
- これらの例は、高いパルス周波数でもCPU負荷を低く保ちつつ、パルスタイミングを正確に測定する方法を示しています。
- テスト信号の出力を持たせ、外部のセンサーなしでキャプチャーのロジックの検証を容易にしています。
- The RA4M1 timer GPT provides hardware‑level timestamping with very low jitter, ideal for pulse‑interval measurement.
- Input-capture and overflow interrupts can be combined to extend the 16‑bit GPT counter to 32 bits in software.
- These examples show how to measure pulse timing accurately while keeping the CPU load low, even at high pulse frequencies.
- A built‑in test signal makes it easy to verify the capture logic without external sensor.

---

## 必要な環境 / Requirements
- Arduino IDE（最新版推奨） / Arduino IDE (latest recommended)  
- Arduino UNO R4 MINIMA / Arduino UNO R4 MINIMA  

---

## License
Copyright (c) 2026 inteGN - MIT License  


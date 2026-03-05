//************************************************
//  FILE        :speed_measurement.ino
//  DATE        :2026/02/28
//  DESCRIPTION :wheel speed measurement by timer GPT capture
//  BOARD TYPE  :UNO R4 MINIMA
//  AUTHER      :inteGN
//************************************************
/*
このプログラムは、Arduino UNO R4 (RA4M1)でGPT2タイマーを使用してホイール回転速度を測定します。
D4/GPT2_Aピンの立ち上がりエッジ毎にタイムスタンプが付けられ、連続するエッジ間の間隔がホイール速度に変換されます。
GPT2の16ビット制限を越えるために、32ビット拡張カウンタがソフトウェア的に実装されています。
検証のために、ホイールパルスをシミュレートするテスト信号がD6ピンから生成されます。

This program measures wheel rotational speed on the Arduino UNO R4 (RA4M1) using the GPT2 timer.
Each rising edge on the pin D4/GPT2_A is timestamped, and the interval between consecutive edges is converted into wheel speed.
A 32‑bit extended counter is implemented in software to overcome the 16‑bit limitation of the GPT2 hardware.
A PWM test signal is generated on D6 to simulate wheel pulses for validation.
*/

//// Pin connection
//  - D4ピンとD6ピンを接続する（テスト用）
//  - Connect pin D4 and D6 (if test)


//// Includes
#include <Arduino.h>
#include "FspTimer.h"
#include "pwm.h"

//// Definitions
#define   diaWheel      700.0f                //700 mm: assumed wheel diameter
#define   minPulse      60000                 //20 msec: minimum valid pulse interval, 0.333 microsecond/LSB
#define   timeOver      7500000               //2.5 sec: timeout threshold to indicate wheel stop, 0.333 microsecond/LSB

//// Grobals
FspTimer  timer2;
PwmOut    pwmD6(D6);                          //Arduino pin D6 / RA4M1 P106 GPT0_B for test signal output
volatile  uint32_t capt_width = 0xFFFFFFFF;   //captured pulse width, set 0xFFFFFFFF as specific marker of timeout
volatile  bool     capt_flag  = false;        //flag indicating new capture data is available
uint32_t  millisSignal;
uint8_t   numSignal = 0;
float     freqSignal[14] = {0.2f, 0.5f, 1.0f, 2.0f, 5.0f, 10.0f, 20.0f, 20.0f, 10.0f, 5.0f, 2.0f, 1.0f, 0.5f, 0.2f};

//// ISR tasks to capture pluse timing and count of timer roll over
void onCallback(timer_callback_args_t *args) {
  static uint32_t extCount   = 0;             //extended counter for 32-bit emulation
  static uint32_t extCountup = 0;             //additional increment when overflow and capture occur simultaneously
  static uint32_t captValue_new;
  static uint32_t captValue_old;
  static uint32_t timeCheck = 0;              //timeover base point of previous pulse signal or timeout
  static bool capt_valid = false;
  if (args->event == TIMER_EVENT_CAPTURE_A) {
    if (R_GPT2->GTST_b.TCFPO == 1) {      //**This part handles the edge case that capture occurs just after overflow.
      if (args->capture < 0x8000) {
        extCountup = 0x00010000;              //overflow occurred just before capture
      }
      else {
        extCountup = 0;
      }
    }
    if ((uint32_t)args->capture + extCount + extCountup - captValue_new > minPulse) {   //reject pulses shorter than the minimum valid
      captValue_old = captValue_new;
      captValue_new = (uint32_t)args->capture + extCount + extCountup;
      if (capt_valid == true) {
        capt_width = captValue_new - captValue_old;
        capt_flag  = true;
      }
      timeCheck = captValue_new;              //set next timeover base point
      capt_valid = true;
    }
  }
  else if (args->event == TIMER_EVENT_CYCLE_END) {
    R_GPT2->GTST_b.TCFPO = 0;
    extCount += 0x00010000;                   //extend counter by 16-bit overflow
    if (extCount - timeCheck > timeOver) {
      capt_width = 0xFFFFFFFF;                //set spesific value of timeout
      capt_flag  = true;
      timeCheck = extCount;                   //set next timeover base point
      capt_valid = false;
    }
  }
}

//// Setup function
void setup() {
//Setup general port
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  delay(2000);
//timer configurate: use Arduino pin D4 / RA4M1 P103 GPT2_A as capture
  noInterrupts();
  bool rv = timer2.begin(TIMER_MODE_PWM, GPT_TIMER, 2, 0, 0, TIMER_SOURCE_DIV_16, onCallback);
  if (rv) {
    auto cfg = timer2.get_cfg();
    auto ext = (gpt_extended_cfg_t*)cfg->p_extend;
    ext->capture_filter_gtioca = GPT_CAPTURE_FILTER_PCLKD_DIV_4;     //(4 x 3 + 1) clocks of delay to sample
    timer2.set_source_capture_a((gpt_source_t)(GPT_SOURCE_GTIOCA_RISING_WHILE_GTIOCB_LOW | GPT_SOURCE_GTIOCA_RISING_WHILE_GTIOCB_HIGH));
    //**This uses rising-edge to capture. Use `_FALLING_` instead if the sensor switches to GND.
    timer2.setup_capture_a_irq(4);
    timer2.setup_overflow_irq(4);
    timer2.open();
    timer2.start();
    R_GPT2->GTST_b.TCFPO = 0;
    pinPeripheral(D4, (uint32_t)(IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_GPT1));     //GPT2_A input
  }
  interrupts();
  if (rv == false) {
    Serial.println("Initialization failed.");
    while (1) {}
  }
//test signal start
  pwmD6.begin((uint32_t)(1000000.0f / freqSignal[0] * 48.0f), (uint32_t)(2000 * 48), true, TIMER_SOURCE_DIV_1);
  millisSignal = millis();
//dump GPT registers
  Serial.print("R_GPT2->GTPR: "); Serial.println(R_GPT2->GTPR, HEX);
  Serial.print("R_GPT2->GTIOR: "); Serial.println(R_GPT2->GTIOR, HEX);
  Serial.print("R_GPT2->GTICASR: "); Serial.println(R_GPT2->GTICASR, HEX);
  Serial.println();
  
}

//// Loop function
void loop() {
//show captured value and velocity in km/h
  if (capt_flag) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print("capt_width: "); Serial.print((float)capt_width / 3000000.0f); Serial.print(" sec    ");
    Serial.print("capt_freq: "); Serial.print(3000000.0f / (float)capt_width); Serial.print(" Hz    ");
    Serial.print("velocity: "); Serial.print(3000000.0f / (float)capt_width * diaWheel / 1000.0f * 3.1416f * 3.6f); Serial.println(" km/h");
    digitalWrite(LED_BUILTIN, LOW);
    capt_flag = false;
  }
//test signal generator: frequency is changed every 10 sec
  if (millis() >= millisSignal + 10000) {
    millisSignal = millis();
    if (++numSignal > 13) {numSignal = 0;}
    pwmD6.period_raw((uint32_t)(1000000.0f / freqSignal[numSignal] * 48.0f));
  }
  delayMicroseconds(10);
}



//************************************************
//  FILE        :gpt_capture_example.ino
//  DATE        :2026/02/25
//  DESCRIPTION :GPT input capture example
//  BOARD TYPE  :UNO R4 MINIMA
//  AUTHER      :inteGN
//************************************************
/*
このプログラム例は、Arduino UNO R4 MINIMA (RA4M1)でGPT2タイマーを使用して入力パルスをキャプチャする方法を示します。
D4/GPT2_Aピンの各立ち上がりエッジにはタイムスタンプが付けられ、キャプチャされた値はシリアルモニターを通じて表示されます。
検証のための簡単な入力ソースを提供するため、PWMテスト信号がD6ピンに生成されます。

This example demonstrates how to capture input pulses on the Arduino UNO R4 MINIMA (RA4M1) using the timer GPT2.
Each rising edge on the pin D4/GPT2_A is timestamped, and the captured value is shown through the serial monitor.
A PWM test signal is generated on pin D6 to provide a simple input source for verification.
*/

//// Pin connection
//  - D4ピンとD6ピンを接続する
//  - Connect pin D4 and D6


//// Includes
#include <Arduino.h>
#include "FspTimer.h"
#include "pwm.h"

//// Grobals
FspTimer  timer2;
PwmOut    pwmD6(D6);                        //Arduino pin D6 / RA4M1 P106 GPT0_B
uint16_t  capt_period = 0;                  //equivarent to 0x010000
uint16_t  capt_count  = 0;
volatile uint32_t capt_value;
volatile uint32_t reg_value;
volatile bool     capt_flag;

//// ISR tasks every pulse captured
void onCallback(timer_callback_args_t *args) {
  if (args->event == TIMER_EVENT_CAPTURE_A) {
    capt_value = args->capture;
    reg_value  = R_GPT2->GTCCR[0];
    capt_flag  = true;
  }
}

//// Setup function
void setup() {
//Setup serial port
  Serial.begin(115200);
  delay(2000);

//timer configurate: use Arduino pin D4 / RA4M1 P103 GPT2_A as capture
  noInterrupts();
  bool rv = timer2.begin(TIMER_MODE_PWM, GPT_TIMER, 2, capt_period, capt_count, TIMER_SOURCE_DIV_1024, onCallback);
  if (rv) {
    auto cfg = timer2.get_cfg();
    auto ext = (gpt_extended_cfg_t*)cfg->p_extend;
    ext->capture_filter_gtioca = GPT_CAPTURE_FILTER_PCLKD_DIV_4;      //(4 x 3 + 1) clocks of delay to sample
    timer2.set_source_capture_a((gpt_source_t)(GPT_SOURCE_GTIOCA_RISING_WHILE_GTIOCB_LOW | GPT_SOURCE_GTIOCA_RISING_WHILE_GTIOCB_HIGH));
    timer2.setup_capture_a_irq();
    timer2.open();
    timer2.start();
    pinPeripheral(D4, (uint32_t)(IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_GPT1));     //GPT2_A capture input
  }
  interrupts();

//setup test signal output
  pwmD6.begin(0.2f, 50.0f);                                           //GPT0_B pwm output                                         //GPT0_B pwm output
}

//// Loop function
void loop() {
  if (capt_flag) {   
    Serial.print("capture value : "); Serial.println(capt_value);
    Serial.print("register value: "); Serial.println(reg_value);
    Serial.println();
    capt_flag = false;
  }
  delayMicroseconds(10);
}

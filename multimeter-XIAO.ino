/**************************************************************************
  モード切り替えはグローバル変数mode使う
  スイッチからの入力はピン割り込みでmode_change()で処理
  loop内でmodeのスイッチ文でモードごとに処理

 **************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <SPI.h>
#include <Mcp320x.h>

#define SPI_CS     7        // SPI slave select
#define ADC_VREF    1200     // 1.2V Vref
#define ADC_CLK     1000000  // SPI clock 1.0MHz
#define ADC_SCALING 38       // 分圧 内部抵抗含めたら38倍になった

MCP3208 adc(ADC_VREF, SPI_CS);

uint8_t mode = 0;

#include <SAMDTimerInterrupt.h>
SAMDTimer ITimer0(TIMER_TC3);
#define TIMER0_INTERVAL_MS        50      // 50ms*2 100ms=10Hz

void TimerHandler0(void)
{
  digitalWrite(6, !digitalRead(6));
  //digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void mode_change() {
  uint8_t tmp = mode;
  mode = 15;
  mode -= digitalRead(3) * 1;
  mode -= digitalRead(0) * 2;
  mode -= digitalRead(1) * 4;
  mode -= digitalRead(2) * 8;
  //Serial.println(mode);
  if (mode != tmp && mode >= 6) oscilloscope(1, true, 1);
}

void setup() {
  delay(100);
  Serial.begin(9600);

  pinMode(0, INPUT_PULLDOWN);
  pinMode(1, INPUT_PULLDOWN);
  pinMode(2, INPUT_PULLDOWN);
  pinMode(3, INPUT_PULLDOWN);
  attachInterrupt(0, mode_change, CHANGE);
  attachInterrupt(1, mode_change, CHANGE);
  attachInterrupt(2, mode_change, CHANGE);
  attachInterrupt(3, mode_change, CHANGE);

  pinMode(SPI_CS, OUTPUT);
  digitalWrite(SPI_CS, HIGH);
  SPISettings settings(ADC_CLK, MSBFIRST, SPI_MODE0);
  SPI.begin();
  SPI.beginTransaction(settings);

  pinMode(6, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  if (ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler0))
    Serial.println("Starting  ITimer0 OK, millis() = " + String(millis()));
  else
    Serial.println("Can't set ITimer0. Select another freq. or timer");

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  mode_change();
}


void loop() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  switch (mode) {
    case 0:
      display.setCursor(0, 0);
      display.println(F("8ch:12V"));
      drawVoltage(12000, 8);
      drawLine(12000, 5000 , 8);
      drawLine(12000, 12000 , 8);
      break;
    case 1:
      display.setCursor(0, 0);
      display.println(F("8ch:5V"));
      drawVoltage(5000, 8);
      drawLine(5000, 5000 , 8);
      drawLine(5000, 2750 , 8);
      drawLine(5000, 2000 , 8);
      drawLine(5000,  800 , 8);
      break;
    case 2:
      display.setCursor(0, 0);
      display.println(F("8ch:3.3V"));
      drawVoltage(3300, 8);
      drawLine(3300, 3300 , 8);
      drawLine(3300, 1650 , 8);
      drawLine(3300, 2000 , 8);
      drawLine(3300,  800 , 8);
      break;
    case 3:
      display.setCursor(0, 0);
      display.println(F("4ch:12V"));
      drawVoltage(12000, 4);
      drawLine(12000, 5000 , 4);
      drawLine(12000, 12000 , 4);
      break;
    case 4:
      display.setCursor(0, 0);
      display.println(F("4ch:5V"));
      drawVoltage(5000, 4);
      drawLine(5000, 5000 , 4);
      drawLine(5000, 2750 , 4);
      drawLine(5000, 2000 , 4);
      drawLine(5000,  800 , 4);
      break;
    case 5:
      display.setCursor(0, 0);
      display.println(F("4ch:3.3V"));
      drawVoltage(3300, 4);
      drawLine(3300, 3300 , 4);
      drawLine(3300, 1650 , 4);
      drawLine(3300, 2000 , 4);
      drawLine(3300,  800 , 4);
      break;
    case 6:
      oscilloscope(1, false, 1);
      break;
    case 7:
      oscilloscope(2, false, 1);
      break;
    case 8:
      oscilloscope(1, false, 2);
      break;
    case 9:
      oscilloscope(2, false, 2);
      break;
    default:
      break;
  }
  display.display();
}


void drawVoltage(uint16_t range, uint8_t channel) {
  uint16_t readval_mV[8];
  readval_mV[0] = adc.toAnalog(adc.read(MCP3208::Channel::SINGLE_0)) * ADC_SCALING;
  readval_mV[1] = adc.toAnalog(adc.read(MCP3208::Channel::SINGLE_1)) * ADC_SCALING;
  readval_mV[2] = adc.toAnalog(adc.read(MCP3208::Channel::SINGLE_2)) * ADC_SCALING;
  readval_mV[3] = adc.toAnalog(adc.read(MCP3208::Channel::SINGLE_3)) * ADC_SCALING;
  readval_mV[4] = adc.toAnalog(adc.read(MCP3208::Channel::SINGLE_4)) * ADC_SCALING;
  readval_mV[5] = adc.toAnalog(adc.read(MCP3208::Channel::SINGLE_5)) * ADC_SCALING;
  readval_mV[6] = adc.toAnalog(adc.read(MCP3208::Channel::SINGLE_6)) * ADC_SCALING;
  readval_mV[7] = adc.toAnalog(adc.read(MCP3208::Channel::SINGLE_7)) * ADC_SCALING;

  for (uint8_t i = 0; i < channel; i++) {
    uint8_t height = readval_mV[i] * 50 / range ;
    display.fillRect(32 + i * 12 + 1, 64 - height, 11, height, SSD1306_WHITE);
  }

  if (channel == 8) {
    display.setCursor(64, 0);
    display.print(F("CH1:")); display.print((float)readval_mV[0] / 1000.0, 1); display.println(F("V"));
  } else {
    for (uint8_t i = 0; i < channel; i++) {
      display.setCursor(85, 15 + i * 9);
      display.print(i + 1); display.print(F(":")); display.print((float)readval_mV[i] / 1000.0, 1); display.println(F("V"));
    }
  }
}


void drawLine(uint16_t range, uint16_t voltage, uint16_t channel) {
  display.setCursor(0, 64 - (voltage * 50 / range) - 3);
  display.println((float)voltage / 1000.0, 2);
  display.drawLine(32, 64 - (voltage * 50 / range), 32 + (channel) * 12, 64 - (voltage * 50 / range), SSD1306_INVERSE);
}


static void oscilloscope(uint8_t sweep, bool reset, uint8_t channel) {
  //引き継ぐ必要あり
  static uint16_t max_1 = 1;
  static uint16_t min_1 = 0xffff;
  static uint16_t max_2 = 1;
  static uint16_t trigger_level = 1;
  static unsigned long period = 1;
  static unsigned long time = millis();
  static bool reset_flag = false;
  static bool trigger_detected = false;
  //引き継ぐ必要なし 描画に時間がかかって周期計算が狂うからアクイジションメモリは描画ごとにリセット
  uint8_t trigger_count = 0;
  uint16_t acquisition_1_mV[128];
  uint16_t acquisition_2_mV[128];
  unsigned long long count = 0;

  if (reset) {
    max_1 = 1;
    min_1 = 0xffff;
    max_2 = 1;
    trigger_level = 1;
    period = 1;
    time = millis();
    reset_flag = true;
    trigger_detected = false;
    count = 0;
    display.setCursor(10, 32);
    display.print(F("oscilloscope:reset"));
    //Serial.println("oscilloscope:reset");
    return;
  }

  //とにかくデータをとる

  for (uint8_t i = 0; i < 128; i++) {
    acquisition_1_mV[i] = 0;
    acquisition_2_mV[i] = 0;
  }

  while (true) {
    //周期に合わせて待機しながらモード切替によるリセットの検知
    unsigned long long delay = micros();
    while (micros() - delay < period * 8 * sweep) {
      if (reset_flag) {
        reset_flag = false;
        return;
      }
    }
    for (uint8_t i = 0; i < 128; i++) {
      acquisition_1_mV[i] = acquisition_1_mV[i + 1];
      acquisition_2_mV[i] = acquisition_2_mV[i + 1];
    }

    acquisition_1_mV[127] = adc.toAnalog(adc.read(MCP3208::Channel::SINGLE_0)) * ADC_SCALING;
    acquisition_2_mV[127] = adc.toAnalog(adc.read(MCP3208::Channel::SINGLE_1)) * ADC_SCALING;
    if (acquisition_1_mV[127] > max_1) {
      max_1 = (uint16_t)((float)acquisition_1_mV[127] * (float)1.2);
    }
    if (acquisition_1_mV[127] < min_1) {
      min_1 = acquisition_1_mV[127];
    }
    if (max_1 > min_1) {
      trigger_level = min_1 + (max_1 - min_1) / 2;
    }
    if (acquisition_2_mV[127] > max_2) {
      max_2 = (uint16_t)((float)acquisition_2_mV[127] * (float)1.2);
    }
    //取得したデータとトリガ条件比べて周期確定
    if ((acquisition_1_mV[125] + acquisition_1_mV[124] < trigger_level * 2)
        && (acquisition_1_mV[127] + acquisition_1_mV[126] >= trigger_level * 2)
        && count > 5) { //最初は0mVだから除外
      //直前のトリガ信号との時間差を計測
      //直前のトリガの直後は無視
      if (millis() - time >= period / 2) {
        //最初のトリガ信号では周期は計算しない
        trigger_count++;
        trigger_detected = true;
        if (trigger_count >= 2) {
          period = millis() - time;
        }
        time = millis();
      }
    }

    //サンプリングをやめて描画に移行
    //正しく周期が計算でき、かつ画面いっぱいまでデータが取れ、真ん中でトリガ信号確認
    if ((acquisition_1_mV[64] >= trigger_level && acquisition_1_mV[63] < trigger_level) && trigger_count >= 3 && (count >= 128)) {
      break;
    }
    //トリガがそもそも確認できてない場合は画面いっぱいまでデータとって描画
    if (!trigger_detected && (count >= 128)) {
      count = 5;
      break;
    }

    count++;
  }

  //描画
  if (channel == 1) {    
    display.drawLine(0, 57 - (trigger_level * 50 / max_1), 128, 57 - (trigger_level * 50 / max_1), SSD1306_INVERSE);
    display.drawLine(64, 0, 64, 64, SSD1306_INVERSE);
    display.setCursor(0, 0);
    display.print(F("CH1:")); display.print(max_1); display.println(F("mV"));
    for (uint8_t i = 0; i < 128; i++) {
      display.drawPixel(i, 57 - (acquisition_1_mV[i] * 50 / max_1), SSD1306_WHITE);
    }
  } else {
    display.drawLine(64, 0, 64, 64, SSD1306_INVERSE);
    display.setCursor(0, 0);
    display.print(F("CH1:")); display.print(max_1); display.println(F("mV"));
    display.setCursor(0, 33);
    display.print(F("CH2:")); display.print(max_2); display.println(F("mV"));
    for (uint8_t i = 0; i < 128; i++) {
      display.drawPixel(i, 31 - (acquisition_1_mV[i] * 25 / max_1), SSD1306_WHITE);
      display.drawPixel(i, 63 - (acquisition_2_mV[i] * 25 / max_2), SSD1306_WHITE);
    }
  }

  display.setCursor(68, 0);
  display.print(period); display.println(F("ms"));
  /*
    display.setCursor(0, 57);
    display.print(F("count:")); display.println(count);*/
}
